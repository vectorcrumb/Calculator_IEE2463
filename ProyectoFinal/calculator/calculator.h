#ifndef CUSTOMROUTINES_H_
#define CUSTOMROUTINES_H_

#define F_CPU	16000000UL

#include <util/delay.h>
#include <stdint.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>

#include "../display/ST7735_commands.h"
#include "../display/graphic_shapes.h"
#include "../tinyexpr/tinyexpr.h"
#include "../usart/usart.h"

typedef struct node{
    char valor;
    struct node  *next;
} Node;

Node * init_keypad(void);
void append(char* valor, Node* cabeza);
char * decode(Node* cabeza, char count);

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
void drawMajorAxes(uint16_t color);
uint8_t calculateFunctionPixels(uint8_t *y_vals, char *expression, double range);

#endif /* CUSTOMROUTINES_H_ */