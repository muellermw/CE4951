/*
 * channel_monitor header file
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_
#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include <stdio.h>

typedef enum{
	IDLE_STATE,
	BUSY_STATE,
	COLLISION_STATE
} state_enum ;

extern void channelMonitorInit();


#endif /* CHANNEL_H_ */
