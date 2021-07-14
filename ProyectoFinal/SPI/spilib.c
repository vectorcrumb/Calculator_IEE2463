#include "spilib.h"


void spi_init(void) {
    // Configure SCLK, CS_SD, CS_TFT, MOSI, D/C as out, MISO as in (pull up)
    DDRB |= PIN_SCLK | PIN_MOSI | PIN_DC;
    DDRB &= ~STATE_SELECT;
    PORTB |= STATE_SELECT;
    // Enable SPI, master, SCK @ 4MHz (max 20MHz) (000)
    SPCR = (1 << SPE) | (1 << MSTR);
    // Enable SPI 2X to run @ 8MHz (100)
    SPSR |= (1 << SPI2X);
}

// This function transmits a single byte over the SPI bus.
// It does *not* control the CS line
void spi_tx(uint8_t data, bool commandmode) {
    // CM true -> D/C low
    if (commandmode) {
        TOGGLE_COMMAND();
    } else {
        TOGGLE_DATA();
    }
    SPDR = data;
    while(!(SPSR & (1 << SPIF)));
}

// This function receives a single byte over the SPI bus.
// This is very easy and short if you understood how SPI works.
// Hint: It is a *full duplex* bus!
char spi_rx(void) {
    SPDR = 0xFF;
    while(!(SPSR & (1 << SPIF)));
    return SPDR;
}