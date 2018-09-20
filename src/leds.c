/*
 * File: led_functions.c
 * Purpose: provide basic functionality for the leds on the dev board
 */

#include "leds.h"

/*
 * initialize all leds
 */
void led_init() {
	// enable gpio a and b
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef gpioA;
	gpioA.Pin = GPIO_PIN_11 |
				GPIO_PIN_10 |
				GPIO_PIN_9  |
				GPIO_PIN_8  |
				GPIO_PIN_7;

	gpioA.Mode = GPIO_MODE_OUTPUT_PP;

	GPIO_InitTypeDef gpioB;
	gpioB.Pin = GPIO_PIN_13 |
			    GPIO_PIN_12 |
			    GPIO_PIN_10 |
			    GPIO_PIN_9  |
			    GPIO_PIN_8;

	gpioB.Mode = GPIO_MODE_OUTPUT_PP;

	HAL_GPIO_Init(GPIOA, &gpioA);
	HAL_GPIO_Init(GPIOB, &gpioB);
}

/*
 * turn on all leds
 */
void led_all_on() {
	HAL_GPIO_WritePin(GPIOA, (LED0|LED1|LED2|LED3|LED4), GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, (LED5|LED6|LED7|LED8|LED9), GPIO_PIN_SET);
}

/*
 * turn off all leds
 */
void led_all_off() {
	HAL_GPIO_WritePin(GPIOA, (LED0|LED1|LED2|LED3|LED4), GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, (LED5|LED6|LED7|LED8|LED9), GPIO_PIN_RESET);
}

/*
 * turn on a specific led
 *
 * arguments:
 * 		lednum: the led number based on the board labels
 */
void led_on(uint8_t lednum) {
	// used a case statement to determine which led to turn on
	switch (lednum) {
		case 0: HAL_GPIO_WritePin(GPIOA, LED0, GPIO_PIN_SET);
		break;
		case 1: HAL_GPIO_WritePin(GPIOA, LED1, GPIO_PIN_SET);
		break;
		case 2: HAL_GPIO_WritePin(GPIOA, LED2, GPIO_PIN_SET);
		break;
		case 3: HAL_GPIO_WritePin(GPIOA, LED3, GPIO_PIN_SET);
		break;
		case 4: HAL_GPIO_WritePin(GPIOA, LED4, GPIO_PIN_SET);
		break;
		case 5: HAL_GPIO_WritePin(GPIOB, LED5, GPIO_PIN_SET);
		break;
		case 6: HAL_GPIO_WritePin(GPIOB, LED6, GPIO_PIN_SET);
		break;
		case 7: HAL_GPIO_WritePin(GPIOB, LED7, GPIO_PIN_SET);
		break;
		case 8: HAL_GPIO_WritePin(GPIOB, LED8, GPIO_PIN_SET);
		break;
		case 9: HAL_GPIO_WritePin(GPIOB, LED9, GPIO_PIN_SET);
		break;
	}
}

/*
 * turn on a specific led
 *
 * arguments:
 * 		lednum: the led number based on the board labels
 */
void led_off(uint8_t lednum) {
	// used a case statement to determine which led to turn on
	switch (lednum) {
		case 0: HAL_GPIO_WritePin(GPIOA, LED0, GPIO_PIN_RESET);
		break;
		case 1: HAL_GPIO_WritePin(GPIOA, LED1, GPIO_PIN_RESET);
		break;
		case 2: HAL_GPIO_WritePin(GPIOA, LED2, GPIO_PIN_RESET);
		break;
		case 3: HAL_GPIO_WritePin(GPIOA, LED3, GPIO_PIN_RESET);
		break;
		case 4: HAL_GPIO_WritePin(GPIOA, LED4, GPIO_PIN_RESET);
		break;
		case 5: HAL_GPIO_WritePin(GPIOB, LED5, GPIO_PIN_RESET);
		break;
		case 6: HAL_GPIO_WritePin(GPIOB, LED6, GPIO_PIN_RESET);
		break;
		case 7: HAL_GPIO_WritePin(GPIOB, LED7, GPIO_PIN_RESET);
		break;
		case 8: HAL_GPIO_WritePin(GPIOB, LED8, GPIO_PIN_RESET);
		break;
		case 9: HAL_GPIO_WritePin(GPIOB, LED9, GPIO_PIN_RESET);
		break;
	}
}

/*
 * flash leds:
 * turn  on all leds [pause for specified time],
 * turn off all leds [pause for specified time]
 */
/*
void led_flash(uint16_t speed) {
	led_all_on();
	delay_ms(speed);
	led_all_off();
	delay_ms(speed);
}
*/

/*
 * scan from left to right and back:
 * turn on a single led from left to right and back in the fashion of knight rider
 */
/*
void led_scan(uint16_t speed) {
	// turn on led 9 down to 0 and turn off after a delay
	for (int8_t i=9; i>0; i--) {
		led_on(i);
		delay_ms(speed);
		led_off(i);
	}

	// turn on led 0 to 8 and turn off after delay
	for (int8_t i=0; i<9; i++) {
		led_on(i);
		delay_ms(speed);
		led_off(i);
	}
}
*/
