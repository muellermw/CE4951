/*
 * led_functions header file
 */

#ifndef LEDS_H_
#define LEDS_H_

#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include <stdio.h>

// GPIO A ODR LEDs    1111_1000_0000 : PA11-7
// GPIO B ODR LEDs 11_0111_0000_0000 : PB13-12, PB10-8
// LED ODR values (starting with 0 as labeled on the board)
#define LED0 (uint16_t) GPIO_PIN_7
#define LED1 (uint16_t) GPIO_PIN_8
#define LED2 (uint16_t) GPIO_PIN_9
#define LED3 (uint16_t) GPIO_PIN_10
#define LED4 (uint16_t) GPIO_PIN_11
#define LED5 (uint16_t) GPIO_PIN_8
#define LED6 (uint16_t) GPIO_PIN_9
#define LED7 (uint16_t) GPIO_PIN_10
#define LED8 (uint16_t) GPIO_PIN_12
#define LED9 (uint16_t) GPIO_PIN_13

extern void led_init();
extern void led_all_on();
extern void led_all_off();
extern void led_on(uint8_t lednum);
extern void led_off(uint8_t lednum);
// all leds flash on and off once
extern void led_flash(uint16_t speed);
// knight rider light effect from left to right on the board
extern void led_scan(uint16_t speed);

#endif /* LEDS_H_ */
