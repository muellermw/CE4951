/*
 * File: channel_monitor.c
 * Purpose: allows for channel monitoring and detects when the signal is idle,collison or busy
 */

#include "channel_monitor.h"
#include "leds.h"
#include "stdint.h"

static void fallingEdgeTrigger();
static void risingEdgeTrigger();
static void disableMonitorClock();
static void enableMonitorClock();
static const uint16_t DELAY_TIME = 900;//Got value after testing with scope to get us a 1.11ms delay
static TIM_HandleTypeDef hTim2 =
{
	.Instance = TIM2
};
int edgetype;

state_enum state;
edge_enum edge;

/**
 * initialize timers, set current state
 */
void channel_Monitor_Init(){
	state=IDLE_STATE;
	//initialize Input pin for channel monitor
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpioB;
	gpioB.Pin = GPIO_PIN_5;
	gpioB.Mode = GPIO_MODE_IT_RISING_FALLING;
	gpioB.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &gpioB);
	HAL_NVIC_SetPriority(EXTI9_5_IRQn,15,15);

	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	// initialize timer 2 as a 1.1ms timer interrupt when triggered
	__HAL_RCC_TIM2_CLK_ENABLE();
	hTim2.Instance = TIM2;
	hTim2.Init.Prescaler = 0;
	hTim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	// set auto-reload register
	hTim2.Init.Period = (16000000/DELAY_TIME); // 16,000,000/910 = 1.11ms timer
	// initialize timer 2 registers
	HAL_TIM_Base_Init(&hTim2);
	HAL_TIM_Base_Start(&hTim2);
	// enable interrupt for timer 2
	HAL_NVIC_SetPriority(TIM2_IRQn,0,0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	disableMonitorClock();
}

/**
 * interrupt handler for the timer. sets the new state of the monitor
 */
void TIM2_IRQHandler(void){

	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);

	led_all_off();//Turns off led due to state change

	if(edge==RISING_EDGE) {
		state=IDLE_STATE;
		led_on(1);
	} else {
		state=COLLISION_STATE;
		led_on(7);
	}

	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	disableMonitorClock();
}

/**
 * Interrupt handler for the input rising and falling edge tigger
 */
void EXTI9_5_IRQHandler(void)
{
	// reset the timer
	disableMonitorClock();

	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)){
		risingEdgeTrigger();
	}else{
		fallingEdgeTrigger();
	}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);

	enableMonitorClock();
}

/**
 * called when a rising edge is found
 */
static void risingEdgeTrigger(){
	led_all_off();//Turns off led due to state change
	//set LEDs to indicate busy_state
	edge=RISING_EDGE;
	state=BUSY_STATE;
	led_on(4);
}

/**
 * Called when a falling edge is found
 */
static void fallingEdgeTrigger(){
	led_all_off();//Turns off led due to state change
	//set LEDs to indicate busy_state
	edge=FALLING_EDGE;
	state=BUSY_STATE;
	led_on(4);
}

static void disableMonitorClock(){
	__HAL_TIM_DISABLE_IT(&hTim2, TIM_IT_UPDATE);
	__HAL_TIM_DISABLE(&hTim2);
	__HAL_TIM_SET_COUNTER(&hTim2, 0);
	__HAL_TIM_CLEAR_FLAG(&hTim2, TIM_IT_UPDATE);
}

static void enableMonitorClock(){

	__HAL_TIM_ENABLE_IT(&hTim2, TIM_IT_UPDATE);
	__HAL_TIM_SET_COUNTER(&hTim2, 0);
	__HAL_TIM_ENABLE(&hTim2);
}

/**
 * Return the current state of the monitor
 */
state_enum getCurrentState(){
	return state;
}

