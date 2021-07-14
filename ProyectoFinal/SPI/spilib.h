#ifndef SPILIB_H_
#define SPILIB_H_

#include <stdint.h>
#include <stdbool.h>
#include "../pindefs.h"

#define wc(DATA) spi_tx(DATA, true)
#define wd(DATA) spi_tx(DATA, false)

void spi_init(void);
void spi_tx(uint8_t data, bool commandmode);
char spi_rx(void);

#endif /* SPILIB_H_ */