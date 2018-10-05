

#include "leds.h"
#include "stdint.h"
#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include <stdio.h>
#include <stdbool.h>

static const uint16_t DELAY_TIME = 2000;//Got value after testing with scope to get us a 1.11ms delay
static TIM_HandleTypeDef hTim4 =
{
	.Instance = TIM4
};


static TIM_OC_InitTypeDef OCTIM;

int original;
int *manchesterArray;//malloc(size);
int index;
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

	if(manchesterArray[index]==0){
		index++;
		transmit_LOW();
	}else if(manchesterArray[index]==1){
		index++;
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
	startTransmission(original);
}


//assume output is in binary
void startTransmission(){

	transmitter_init();

	//Size of the buffer
	//*manchesterArray=malloc(sizeof());

	//convert to binary and store in manchesterArray

	index=0;

	if(manchesterArray[index]==0){
		index++;
		transmit_LOW();
	}
	if(manchesterArray[index]==1){
		index++;
		transmit_HIGH();
	}
}
