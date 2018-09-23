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

int main(void)
{
	HAL_Init();
	SysTick_Init();
	led_init();
	channel_Monitor_Init();

	while(1){}
}
