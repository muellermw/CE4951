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
	SysTick_Config(SystemCoreClock/dealy);
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
	//set LED'S to indicate busy_state
	//START TIMER FOR 1.11MS //see
	SysTick_Config(SystemCoreClock/dealy);
}

/**
 * Called when a falling edge is found. Starts timer to run for time "x"
 */
void fallingEdgeTrigger(){
	state=BUSY_STATE;
	//set LED'S to indicate busy_state
	//START TIMER FOR 1.11MS //see
	SysTick_Config(SystemCoreClock/dealy);
}

/**
 * Return the current state of the monitor
 */
state_enum getCurrentState(){
	return state;
}

