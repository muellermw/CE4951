/**
  ******************************************************************************
  * @file    transmitter.c
  * @author  Marcus Mueller, Paul Scarbrough
  ******************************************************************************
*/

#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include "channel_monitor.h"
#include "transmitter.h"
#include <stdbool.h>

static const uint16_t DELAY_TIME = 2000;

static TIM_HandleTypeDef hTim4 =
{
	.Instance = TIM4
};

static TIM_OC_InitTypeDef hOCTim4;

static uint8_t manchesterArray[TRANSMISSION_SIZE_MAX * 8 * 2];
static int indexOfManchester = 0;
static int manchesterSize = 0;
static bool readyToTransmit = false;


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
	// disable interrupt for timer 4
	HAL_TIM_OC_Stop_IT(&hTim4, TIM_CHANNEL_1);
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
	// initialize pin output to high
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

/*
 * Timer 4 interrupt handles the sending of Manchester binary data
 */
void TIM4_IRQHandler(void){
	// clear the pending OC interrupt
	__HAL_TIM_CLEAR_IT(&hTim4, TIM_IT_CC1);
	// check if index is equal to the size of the array,
	// this means that we are done transmitting the message
	// the index should never get larger than the array size
	if (indexOfManchester >= manchesterSize)
	{
		// disable interrupt for timer 4
		HAL_TIM_OC_Stop_IT(&hTim4, TIM_CHANNEL_1);
		readyToTransmit = false;
		stopTransmission();
	}
	else
	{
		state_enum monitorState = getCurrentMonitorState();

		if (readyToTransmit==true && monitorState==IDLE_STATE)
		{
			// Changes the output of pin PA5 (D13)
			// This outputs 3.3V logic which is just what we need
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, manchesterArray[indexOfManchester]);
			indexOfManchester++;
		}

		if (monitorState == COLLISION_STATE)
		{
			stopTransmission();
		}
	}
}

//called when we have a collision on the channel monitor
void stopTransmission(){
	// set pin back to idle high
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	// reset position to resend all buffer data
	indexOfManchester = 0;
}


/*
 * This method will transform a string a binary encoded Manchester array
 * and let the transmitter know that it is ready to start the transmission
 * after the Manchester encoding is done
 */
void startTransmission(char *array, int amountOfChars){
	//Converts from ASCII to binary

	// this would cause a memory overflow, cannot continue
	if (amountOfChars > TRANSMISSION_SIZE_MAX)
	{
		return;
	}

	manchesterSize = amountOfChars * 8 * 2;
	int charIndex = 0;
	int manchesterIndex = 0;

	// loop through the entire string to fill the manchester array
	while (manchesterIndex < manchesterSize)
	{
		char referenceChar = array[charIndex];
		// implement override test characters to make unit testing easier
		if (referenceChar == '*')
		{
			referenceChar = (char)0xAA;
		}

		// increment the character index
		charIndex++;

		for (int i=7; i>=0; i--)
		{
			// go through each bit of the character and assign
			// not the bit, and the bit to the manchester array
			manchesterArray[manchesterIndex] = !((referenceChar>>i) & 0b1);
			manchesterIndex++;
			manchesterArray[manchesterIndex] = ((referenceChar>>i) & 0b1);
			manchesterIndex++;
		}
	}
	// enable interrupt for timer 4
	HAL_TIM_OC_Start_IT(&hTim4, TIM_CHANNEL_1);
	readyToTransmit = true;
}
