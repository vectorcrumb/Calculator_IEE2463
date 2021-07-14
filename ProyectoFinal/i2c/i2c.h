#ifndef I2C_H_
#define I2C_H_

#define F_CPU 16000000UL

#define WRITE_ADDR(x) (x << 1)
#define READ_ADDR(x) (1 + (x << 1))

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

void i2c_init();
void i2c_send_start();
void i2c_send_stop();
void i2c_wait4complete();
void i2c_halt_module();

void i2c_tx_byte(uint8_t data);
uint8_t i2c_rx_byte(bool send_ACK);

#endif /* I2C_H_ */