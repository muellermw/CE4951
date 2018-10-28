#ifndef CRC_H
#define CRC_H

#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include <stdio.h>
#include <stdbool.h>

void generate_CRC_Table();
char calculate_CRC(char *inputArray,int numberOfChars);

#endif/* CRC_H */
