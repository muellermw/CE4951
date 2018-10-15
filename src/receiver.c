/*
 * receiver.c
 */

#include "receiver.h"
#include "channel_monitor.h"
#include "leds.h"
#include <stdint.h>

static void fallingEdgeTrigger();
static void risingEdgeTrigger();
static void disableMonitorClock();
static void enableMonitorClock();

static const uint16_t DELAY_TIME = 900;//Got value after testing with scope to get us a 1.11ms delay
static TIM_HandleTypeDef hTim3 =
{
	.Instance = TIM3
};

static state_enum monitor_state;
static edge_enum edge;

/**
 * initialize timers, set current state
 */
void receiver_Init() {
	//initialize Input pin PB3 (Arduino pin D3) for the receiver
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpioB;
	gpioB.Pin = GPIO_PIN_3;
	gpioB.Mode = GPIO_MODE_IT_RISING_FALLING;
	gpioB.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &gpioB);
	HAL_NVIC_SetPriority(EXTI3_IRQn,14,14);

	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	// initialize timer 2 as a 1.1ms timer interrupt when triggered
	__HAL_RCC_TIM3_CLK_ENABLE();
	hTim3.Instance = TIM3;
	hTim3.Init.Prescaler = 0;
	hTim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	// set auto-reload register
	hTim3.Init.Period = (16000000/DELAY_TIME); // 16,000,000/910 = 1.11ms timer
	// initialize timer 2 registers
	HAL_TIM_Base_Init(&hTim3);
	HAL_TIM_Base_Start(&hTim3);
	// enable interrupt for timer 3
	HAL_NVIC_SetPriority(TIM3_IRQn,1,1);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	disableMonitorClock();

	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5))
	{
		monitor_state=IDLE_STATE;
		led_on(0);
	}
	else
	{
		monitor_state=COLLISION_STATE;
		led_on(9);
	}
}

/**
 * interrupt handler for the timer. sets the new state of the monitor
 */
void TIM3_IRQHandler(void){

	HAL_NVIC_DisableIRQ(EXTI3_IRQn);

	led_all_off();//Turns off led due to state change

	if(edge==RISING_EDGE) {
		monitor_state=IDLE_STATE;
		led_on(0);
	} else {
		monitor_state=COLLISION_STATE;
		led_on(9);
	}

	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	disableMonitorClock();
}

/**
 * Interrupt handler for the input rising and falling edge tigger
 */
void EXTI3_IRQHandler(void)
{
	// reset the timer
	disableMonitorClock();

	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)){
		risingEdgeTrigger();
	}else{
		fallingEdgeTrigger();
	}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);

	enableMonitorClock();
}

/**
 * called when a rising edge is found
 */
static void risingEdgeTrigger(){
	led_all_off();//Turns off led due to state change
	//set LEDs to indicate busy_state
	edge=RISING_EDGE;
	monitor_state=BUSY_STATE;
	led_on(4);
}

/**
 * Called when a falling edge is found
 */
static void fallingEdgeTrigger(){
	led_all_off();//Turns off led due to state change
	//set LEDs to indicate busy_state
	edge=FALLING_EDGE;
	monitor_state=BUSY_STATE;
	led_on(4);
}

static void disableMonitorClock(){
	__HAL_TIM_DISABLE_IT(&hTim3, TIM_IT_UPDATE);
	__HAL_TIM_DISABLE(&hTim3);
	__HAL_TIM_SET_COUNTER(&hTim3, 0);
	__HAL_TIM_CLEAR_FLAG(&hTim3, TIM_IT_UPDATE);
}

static void enableMonitorClock(){

	__HAL_TIM_ENABLE_IT(&hTim3, TIM_IT_UPDATE);
	__HAL_TIM_SET_COUNTER(&hTim3, 0);
	__HAL_TIM_ENABLE(&hTim3);
}

