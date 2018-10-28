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
#include "crc.h"
#include "randTime.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef enum
{
	EnterDestination = 0,
	EnterCRCflag,
	EnterString
} menuLevel;

int main(void)
{
	HAL_Init();
	SysTick_Init();
	generate_CRC_Table();
	initRandTime();
	led_init();
	transmitter_init();
	channel_Monitor_Init();
	receiver_Init();
	usart2_init(38400);

	char inputBuf[MESSAGE_SIZE_MAX];
	menuLevel menuSelect = EnterDestination;
	unsigned int destinationAddr = 0;
	unsigned int CRCflag = 0;

	// program loop (will never stop)
	while(1)
	{
		// the menu has 3 different stages to for the packet information from the user
		switch (menuSelect)
		{
		case EnterDestination:
			printf("Enter the destination address you want to send a message to:\n");
			break;

		case EnterCRCflag:
			printf("Enter 1 to use CRC, 0 to ignore CRC:\n");
			break;

		case EnterString:
			printf("Enter characters to transmit here:\n");
			break;
		}

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
			/*********************************************************
			 * AFTER THIS IF-STATEMENT WE CHECK FOR INTERRUPT FLAGS,
			 * AS THIS LOOP IS WHERE WE SPEND MOST OF OUR TIME
			 ********************************************************/
			printAnyReceivedMessage();
		}
		// append the ending null character
		inputBuf[size] = '\0';

		// check if the user has entered more than just a newline character
		if (strlen(inputBuf) > 1)
		{
			switch (menuSelect)
			{
			case EnterDestination:
				sscanf(inputBuf, "%u", &destinationAddr);
				menuSelect = EnterCRCflag;
				break;

			case EnterCRCflag:
				sscanf(inputBuf, "%u", &CRCflag);
				// if anything other than 0 was entered, default to 1 to check the CRC
				if (CRCflag != 0)
				{
					CRCflag = 1;
				}
				menuSelect = EnterString;
				break;

			case EnterString:
				// string length - 1 so the newline does not get transmitted
				startTransmission(destinationAddr, CRCflag, inputBuf, strlen(inputBuf)-1);
				menuSelect = EnterDestination;
				break;
			}
		}
	}
}
