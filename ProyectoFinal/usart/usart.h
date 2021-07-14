#ifndef USART_H_
#define USART_H_

#define F_CPU 16000000UL

#include "ringbuff.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


struct USART_configuration
{
	uint16_t baud_rate;
	uint8_t frame_size;
	uint8_t parity_bits;
	uint8_t stop_bits;
};

volatile RingBuffer tx_buff, rx_buff;

void disable_tx_int(void);
void enable_tx_int(void);

void rx_buff_push(uint8_t data);
bool tx_buff_pop(uint8_t * data);

// Initialize TX and RX circular buffers
uint8_t init_buffers(uint16_t tx_len, uint16_t rx_len);

// Convert baud rate to AVR ubbr
uint16_t baud2ubbr(uint16_t baudrate);

// Call once to initialize USART communication
uint8_t USART_Init(struct USART_configuration config);

// Transmits a single character
void USART_Transmit_char(uint8_t data );

// Transmits a given string
void USART_Transmit_String(char* string);

// Receives a single character
char USART_Receive_char(void);

// Receives a '\n' terminated string and writes it into a supplied buffer.
// The buffer must be guaranteed to handle at least bufflen bytes.
// Returns the number of bytes written into the buffer.
uint8_t USART_Receive_String(char* buffer, uint8_t bufflen);


#endif /* USART_H_ */