/*
 * receiver.c
 */

#include "receiver.h"
#include "channel_monitor.h"
#include "transmitter.h"
#include "crc.h"
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static void fallingEdgeTrigger();
static void risingEdgeTrigger();
static void disableMonitorClock();
static void enableMonitorClock();
static void resetReceivedMessage();
static void convertReceivedMessageToASCII();
static receiverErrorType receiverParsePacket();
static bool receiverCheckPreamble();
static bool receiverCheckVersion();
static bool receiverGetLength();
static bool receiverCheckCRCflag();
static bool receiverCheckCRCtrailer();
static void receiverGetSource();
static void receiverGetDestination();

static char manchesterArray[TRANSMISSION_SIZE_MAX * 8];
// +1 to allow characters plus a null character
static char asciiArray[TRANSMISSION_SIZE_MAX + 1];

static uint32_t manchesterIndex = 0;
static bool messageReceived = false;

static const uint16_t DELAY_TIME = 968;
static TIM_HandleTypeDef hTim3 =
{
	.Instance = TIM3
};

static struct packet receivedPacket;

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

	// initialize timer 3 as a 1.32ms timer interrupt when triggered
	__HAL_RCC_TIM3_CLK_ENABLE();
	hTim3.Instance = TIM3;
	hTim3.Init.Prescaler = 0;
	hTim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	// set auto-reload register
	hTim3.Init.Period = (16000000/DELAY_TIME); // 16,000,000/968 = 1032us timer (1032us * 16 = 16,512)
	// initialize timer 2 registers
	HAL_TIM_Base_Init(&hTim3);
	HAL_TIM_Base_Start(&hTim3);
	// enable interrupt for timer 3
	HAL_NVIC_SetPriority(TIM3_IRQn,1,1);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	disableMonitorClock();
	resetReceivedMessage();
}

/**
 * interrupt handler for the timer. When the timer runs out it means 1 of 3 things.
 * There is a collision. There was no edge but it is still high. There was no edge but it is still low.
 * Manchester does not change on every edge so this is needed to detect non edge transition bits
 */
void TIM3_IRQHandler(void){
	HAL_NVIC_DisableIRQ(EXTI3_IRQn);

	if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3))
	{
		if (manchesterIndex >= 8)
		{
			messageReceived = true;
		}
		else
		{
			resetReceivedMessage();
		}
	}

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	disableMonitorClock();
}

/**
 * Interrupt handler for the input rising and falling edge trigger
 */
void EXTI3_IRQHandler(void)
{
	uint32_t ticks = __HAL_TIM_GET_COUNTER(&hTim3);
	int edgeSample = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3);
	if (ticks == 0)
	{
		manchesterArray[manchesterIndex] = 0;
		manchesterIndex++;
		enableMonitorClock();
	}
	// (500us + 1.32% = 506.6us) (16 ticks/us * 506.6us = 8106 ticks)
	else if (ticks <= 8106)
	{
		// we ignore the clock edge
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
		return;
	}
	else
	{
		//Sets the edge as rising or falling
		if (edgeSample) {
			risingEdgeTrigger();
		} else {
			fallingEdgeTrigger();
		}

		// reset the timer
		disableMonitorClock();
		enableMonitorClock();
	}

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
}

/**
 * called when a rising edge is found
 */
static void risingEdgeTrigger(){
	// make sure that we aren't overflowing the bit buffer
	if (manchesterIndex < (TRANSMISSION_SIZE_MAX*8))
	{
		manchesterArray[manchesterIndex]=0b1;
		manchesterIndex++;
	}
}

/**
 * Called when a falling edge is found
 */
static void fallingEdgeTrigger(){
	// make sure that we aren't overflowing the bit buffer
	if (manchesterIndex < (TRANSMISSION_SIZE_MAX*8))
	{
		manchesterArray[manchesterIndex]=0b0;
		manchesterIndex++;
	}
}

/**
 * Convert Manchester bit sequence to ASCII characters ended with a null character
 */
static void convertReceivedMessageToASCII()
{
	int manchesterIterator = 0;
	int asciiIndex = 0;

	while (manchesterIterator < manchesterIndex)
	{
		asciiArray[asciiIndex] = (manchesterArray[manchesterIterator+0] << 7 |
								  manchesterArray[manchesterIterator+1] << 6 |
								  manchesterArray[manchesterIterator+2] << 5 |
								  manchesterArray[manchesterIterator+3] << 4 |
								  manchesterArray[manchesterIterator+4] << 3 |
								  manchesterArray[manchesterIterator+5] << 2 |
								  manchesterArray[manchesterIterator+6] << 1 |
								  manchesterArray[manchesterIterator+7]);
		manchesterIterator += 8;
		asciiIndex++;
	}

	if (asciiIndex < TRANSMISSION_SIZE_MAX+1)
	{
		// add a null terminator to the string
		asciiArray[asciiIndex] = '\0';
	}
}

