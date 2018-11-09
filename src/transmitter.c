/**
  ******************************************************************************
  * @file    transmitter.c
  * @author  Marcus Mueller, Paul Scarbrough
  ******************************************************************************
*/

#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include "channel_monitor.h"
#include "crc.h"
#include "randTime.h"
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
static bool thereWasACollision = false;
static uint8_t numberOfRandomBackoffs = 0;

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
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef D10;
	D10.Pin = GPIO_PIN_6;
	D10.Mode = GPIO_MODE_OUTPUT_PP;
	D10.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &D10);
	// initialize pin output to high
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
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
		stopTransmission();
	}
	else
	{
		state_enum monitorState = getCurrentMonitorState();
		// make sure readyToTransmit is true,
		// make sure that the beginning of the transmission is in the IDLE_STATE,
		// if the transmission has started, make sure the line is not in COLLISION_STATE
		if ( (readyToTransmit==true) && ( (monitorState==IDLE_STATE && indexOfManchester==0) ||
				(monitorState!=COLLISION_STATE && indexOfManchester>0) ) )
		{
			// Changes the output of pin PB6 (D10)
			// This outputs 3.3V logic which is just what we need
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, manchesterArray[indexOfManchester]);
			indexOfManchester++;
		}

		// check if a collision occurred from the channel monitor and an IDLE state has yet to be encountered
		if (monitorCheckForCollision())
		{
			readyToTransmit = false;
			indexOfManchester = 0;
			thereWasACollision = true;
			// set pin back to idle high
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
		}
	}
}

//called when we have a collision on the channel monitor
void stopTransmission(){
	// set pin back to idle high
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	// disable interrupt for timer 4
	HAL_TIM_OC_Stop_IT(&hTim4, TIM_CHANNEL_1);
	readyToTransmit = false;
	// reset position to resend all buffer data
	indexOfManchester = 0;
	numberOfRandomBackoffs = 0;
}

/*
 * Check if there was any collisions and if there was,
 * back off with a random delay after the collision is gone
 */
void handleAnyTransmissionCollision()
{
	if (thereWasACollision)
	{
		if (getCurrentMonitorState() != COLLISION_STATE)
		{
			numberOfRandomBackoffs++;

			if (numberOfRandomBackoffs >= BACKOFF_LIMIT)
			{
				stopTransmission();
				printf("Abandoning transmission because of too many backoffs...\n");
			}
			else
			{
				HAL_Delay(randomTimeMilliseconds());
				readyToTransmit = true;
			}

			thereWasACollision = false;
		}
	}

}

/*
 * This method will transform a string a binary encoded Manchester array
 * and let the transmitter know that it is ready to start the transmission
 * after the Manchester encoding is done
 */
void startTransmission(uint8_t destination, uint8_t CRCflag, char *array, int amountOfChars){
	//Converts from ASCII to binary

	// this would cause a memory overflow, cannot continue
	if (amountOfChars > TRANSMISSION_SIZE_MAX)
	{
		return;
	}

	char packetString[6];
	// +1 for the CRC FCS
	manchesterSize = ((amountOfChars+sizeof(packetString)+1) * 8 * 2);
	int charIndex = 0;
	int manchesterIndex = 0;

	// form the packet string
	packetString[PREAMBLE_INDEX] = PREAMBLE;
	packetString[VERSION_INDEX] = VERSION;
	packetString[SOURCE_INDEX] = MY_SOURCE_ADDRESS;
	packetString[DESTINATION_INDEX] = destination;
	packetString[LENGTH_INDEX] = amountOfChars;
	packetString[CRC_FLAG_INDEX] = CRCflag;

	// add the start of the packet to the manchester array first

	for (int i=0; i<sizeof(packetString); i++)
	{
		for (int j=7; j>=0; j--)
		{
			// go through each bit of the character and assign
			// not the bit, and the bit to the manchester array
			manchesterArray[manchesterIndex] = !((packetString[i]>>j) & 0b1);
			manchesterIndex++;
			manchesterArray[manchesterIndex] = ((packetString[i]>>j) & 0b1);
			manchesterIndex++;
		}
	}

	// loop through the entire string to fill the manchester array
	// leave space to put the CRC into the end of the packet
	while (manchesterIndex < (manchesterSize-16))
	{
		char referenceChar = array[charIndex];

		/*
		// implement override test characters to make unit testing easier
		if (referenceChar == '^')
		{
			referenceChar = (char)0x00;
		}
		else if (referenceChar == '&')
		{
			referenceChar = (char)0xFF;
		}
		else if (referenceChar == '*')
		{
			referenceChar = (char)0xAA;
		}
		*/

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

	char crcChar = 0xAA;
	if (CRCflag == 0x01)
	{
		crcChar = calculate_CRC(&array[0], amountOfChars);
	}

	// add the CRC FCS onto the end of the packet
	for (int i=7; i>=0; i--)
	{
		// go through each bit of the character and assign
		// not the bit, and the bit to the manchester array
		manchesterArray[manchesterIndex] = !((crcChar>>i) & 0b1);
		manchesterIndex++;
		manchesterArray[manchesterIndex] = ((crcChar>>i) & 0b1);
		manchesterIndex++;
	}

	// enable interrupt for timer 4
	HAL_TIM_OC_Start_IT(&hTim4, TIM_CHANNEL_1);
	readyToTransmit = true;
}
