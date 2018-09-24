/*
 * File: channel_monitor.c
 * Purpose: allows for channel monitoring and detects when the signal is idle,collison or busy
 */

#include "channel_monitor.h"
#include "leds.h"
#include "stdint.h"

static TIM_HandleTypeDef hTim2 =
{
	.Instance = TIM2
};

state_enum state;

/**
 * initialize timers, set current state
 */
void channel_Monitor_Init(){
	//initialize Input pin for channel monitor
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpioB;
	gpioB.Pin = GPIO_PIN_5;
	gpioB.Mode = GPIO_MODE_IT_RISING_FALLING;
	HAL_GPIO_Init(GPIOB, &gpioB);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	// initialize timer 2 as a 1.1ms timer interrupt when triggered
	__HAL_RCC_TIM2_CLK_ENABLE();
	hTim2.Instance = TIM2;
	hTim2.Init.Prescaler = 0;
	hTim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	// set auto-reload register
	hTim2.Init.Period = 14545; // 16,000,000/1100 = 1.1ms timer
	HAL_TIM_Base_Init(&hTim2);
	HAL_TIM_Base_Start(&hTim2);
	// enable update interrupt
	__HAL_TIM_ENABLE_IT(&hTim2, TIM_IT_UPDATE);
	// enable interrupt for timer 2
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	led_all_off();//Turns off led due to state change
	state=IDLE_STATE;
	led_on(1);
}

/**
 * interrupt handler for the timer. sets the new state of the monitor
 */
void TIM2_IRQHandler(void){
	led_all_off();//Turns off led due to state change
	//Reads current input pin to determine state
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)==1){
		state=IDLE_STATE;
		led_on(1);
	}else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)==0){
		state=COLLISION_STATE;
		led_on(7);
	}
	// clear interrupt flag
	__HAL_TIM_CLEAR_FLAG(&hTim2, TIM_IT_UPDATE);
	// disable the update interrupt for timer 2
	__HAL_TIM_DISABLE_IT(&hTim2, TIM_IT_UPDATE);
}

/**
 * interrupt handler for the timer. sets the new state of the monitor
 */
void Timer_Handler(){

}

/**
 * Interrupt handler for the input rising and falling edge tigger
 */
void EXTI9_5_IRQHandler(void)
{
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)){
		risingEdgeTrigger();
	}else{
		fallingEdgeTrigger();
	}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
}

/**
 * called when a rising edge is found. Starts timer to run for time "x"
 */
void risingEdgeTrigger(){
	led_all_off();//Turns off led due to state change
	state=BUSY_STATE;
	led_on(4);
	//set LED'S to indicate busy_state
	//START TIMER FOR 1.11MS //see
	//SysTick_Config(SystemCoreClock/dealy);
}

/**
 * Called when a falling edge is found. Starts timer to run for time "x"
 */
void fallingEdgeTrigger(){
	led_all_off();//Turns off led due to state change
	state=BUSY_STATE;
	led_on(4);
	//set LED'S to indicate busy_state
	//START TIMER FOR 1.11MS //see
	//SysTick_Config(SystemCoreClock/dealy);
}

/**
 * Return the current state of the monitor
 */
state_enum getCurrentState(){
	return state;
}

