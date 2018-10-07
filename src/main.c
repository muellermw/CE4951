/**
  ******************************************************************************
  * @file    main.c
  * @author  Marcus Mueller, Paul Scarbrough
  ******************************************************************************
*/

#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include "stm32f4xx_it.h"
#include "leds.h"
#include "channel_monitor.h"
#include "uart_driver.h"
#include "transmitter.h"
#include <stdlib.h>

int main(void)
{
	HAL_Init();
	SysTick_Init();
	led_init();
	channel_Monitor_Init();
	usart2_init(38400);
	transmitter_init();
	//char inputBuf[TRANSMISSION_SIZE_MAX];
	printf("Enter characters to transmit here:\n");
	//fgets(inputBuf, TRANSMISSION_SIZE_MAX, stdin);
	char arry[2]={'*','i'};

	int num=2;
	startTransmission(arry,num);

	while(1)
	{
//		printf("Enter characters to transmit here:\n");
//		fgets(inputBuf, TRANSMISSION_SIZE_MAX, stdin);
//		// check if the user has entered more than just a newline character
//		if (strlen(inputBuf) > 1)
//		{
//			// string length - 1 so the newline does not get transmitted (for now)
//			startTransmission(inputBuf, strlen(inputBuf)-1);
//		}
	}
}