void printAnyReceivedMessage()
{
	if (messageReceived == true)
	{
		convertReceivedMessageToASCII();
		receiverErrorType errorCheck;
		errorCheck = receiverParsePacket();

		switch (errorCheck)
		{
		case PreambleError:
			printf("Message received with no valid preamble. Discarding...\n");
			break;

		case VersionError:
			printf("Message received with invalid version number. Continuing anyway...\n");
			break;

		case LengthError:
			printf("Message received with 0 message length. Trying with length 1 instead.\n");
			break;

		case CRCflagError:
			printf("Message received with invalid CRC flag. Continuing and checking CRC.\n");
			break;

		case CRCtrailerError:
			printf("CRC trailer does not match expected CRC. The message may have been corrupted...\n");
			break;

		case NoError:
			printf("Message sent to address %d detected.\n", receivedPacket.Destination);
			break;
		}

		// accept packets addressed to our address or address 0
		if ((errorCheck!=PreambleError) && (receivedPacket.Destination==MY_SOURCE_ADDRESS || receivedPacket.Destination==0))
		{
			char receivedString[MESSAGE_SIZE_MAX];
			// copy the message from the received ASCII array into the new char array to print out the message
			strncpy(receivedString, &asciiArray[MESSAGE_INDEX], sizeof(receivedString));
			// add a null character at the end of the string
			receivedString[(uint8_t)receivedPacket.Length] = '\0';
			printf("MESSAGE RECEIVED:\n%s\n", receivedString);
		}

		resetReceivedMessage();
	}
}

/*
 * Parse through the packet that was received
 *
 * Return any error that was found
 */
static receiverErrorType receiverParsePacket()
{
	receiverErrorType packetCheck = NoError;

	if (!receiverCheckPreamble())
	{
		// do not attempt to parse any further if the preamble is incorrect
		return PreambleError;
	}

	if (!receiverCheckVersion())
	{
		packetCheck = VersionError;
	}

	if (!receiverGetLength())
	{
		packetCheck = LengthError;
	}

	// get packet source and destination
	receiverGetSource();
	receiverGetDestination();

	if (!receiverCheckCRCflag())
	{
		packetCheck = CRCflagError;
	}

	if (!receiverCheckCRCtrailer())
	{
		packetCheck = CRCtrailerError;
	}

	return packetCheck;
}

/*
 * Check that the preamble of the message contains 0x55
 */
static bool receiverCheckPreamble()
{
	receivedPacket.Sync = asciiArray[PREAMBLE_INDEX];
	if (receivedPacket.Sync == PREAMBLE)
	{
		return true;
	}
	else
	{
		// the preamble of the message is not 0x55, so let caller know that this is invalid
		return false;
	}
}

/*
 * Check that the packet version is 0x01
 */
static bool receiverCheckVersion()
{
	receivedPacket.Version = asciiArray[VERSION_INDEX];
	if (receivedPacket.Version == VERSION)
	{
		return true;
	}
	else
	{
		// the version of the message is not 0x01, so let caller know that this is invalid
		return false;
	}
}

/*
 * Check that the length is greater than 0
 */
static bool receiverGetLength()
{
	// default to a length of 1
	receivedPacket.Length = 1;
	if (asciiArray[LENGTH_INDEX] != 0)
	{
		receivedPacket.Length = asciiArray[LENGTH_INDEX];
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * Get the source address from the packet
 */
static void receiverGetSource()
{
	receivedPacket.Source = asciiArray[SOURCE_INDEX];
}

/*
 * Get the source address from the packet
 */
static void receiverGetDestination()
{
	receivedPacket.Destination = asciiArray[DESTINATION_INDEX];
}

/*
 * Get the CRC flag from the packet,
 * return false if the flag is not 0 or 1
 */
static bool receiverCheckCRCflag()
{
	// default to perform CRC check
	receivedPacket.CRCflag = 0x1;
	if (asciiArray[CRC_FLAG_INDEX]==0x00 || asciiArray[CRC_FLAG_INDEX]==0x01)
	{
		receivedPacket.CRCflag = asciiArray[CRC_FLAG_INDEX];
		return true;
	}
	else
	{
		// an invalid CRC flag has been encountered, let the caller know
		return false;
	}
}

/**
 * Check the CRC trailer that concluded the packet.
 * If the CRC flag is 1, compare the CRC trailer with the calculated CRC for the message
 * If the CRC flag is 0, compare the CRC trailer with 0xAA
 * Return false is the data does not compare correctly
 * This function assumes that the receivedPacket variable has been correctly updated thus far!
 */
static bool receiverCheckCRCtrailer()
{
	char crcCheck = (char)0xAA;
	// get the CRC trailer by calculating the offset by going one address after the crc flag offset and message length
	char crcTrailer = asciiArray[CRC_FLAG_INDEX + receivedPacket.Length + 1];

	if (receivedPacket.CRCflag == 0x01)
	{
		crcCheck = calculate_CRC(&asciiArray[MESSAGE_INDEX], receivedPacket.Length);
	}

	if (crcCheck == crcTrailer)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static void resetReceivedMessage()
{
	manchesterIndex=0;
	messageReceived = false;
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
