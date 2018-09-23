/*
 * File: channel_monitor.c
 * Purpose: allows for channel monitoring and detects when the signal is idle,collison or busy
 */
#include "stdint.h"
#include"channel_monitor.h"

state_enum state;
static uint32_t dealy=1100;

/**
 * initalize timers, set current state
 */
void channelMonitorInit(){
	state=IDLE_STATE;

	//initalize Input pin for channel monitor
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpioB;
	gpioB.Pin = GPIO_PIN_5;
	gpioB.Mode = GPIO_MODE_IT_RISING_FALLING;
	//SYSCFG_EXTI_LineConfig(EXTI_PortSoruceGPIOB,EXTI_PinSource5);
	HAL_GPIO_Init(GPIOB, &gpioB);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);


	//SysTick_Config(SystemCoreClock/dealy);
}

/**
 * Intrumpt handler for the input rising and falling edge tigger
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
 * intrupt handler for the timer. sets the new state of the monitor
 */
void Timer_Handler(){
	if(1==1){
		state=IDLE_STATE;
	}else{
		state=COLLISION_STATE;
	}
}

/**
 * called when a rising edge is found. Starts timer to run for time "x"
 */
void risingEdgeTrigger(){
	state=BUSY_STATE;
	led_all_on();
	//set LED'S to indicate busy_state
	//START TIMER FOR 1.11MS //see
	//SysTick_Config(SystemCoreClock/dealy);
}

/**
 * Called when a falling edge is found. Starts timer to run for time "x"
 */
void fallingEdgeTrigger(){
	state=BUSY_STATE;
	led_all_off();
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

