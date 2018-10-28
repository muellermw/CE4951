/*
 * receiver.h
 *
 *  Created on: Oct 13, 2018
 *      Author: muellermw
 */

#ifndef RECEIVER_H_
#define RECEIVER_H_

#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define MY_ADDRESS_11 11
#define MY_ADDRESS_12 12

typedef enum
{
	NoError,
	PreambleError,
	VersionError,
	LengthError,
	CRCflagError,
	CRCtrailerError
} receiverErrorType;

void receiver_Init();
void printAnyReceivedMessage();

#endif /* RECEIVER_H_ */
