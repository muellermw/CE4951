

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

static const uint16_t DELAY_TIME = 2000;//Got value after testing with scope to get us a 1.11ms delay
static TIM_HandleTypeDef hTim4 =
{
	.Instance = TIM4
};


static TIM_OC_InitTypeDef OCTIM;

int original;
int *manchesterArray;//malloc(size);
int indexOfManchester;
bool stopCalled=false;



void transmitter_init(){
	__HAL_RCC_TIM4_CLK_ENABLE();
	hTim4.Instance = TIM4;
	hTim4.Init.Prescaler = 0;
	hTim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	// set auto-reload register
	hTim4.Init.Period = (16000000/DELAY_TIME); // 16,000,000/910 = 1.11ms timer
	// initialize timer 2 registers
	HAL_TIM_Base_Init(&hTim4);
	HAL_TIM_Base_Start(&hTim4);


	hTim4.Channel = HAL_TIM_ACTIVE_CHANNEL_1;

	HAL_TIM_OC_Init(&hTim4);

//	OCTIM.OCMode=TIM_OCMODE_TOGGLE;
//	OCTIM.OCMode=
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	HAL_TIM_OC_ConfigChannel(&hTim4, &OCTIM, TIM_CHANNEL_4);

	// enable interrupt for timer 2
	//HAL_NVIC_SetPriority(TIM2_IRQn,0,0);
	//HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

//Forces a 1 on the output
void transmit_HIGH(){
	OCTIM.OCMode=TIM_OCMODE_FORCED_ACTIVE;
	HAL_TIM_OC_ConfigChannel(&hTim4, &OCTIM, TIM_CHANNEL_4);
	HAL_TIM_OC_Start(&hTim4, TIM_CHANNEL_4);
}

//Forices a 0 on the ouput
void transmit_LOW(){
	OCTIM.OCMode=TIM_OCMODE_FORCED_INACTIVE;
	HAL_TIM_OC_ConfigChannel(&hTim4, &OCTIM, TIM_CHANNEL_4);
	HAL_TIM_OC_Start(&hTim4, TIM_CHANNEL_4);
}

void TIM4_IRQHandler(void){

	//If the current state collison stop all transmission

	if(manchesterArray[indexOfManchester]==0){
		indexOfManchester++;
		transmit_LOW();
	}else if(manchesterArray[indexOfManchester]==1){
		indexOfManchester++;
		transmit_HIGH();
	}
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

	//Conversts from characters to Askii
	int *arrayAskii;
	arrayAskii=malloc(sizeof(int)*ammountOfChars);
	memset(arrayAskii,0,sizeof(int)*ammountOfChars);
	for(int i=0;i<ammountOfChars;i++){
		arrayAskii[i]=5;
	}
	//encdoign siez= (ammoutn of chars)*(nubmer of bits per char)*2 for manchester up down encoding
	manchesterArray=malloc(ammountOfChars*8*2);

	//Cnverts from Askii to binary
	int i =0;
	int manchesterBits=0;
	while( i<ammountOfChars){
		for(int f=8;f!=0;f--){
			int bit=(( unsigned char )arrayAskii[i] >> f & 1);

			if(bit==0){
				manchesterArray[manchesterBits]=1;
				manchesterArray[manchesterBits+1]=0;
			}
			if(bit==1){
				manchesterArray[manchesterBits]=0;
				manchesterArray[manchesterBits+1]=1;
			}
			manchesterBits=manchesterBits+2;
		}
		i++;
	}





	//if(arry[inputIndex])
	//Size of the buffer
	//*manchesterArray=malloc(sizeof());

	//convert to binary and store in manchesterArray

	indexOfManchester=0;

	if(manchesterArray[indexOfManchester]==0){
		indexOfManchester++;
		transmit_LOW();
	}
	if(manchesterArray[indexOfManchester]==1){
		indexOfManchester++;
		transmit_HIGH();
	}
}
