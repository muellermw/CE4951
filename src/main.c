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

int main(void)
{
	HAL_Init();
	SysTick_Init();
	led_init();
	channel_Monitor_Init();
	usart2_init(38400);
	char test[10];
	int number;
	fgets(test, 10, stdin);
	number = atoi(test);
	printf("captured number: %d", number);
	while(1){}
}
