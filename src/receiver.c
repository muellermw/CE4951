/*
 * receiver.c
 */

#include "receiver.h"
#include "channel_monitor.h"
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static void fallingEdgeTrigger();
static void risingEdgeTrigger();
static void disableMonitorClock();
static void enableMonitorClock();

static char manchesterArray[TRANSMISSION_SIZE_MAX * 8 * 2];
static char askiiArray[TRANSMISSION_SIZE_MAX * 8 * 2];
static char charAray[TRANSMISSION_SIZE_MAX];//THIS IS THE ARRAY THAT HOLDS THE CAHRS. IT NEEDS TO BE CLEARED AND INDEX RESET IN MAIN

static uint32_t manchesterIndex=1;
static uint32_t manchesterIterator=1;
static uint32_t cahrArrayIndex=0;//THIS IS THE INDEX FOR THE CHAR ARRAY. IT NEEDS TO BE RESET TO 0 IN MAIN

static const uint16_t DELAY_TIME = 2000;//Got value after testing with scope to get us a .5ms delay
static TIM_HandleTypeDef hTim3 =
{
	.Instance = TIM3
};

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

	// initialize timer 3 as a 1.1ms timer interrupt when triggered
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
	memset(manchesterArray,'0',TRANSMISSION_SIZE_MAX * 8 * 2);
	memset(askiiArray,'0',TRANSMISSION_SIZE_MAX * 8 * 2);
	memset(charAray,0,TRANSMISSION_SIZE_MAX );

	manchesterArray[0]='1';//sets the first manchester bit

}

/**
 * interrupt handler for the timer. When the timer runs out it means 1 of 3 things. Their is a colision. THeir was no edge but it is still high. Their was no edge but it is still low. Manchester dose not change on every edge so this is needed to detect non edge transision bits
 */
void TIM3_IRQHandler(void){

	HAL_NVIC_DisableIRQ(EXTI3_IRQn);

	if(getCurrentMonitorState()!=COLLISION_STATE){

	if(edge==RISING_EDGE){
		risingEdgeTrigger();
	}else{
		fallingEdgeTrigger();
	}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);

	}else{
		memset(manchesterArray,'0',TRANSMISSION_SIZE_MAX * 8 * 2);
		memset(askiiArray,'0',TRANSMISSION_SIZE_MAX * 8 * 2);
		memset(charAray,'0',TRANSMISSION_SIZE_MAX );
		manchesterArray[0]='1';//sets the first manchester bit //WHEN TRANSMITTING SEND A 7 AS THE FIRST BIT THIS IF 0X55 AS PER LAB MANUAL
s
		cahrArrayIndex=0;
		manchesterIndex=1;
		manchesterIterator=1;
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

	//Sets the edge as rising or falling
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
	edge=RISING_EDGE;
	manchesterArray[manchesterIndex]='1';

	//We now have 16 bits recived
		if((manchesterIndex+1)%16==0){
			//this pulles the clock signal out and keeps the data signal
			for(int i=0; i<8;i++){
				askiiArray[i]=manchesterArray[manchesterIterator];
				manchesterIterator=(manchesterIterator+2);
			}

			//This converts from a string of binary to char array.
			char bitarray8[8];
			for(int j=0;j<8;j++){
				bitarray8[j]=(char)askiiArray[j];
			}
			charAray[cahrArrayIndex]=strtol(bitarray8,(char **)NULL,2);//array where all chars are stored
			cahrArrayIndex++;//index of cahr array

			memset(manchesterArray,'0',TRANSMISSION_SIZE_MAX * 8 * 2);//resets the manchester array
			memset(askiiArray,'0',TRANSMISSION_SIZE_MAX * 8 * 2);//resets the askii array

			//Re initalizes manchester aand
			manchesterIndex=0;
			manchesterIterator=1;
		}
	manchesterIndex++;
}

/**
 * Called when a falling edge is found
 */
static void fallingEdgeTrigger(){
	edge=FALLING_EDGE;
	manchesterArray[manchesterIndex]='0';

	//We now have 16 bits recived
		if((manchesterIndex+1)%16==0){
			//this pulles the clock signal out and keeps the data signal
			for(int i=0; i<8;i++){
				askiiArray[i]=manchesterArray[manchesterIterator];
				manchesterIterator=(manchesterIterator+2);
			}

			//This converts from a string of binary to char array.
			char bitarray8[8];
			for(int j=0;j<8;j++){
				bitarray8[j]=(char)askiiArray[j];
			}
			charAray[cahrArrayIndex]=strtol(bitarray8,(char **)NULL,2);//array where all chars are stored
			cahrArrayIndex++;//index of cahr array

			memset(manchesterArray,'0',TRANSMISSION_SIZE_MAX * 8 * 2);//resets the manchester array
			memset(askiiArray,'0',TRANSMISSION_SIZE_MAX * 8 * 2);//resets the askii array

			//Re initalizes manchester aand
			manchesterIndex=0;
			manchesterIterator=1;
		}
	manchesterIndex++;
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

