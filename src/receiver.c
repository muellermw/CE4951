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
static void resetReceivedMessage();

static char manchesterArray[TRANSMISSION_SIZE_MAX * 8];
static char asciiArray[TRANSMISSION_SIZE_MAX];

static uint32_t manchesterIndex = 0;
static bool messageReceived = false;

static const uint16_t DELAY_TIME = 758;
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

	// initialize timer 3 as a 1.32ms timer interrupt when triggered
	__HAL_RCC_TIM3_CLK_ENABLE();
	hTim3.Instance = TIM3;
	hTim3.Init.Prescaler = 0;
	hTim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	// set auto-reload register
	hTim3.Init.Period = (16000000/DELAY_TIME); // 16,000,000/758 = 1.32ms timer (1320us * 16 = 21,120)
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
 * Manchester dose not change on every edge so this is needed to detect non edge transition bits
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
 * Interrupt handler for the input rising and falling edge tigger
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
	// 0.5 + 1.32% (16 * 506.6 = 8106)
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
	edge=RISING_EDGE;
	manchesterArray[manchesterIndex]=0b1;
	manchesterIndex++;
}

/**
 * Called when a falling edge is found
 */
static void fallingEdgeTrigger(){
	edge=FALLING_EDGE;
	manchesterArray[manchesterIndex]=0b0;
	manchesterIndex++;
}

/**
 * Convert Manchester bit sequence to ASCII characters
 */
void convertReceivedMessage()
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

	if (asciiIndex <= TRANSMISSION_SIZE_MAX)
	{
		// add a null terminator to the string
		asciiArray[asciiIndex] = '\0';
	}
}

void printAnyReceivedMessage()
{
	if (messageReceived == true)
	{
		convertReceivedMessage();
		printf("MESSAGE RECEIVED:\n");
		printf("%s\n", asciiArray);
		resetReceivedMessage();
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

