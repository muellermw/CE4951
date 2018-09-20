/**
  ******************************************************************************
  * @file    main.c
  * @author  Marcus Mueller, Paul Scarbrough
  ******************************************************************************
*/


#include "leds.h"


int main(void)
{
	led_init();
	led_all_on();

	for(;;);
}
