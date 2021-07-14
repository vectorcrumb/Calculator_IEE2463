#include "usart.h"


uint8_t init_buffers(uint16_t tx_len, uint16_t rx_len) {
	tx_buff.size = tx_len;
	rx_buff.size = rx_len;
	tx_buff.buffer = malloc(tx_len);
	rx_buff.buffer = malloc(rx_len);
	if (tx_buff.buffer == NULL || rx_buff.buffer == NULL) {
		free(tx_buff.buffer);
		free(rx_buff.buffer);
		return 1;
	}
	ringbuff_reset(&tx_buff);
	ringbuff_reset(&rx_buff);
	return 0;
}

uint16_t baud2ubbr(uint16_t baudrate) {
	return F_CPU/16/baudrate-1;
}

void disable_tx_int(void) {
	UCSR0B &= ~(1 << UDRIE0);
}

void enable_tx_int(void) {
	UCSR0B |= (1 << UDRIE0);
}

void rx_buff_push(uint8_t data) {
	ringbuff_push(&rx_buff, data);
}

bool tx_buff_pop(uint8_t * data) {
	if (ringbuff_empty(tx_buff)) {
		disable_tx_int();
		return false;
	}
	ringbuff_pop(&tx_buff, data);
	return true;
}

uint8_t USART_Init(struct USART_configuration config) {
	// Clear control register C
	UCSR0C = 0;
	uint8_t ERR_FLAG = 0;
	uint16_t ubbr = baud2ubbr(config.baud_rate);
	// Bitshift MSB to LSB in baudrate and write to HIGH register
	UBRR0H = (uint8_t) (ubbr >> 8);
	// Right LSB from baudrate to LOW register
	UBRR0L = (uint8_t) ubbr;
	// Enable transmit and receive module
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
	// Enable interrupts
	UCSR0B |= (1 << UDRIE0) | (1 << RXCIE0);
	switch (config.frame_size) {
		case 5:
			// 5 bits is configured as 000
			break;
		case 6:
			UCSR0C |= (1 << UCSZ00);
			break;
		case 7:
			UCSR0C |= (1 << UCSZ01);
			break;
		case 8:
			UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
			break;
		case 9:
			UCSR0C |= (1 << UCSZ02) | (1 << UCSZ01) | (1 << UCSZ00);
			break;
		default:
			ERR_FLAG = 1;
			break;
	}
	switch (config.parity_bits) {
		case 0:
			// default state, pass
			break;
		case 1:
			UCSR0C |= (1 << UPM01);
			break;
		case 2:
			UCSR0C |= (1 << UPM01) | (1 << UPM00);
			break;
		default:
			ERR_FLAG = 2;
			break;
	}
	switch (config.stop_bits) {
		case 1:
			// default state, pass
			break;
		case 2:
			UCSR0C |= (1 << USBS0);
			break;
		default:
			ERR_FLAG = 3;
			break;
	}
	disable_tx_int();
	return ERR_FLAG;
}



// Transmits a single character
void USART_Transmit_char(uint8_t data) {
	// A nice hint: With interrupts, you can send bytes whenever the register UDR0
	// is free. And there is an interrupt called USART_UDRE_vect that *tells you*
	// whenever UDR0 is free.
	// This requires you to have some bytes in the buffer that you would like to
	// send, of course. You have a buffer, don't you?
	ringbuff_push(&tx_buff, data);
	enable_tx_int();
}



// Transmits a given string
void USART_Transmit_String(char* string) {
	bool first_run = true;
	while (*string) {
		ringbuff_push(&tx_buff, *string);
		if (first_run) {
			//enable_tx_int();
			first_run = false;
		}
		string++;
	}
	enable_tx_int();
}


// Receives a single character.
char USART_Receive_char(void) {
	if (ringbuff_empty(rx_buff)) return 0;
	uint8_t data = 0;
	ringbuff_pop(&rx_buff, &data);
	return data;
}



// Receives a '\n' terminated string and writes it into a supplied buffer.
// The buffer must be guaranteed to handle at least bufflen bytes.
// Returns the number of bytes written into the buffer.
uint8_t USART_Receive_String(char* buffer, uint8_t bufflen) {
	uint8_t data = 0;
	uint8_t ctr = 0;
	for (int i = 0; i <= bufflen; i++) {
		if (ringbuff_empty(rx_buff)) return ctr;
		ringbuff_pop(&rx_buff, &data);
		buffer[i] = data;
		ctr++;
		if (data == '\n') disable_tx_int();
	}
	return ctr;
} 