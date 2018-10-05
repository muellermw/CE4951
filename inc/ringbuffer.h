/*
 * ringbuffer header file
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <inttypes.h>

#define BUFFER_SIZE (uint32_t) 50

typedef struct {
	unsigned int put;
	unsigned int get;
	char buffer[BUFFER_SIZE];
} RingBuffer;


extern void put(RingBuffer* buffer, char element);
extern char get(RingBuffer* buffer);
extern int hasSpace(RingBuffer* buffer);
extern int hasElement(RingBuffer* buffer);


#endif /* RINGBUFFER_H_ */
