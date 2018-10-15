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
#include "receiver.h"
#include <stdlib.h>
#include <string.h>

int main(void)
{
	HAL_Init();
	SysTick_Init();
	led_init();
	transmitter_init();
	channel_Monitor_Init();
	receiver_Init();
	usart2_init(38400);
	char inputBuf[TRANSMISSION_SIZE_MAX];

	// program loop (will never stop)
	while(1)
	{
		printf("Enter characters to transmit here:\n");

		int size = 0;
		char input = '\0';

		while (input != '\n')
		{
			input = usart2_getch_noblock();
			// make sure the character is not null
			if (input != '\0')
			{
				// make sure the buffer has enough space for another character
				if (size < TRANSMISSION_SIZE_MAX-1)
				{
					// check for backspaces
					if(input=='\b' || input=='\177')
					{
						// only backspace if there is data to clear
						if (size > 0)
						{
							// move back a character
							size--;
							// delete the last character
							inputBuf[size] = '\0';
						}
					}
					else
					{
						inputBuf[size] = input;
						size++;
					}
				}
			}
			// TODO
			/*********************************************************
			 * AFTER THIS IF-STATEMENT WE CAN CHECK FOR RECEIVER INPUT
			 ********************************************************/
		}
		// append the ending null character
		inputBuf[size] = '\0';

		// check if the user has entered more than just a newline character
		if (strlen(inputBuf) > 1)
		{
			// string length - 1 so the newline does not get transmitted (for now)
			startTransmission(inputBuf, strlen(inputBuf)-1);
		}
	}
}
