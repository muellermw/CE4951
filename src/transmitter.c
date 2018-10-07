/**
  ******************************************************************************
  * @file    transmitter.c
  * @author  Marcus Mueller, Paul Scarbrough
  ******************************************************************************
*/

#include "leds.h"
#include "stdint.h"
#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include "transmitter.h"
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static const uint16_t DELAY_TIME = 2000;

static TIM_HandleTypeDef hTim4 =
{
	.Instance = TIM4
};


static TIM_OC_InitTypeDef hOCTim4;

int original;
int *manchesterArray;//malloc(size);
int indexOfManchester;
bool stopCalled=false;



void transmitter_init(){
	/*
	 * Enable the timer and its output compare.
	 * This timer is a free running counter that
	 * generates an timer interrupt every 0.5 milliseconds
	 */
	__HAL_RCC_TIM4_CLK_ENABLE();
	hTim4.Instance = TIM4;
	hTim4.Init.Prescaler = 0;
	hTim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	// set auto-reload register
	hTim4.Init.Period = (16000000/DELAY_TIME); // 16,000,000/2000 = 0.5ms timer
	// initialize timer 2 registers
	HAL_TIM_Base_Init(&hTim4);
	HAL_TIM_Base_Start(&hTim4);
	// initialize output compare
	hTim4.Channel = HAL_TIM_ACTIVE_CHANNEL_1;
	HAL_TIM_OC_Init(&hTim4);
	hOCTim4.OCMode=TIM_OCMODE_ACTIVE;
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	HAL_TIM_OC_ConfigChannel(&hTim4, &hOCTim4, TIM_CHANNEL_4);
	__HAL_TIM_SET_COMPARE(&hTim4, TIM_CHANNEL_1, (16000000/DELAY_TIME));
	// enable interrupt for timer 4
	HAL_TIM_OC_Start_IT(&hTim4, TIM_CHANNEL_1);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);

	/*
	 * Enable output pin
	 */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef D13;
	D13.Pin = GPIO_PIN_5;
	D13.Mode = GPIO_MODE_OUTPUT_PP;
	D13.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &D13);
}

void TIM4_IRQHandler(void){
	// clear the pending OC interrupt
	__HAL_TIM_CLEAR_IT(&hTim4, TIM_IT_CC1);
	// USE THESE COMMANDS to change the output of pin PA5 (D13)
	// This is output 3.3V logic which is just what we need
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	//If the current state collison stop all transmission

//	if(manchesterArray[indexOfManchester]==0){
//		indexOfManchester++;
//		transmit_LOW();
//	}else if(manchesterArray[indexOfManchester]==1){
//		indexOfManchester++;
//		transmit_HIGH();
//	}
}

//called when we have a
void stopTransmission(){
	HAL_TIM_OC_Stop(&hTim4, TIM_CHANNEL_4);
	stopCalled=true;
	//Delay for set amout fo time
	//index now hoolds how much we have sent
	//for this lab we just retransmit

	//wait for random amout of time
//	startTransmission(char *array, int ammountOfChars);
}


//assume output is in binary
void startTransmission(char *array, int ammountOfChars){
	transmitter_init();
	int inputIndex=0;

	//encoding size= (amount of chars)*(number of bits per char)*2 for manchester up down encoding
	manchesterArray=malloc(sizeof(int)*ammountOfChars*8*2);
	memset(manchesterArray,0,sizeof(int)*ammountOfChars*8*2);

	//Converts from Ascii to binary
	int i =0;
	int fillUpArray[8];
	int manchesterBits=0;
	while( i<ammountOfChars){
		for(int f=0;f<8;f++){
			int bit=((array[i] >> f) & 1);
			fillUpArray[f]=bit;
		}
		//Flips the fillUpArray back into correct binary order
		//Stores into manchesterArray with clk and data
		for(int j=7;j>=0;j--){
			if(fillUpArray[j]==0){
				manchesterArray[manchesterBits]=1;//clock
				manchesterArray[manchesterBits+1]=0;//data
			}
			if(fillUpArray[j]==1){
				manchesterArray[manchesterBits]=0;//clock
				manchesterArray[manchesterBits+1]=1;//data
			}

		//incroments manchester bits up 2 (1 for clk 1 for data)
		manchesterBits=manchesterBits+2;
		//clears the fillUpArray
		memset(fillUpArray,0,8);
		}
		i++;//goes to next character
	}
	indexOfManchester=0;

	int test=manchesterArray[indexOfManchester];//NOT RETUNING CORRECT VALUE
	if(manchesterArray[indexOfManchester]==0){
		indexOfManchester++;
		//transmit_LOW();
	}
	if(manchesterArray[indexOfManchester]==1){
		indexOfManchester++;
		//transmit_HIGH();
	}
}
