#include "ringbuff.h"


bool ringbuff_empty(volatile RingBuffer buff) {
	// If head ptr == tail ptr, buffer has no data
	return (buff.head ==  buff.tail);
}

bool ringbuff_full(volatile RingBuffer buff) {
	return ((buff.head + 1) % buff.size) == buff.tail;
}

uint8_t ringbuff_reset(volatile RingBuffer * buff) {
	if (!buff) return 1;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		buff->head = 0;
		buff->tail = 0;
	}
	return 0;
}


uint8_t ringbuff_push(volatile RingBuffer * buff, uint8_t data) {
	if (!buff) return 1;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Write data to head location
		buff->buffer[buff->head] = data;
		// Relocate head (++ and modulo size to wrap around)
		buff->head = (buff->head + 1) % buff->size;
		// If the write ptr (head) reaches the read ptr (tail), advance tail (wrap around if needed)
		if (buff->head == buff->tail) {
			buff->tail = (buff->tail + 1) % buff->size;
		}
	}
	return 0;
}

uint8_t ringbuff_pop(volatile RingBuffer * buff, uint8_t * data) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (buff && data && !ringbuff_empty(*buff)) {
			// Get data from tail (read ptr)
			*data = buff->buffer[buff->tail];
			// Relocate tail (read ptr)
			buff->tail = ((buff->tail + 1) % buff->size);
		} else {
			return 1;
		}
	}
	return 0;
}