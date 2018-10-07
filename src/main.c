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
#include <stdlib.h>
#include "transmitter.h"

int main(void)
{
	HAL_Init();
	SysTick_Init();
	led_init();
	channel_Monitor_Init();
	usart2_init(38400);
	transmitter_init();
	char inputBuf[255];
	printf("Enter characters to transmit here:\n");
	//fgets(inputBuf, 255, stdin);
	char arry[2]={'h','i'};

	int num=2;
	startTransmission(arry,num);

	while(1)
	{
//		printf("Enter characters to transmit here:\n");
//		fgets(inputBuf, 255, stdin);
//		// check if the user has entered more than just a newline character
//		if (strlen(inputBuf) > 1)
//		{
//			startTransmission(inputBuf, strlen(inputBuf));
//		}
	}
}
