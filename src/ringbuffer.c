/*
 * file: ringbuffer.c
 * purpose: provides a ringbuffer for applications such as usart interrupts
 * author: Marcus Mueller
 */

#include <inttypes.h>
#include <string.h>
#include "ringbuffer.h"

// adds element to buffer.  Will block if there is no space in buffer.
void put(RingBuffer* buffer, char element) {
	while (!hasSpace(buffer)) {}
	buffer->buffer[buffer->put] = element;
	if (buffer->put == BUFFER_SIZE) {
		buffer->put = 0;
	} else {
		buffer->put++;
	}
}

// gets element from buffer.  Will block if buffer is empty.
char get(RingBuffer* buffer) {
	while (!hasElement(buffer)) {}
	char element = buffer->buffer[(buffer->get)];

	if (buffer->get == BUFFER_SIZE) {
		buffer->get = 0;
	} else {
		buffer->get++;
	}

	return element;
}

// returns true (non-zero) if there is room for one element in buffer
int hasSpace(RingBuffer* buffer) {
	int has_space = 1;
	// check if buffer is full
	if (buffer->put == (buffer->get-1) ||
			((buffer->put == BUFFER_SIZE) && (buffer->get == 0))) {
		has_space = 0;
	}
	return has_space;
}

// return true (non-zero) if there is at least one element in buffer
int hasElement(RingBuffer* buffer) {
	int has_element = 1;
	// check if buffer is empty
	if (buffer->put == buffer->get) {
		has_element = 0;
	}
	return has_element;
}
