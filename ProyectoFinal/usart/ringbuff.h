#ifndef _RINGBUFF_H_
#define _RINGBUFF_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/atomic.h>

typedef struct ring_buffer {
	uint8_t * buffer;
	size_t head;
	size_t tail;
	size_t size;
} RingBuffer;


bool ringbuff_empty(volatile RingBuffer buff);
bool ringbuff_full(volatile RingBuffer buff);
uint8_t ringbuff_reset(volatile RingBuffer * buff);
uint8_t ringbuff_push(volatile RingBuffer * buff, uint8_t data);
uint8_t ringbuff_pop(volatile RingBuffer * buff, uint8_t * data);

#endif /* _RINGBUFF_H_ */