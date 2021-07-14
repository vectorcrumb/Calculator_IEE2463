#include "i2c.h"

void i2c_init() {
    // Set bit rate divisor to 12 for a 400 kHz SCL signal (PS = 1)
    TWBR = 12;
    // Enable TWI interface
    TWCR = (1 << TWEN);
}

void i2c_send_start() {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    i2c_wait4complete();
}

void i2c_send_stop() {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void i2c_wait4complete() {
    while (!(TWCR & (1 << TWINT)));
}

void i2c_halt_module() {
    TWCR = 0x00;
}

void i2c_tx_byte(uint8_t data) {
    _delay_us(100);
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_wait4complete();
}
uint8_t i2c_rx_byte(bool send_ACK) {
    TWCR = (1 << TWINT) |  (1 << TWEN) | (send_ACK << TWEA);
    i2c_wait4complete();
    return TWDR;
}