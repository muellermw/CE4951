/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32f4xx_it.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static volatile SYSTICK_REGISTERS* STK = (SYSTICK_REGISTERS*) SYSTICK_BASE;
static uint32_t CPU_Clock_Frequency;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * Initialize the SysTick Timer for 1ms interrupts
 */
void SysTick_Init(void)
{
	CPU_Clock_Frequency = HAL_RCC_GetSysClockFreq();
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	SysTick_Config((CPU_Clock_Frequency/1000)-1);
}

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
}

/**
 * Provides a delay in microseconds using the SysTick timer
 */
void Delay_Micro(uint32_t time_us) {
	// use delay_ms if time_us > 1000
	if (time_us > 1000) {
		HAL_Delay(time_us/1000);
		// get the remainder of time in us to run
		time_us %= 1000;
	}

	if (time_us) {
		int32_t time_passed = 0;
		uint32_t start = STK->VALUE;
		// find how many clock cycles are needed to be equivalent to the time in us
		uint32_t delay_us = time_us * (CPU_Clock_Frequency/1000000);

		// wait until the elapsed time is greater than the delay in us
		while (delay_us > time_passed) {
			time_passed = start - STK->VALUE;

			// if negative, add the reload time to it
			if (time_passed < 0) {
				time_passed += STK->RELOAD;
			}
		}
	}
}

