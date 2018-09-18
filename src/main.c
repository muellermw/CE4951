/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
			

int main(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef gpioB;
	gpioB.Pin = GPIO_PIN_11 |
				 GPIO_PIN_10 |
				 GPIO_PIN_9  |
				 GPIO_PIN_8  |
				 GPIO_PIN_7;
	gpioB.Mode = GPIO_MODE_OUTPUT_PP;

	HAL_GPIO_Init(GPIOB, &gpioB);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);

	for(;;);
}
