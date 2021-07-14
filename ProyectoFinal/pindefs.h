#ifndef PINDEFS_H_
#define PINDEFS_H_

#include <avr/io.h>

// Pin configurations
#define PIN_SCLK	(1 << PORTB5)
#define STATE_SELECT (1 << PORTB4)
#define PIN_MOSI	(1 << PORTB3)
#define PIN_DC		(1 << PORTB2)
#define PIN_RST		(1 << PORTD1)
#define STATE_LED   (1 << PORTD0)

#define SELECT_TFT()		(PORTB &= ~CS_TFT)	/* CS = L */
#define	DESELECT_TFT()		(PORTB |= CS_TFT)	/* CS = H */

#define SELECT_SD()			(PORTB &= ~CS_SD)	/* CS = L */
#define DESELECT_SD()		(PORTB |= CS_SD)	/* CS = H */

#define TOGGLE_COMMAND()	(PORTB &= ~PIN_DC)	/* D/C = L */
#define TOGGLE_DATA()		(PORTB |= PIN_DC)	/* D/C = H */

#define TFT_RST_H()			(PORTD |= PIN_RST)	// Set TFT reset high
#define TFT_RST_L()			(PORTD &= ~PIN_RST)	// Set TFT reset low

#define LED_ON()			(PORTD |= STATE_LED)
#define LED_OFF()			(PORTD &= ~STATE_LED)


#endif /* PINDEFS_H_ */