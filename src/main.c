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
	SysTick_Init();
	led_init();
	led_all_on();
	channelMonitorInit();

	// accumulate a 5 second delay using 50 microsecond delays
	//for (int i=0; i<100000; i++)
	//{
	//	Delay_Micro(50);
	//}

	led_all_off();
	while(1){
		int test=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5);
	}

	for(;;);
}
