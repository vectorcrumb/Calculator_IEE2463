#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define TX_BUFFLEN 128
#define RX_BUFFLEN 128
#define LCD_ADDR 0x3F
#define EQ_BUFF_LENGTH 32

//#define SERIAL_DEBUG
//#define DRAW_POINTS

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "pindefs.h"
#ifdef SERIAL_DEBUG
#include "usart/usart.h"
#endif
#include "i2c/i2c.h"
#include "SPI/spilib.h"

#include "calculator/calculator.h"
#include "tinyexpr/tinyexpr.h"
#include "lcd_i2c/lcd_i2c.h"
#include "display/ST7735_commands.h"
#include "display/graphic_shapes.h"

void errorHalt(char* msg);
void lcd_moveCursor(uint8_t x, uint8_t y);
const char equals_sign[] = "=";
char teclas[17] = {'x', '/', '=', '0', '.', '*', '9', '8', '7', '-', '6','5','4','+','3','2','1'};
char teclas_extra[16][7] =	{"pi", "d", ")", "(", "log10(", "sqrt(", "^", "x", "ln(", "atan(", "acos(", "asin(", "exp(", "tan(", "cos(", "sin("};

uint8_t pi_char[8] = {
	0b00000,
	0b00000,
	0b11111,
	0b01010,
	0b01010,
	0b01010,
	0b10011,
	0b00000,
};

volatile bool equals_flag = false;
volatile bool plot_mode = false;
volatile uint16_t keypad_ll_len = 0;
volatile uint8_t keypad_button_index = 0;
volatile uint8_t second_keypad=0;
uint8_t lcd_pos[] = {0, 0};
Node * keypad_ll;
char * plot_operation;
volatile bool ask_for_range = false;


int main(void) {
    // Init USART
#ifdef SERIAL_DEBUG
    init_buffers(TX_BUFFLEN, RX_BUFFLEN);
    struct USART_configuration config = {BAUD_RATE, 8, 0, 1};
    USART_Init(config);
    _delay_ms(10);
#endif
    // Init I2C and LCD
    i2c_init();
    lcd_init(LCD_ADDR);
    lcd_begin(16, 2, LCD_5x8DOTS);
	lcd_createChar(0, pi_char);
    // Init SPI and TFT
    spi_init();
    ST7735_init();
    // Init keypads
    keypad_ll = init_keypad();
    // Activate interrupts
    sei();
    // Configure TFT
    fillScreen(ST7735_BACKGROUND);
    drawMajorAxes(ST7735_WHITE);
    // Configure LCD
    lcd_setBacklight(255);
    lcd_home();
    lcd_clear();
    
    uint8_t y_vals[TFT_WIDTH];
    
    //Main loop
    while (true) {
        // Check for plot or calc mode
        plot_mode = PINB & STATE_SELECT;
        if (plot_mode) {
            LED_ON();
        } else {
            LED_OFF();
            ask_for_range=false;
        }
        // Is an operation result queued?
        if (equals_flag) {
            // Plot mode
            if (plot_mode) {
                // Function to plot input
                if (!ask_for_range) {
                    plot_operation = decode(keypad_ll, keypad_ll_len);
                    ask_for_range = true;
                    lcd_setCursor(0,1);
                    lcd_print("Rango:");
                    lcd_setCursor(7, 1);
                }
                // Range received, plot function
                else {
                    char * range_str = decode(keypad_ll, keypad_ll_len);
                    double range_val = atof(range_str);
                    if (range_val == 0.0){
                        // Error state.
                        lcd_home();
                        lcd_clear();
                        lcd_print("Error en rango.");
                    } else {
                        uint8_t err;
                        err = calculateFunctionPixels(y_vals, plot_operation, range_val);
                        if (err) {
                            // Error state.
                            lcd_home();
                            lcd_clear();
                            lcd_print("Error en funcion");
                        } else {
                            fillScreen(ST7735_BACKGROUND);
                            drawMajorAxes(ST7735_WHITE);
                            #ifdef DRAW_POINTS
                            for (int i = 0; i < TFT_WIDTH; i++) {
                                drawPixel(i, y_vals[i], ST7735_OLDGREEN);
                            }                            
                            #else
                            for (int i = 1; i < TFT_WIDTH; i++) {
                                drawLine(TFT_WIDTH - i, y_vals[i - 1], TFT_WIDTH - (i + 1), y_vals[i], ST7735_OLDGREEN);
                            }                        
                            #endif
                        }
                    }                                           
                    free(plot_operation);
                    free(range_str);
                    ask_for_range = false;        
                }
                keypad_ll_len = 0;
                equals_flag = false;
            }
            // Calc Mode
            else {
                ask_for_range = false; 
                char * operation = decode(keypad_ll, keypad_ll_len);
    #ifdef SERIAL_DEBUG
                USART_Transmit_String(operation);
    #endif
                // Print equals sign
                lcd_setCursor(15, 0);
                lcd_print(equals_sign);
                lcd_setCursor(0, 1);
                // Evaluate expression
                int err_flag = 0;
                double res = te_interp(operation, &err_flag);
                // If error, display NaN on LCD
                if(err_flag) {
                    lcd_print("NaN");
                } else {
                    char sres[16];
                    // TODO: Define display logic
                    if (res > 10e6) {
                        dtostre(res, sres, 2, 0x04);
                    } else {
                        dtostrf(res, 3, 2, sres);
                    }
                    lcd_print(sres);
    #ifdef SERIAL_DEBUG
                    USART_Transmit_char('=');
                    USART_Transmit_String(sres);
                    USART_Transmit_char('\n');
    #endif
                }
                keypad_ll_len = 0; 
                equals_flag = false;
                free(operation);
            }           
        }             
    }
}

#ifdef SERIAL_DEBUG
ISR (USART_UDRE_vect) {
    uint8_t outgoing;
    bool tx_buff_state = tx_buff_pop(&outgoing);
    if (tx_buff_state) {
        UDR0 = outgoing;
    }
}

ISR (USART_RX_vect) {
    uint8_t incoming = UDR0;
    rx_buff_push(incoming);
}
#endif

ISR	(PCINT0_vect){
    TCCR0B = (1 << CS01) + (1 << CS00);
}

ISR (PCINT1_vect){
	TCCR0B = (1 << CS01) + (1 << CS00);
}

ISR (PCINT2_vect){
    TCCR0B = (1 << CS01) + (1 << CS00);
}

ISR (TIMER0_OVF_vect){
    TCCR0B = 0;
    TCNT0 = 0;
    keypad_button_index=0;
	second_keypad=0;
    
    if (!(PIND & (1 << 6))){
        keypad_button_index = 1;
    }
    else if (!(PIND & (1 << 7))){
        keypad_button_index = 5;
    }
    else if (!(PINB & (1 << 0))){
        keypad_button_index = 9;
    }
    else if (!(PINB & (1 << 1))){
        keypad_button_index = 13;
    }
    // Input columns, output rows
	DDRD &= ~((1<<2)  + (1<<3) + (1<<4) + (1<<5));
	DDRC &= ~((1<<0)  + (1<<1) + (1<<2) + (1<<3));
	DDRD |= (1<<6) + (1<<7);
	DDRB |= (1<<0) + (1<<1);
	PORTD |= (1<<2)  + (1<<3) + (1<<4) + (1<<5);
	PORTC |= (1<<0)  + (1<<1) + (1<<2) + (1<<3);
	PORTD &= ~((1<<6) + (1<<7));
	PORTB &= ~((1<<0) + (1<<1));
    _delay_us(20);
    
	if (keypad_button_index!=0){
		if (!(PIND&(1<<2))){
			keypad_button_index += 0;
			second_keypad=0;
		}
		else if (!(PIND&(1<<3))){
			keypad_button_index += 1;
			second_keypad=0;
		}
		else if (!(PIND&(1<<4))){
			keypad_button_index += 2;
			second_keypad=0;
		}
		else if (!(PIND&(1<<5))){
			keypad_button_index += 3;
			second_keypad=0;
		}
		
		else if (!(PINC&(1<<3))){
			keypad_button_index += 0;
			second_keypad=1;
		}
		else if (!(PINC&(1<<2))){
			keypad_button_index += 1;
			second_keypad=1;
		}
		else if (!(PINC&(1<<1))){
			keypad_button_index += 2;
			second_keypad=1;
		}
		else if (!(PINC&(1<<0))){
			keypad_button_index += 3;
			second_keypad=1;
		}
	}
    // Input rows, output columns
	DDRD &= ~((1<<6) + (1<<7));
	DDRB &= ~((1<<0) + (1<<1));
	DDRD |= (1<<2)  + (1<<3) + (1<<4) + (1<<5);
	DDRC |= (1<<0)  + (1<<1) + (1<<2) + (1<<3);
	PORTD &= ~((1<<2) + (1<<3) + (1<<4) + (1<<5));
	PORTC &= ~((1<<0)  + (1<<1) + (1<<2) + (1<<3));
	PORTD |= (1<<6) + (1<<7);
	PORTB |= (1<<0) + (1<<1);
    
    _delay_us(30);
    PCIFR = (1<<PCIF2) + (1<<PCIF1) + (1<<PCIF0);
    
	char *keypad_input=&teclas[keypad_button_index];
	char *keypad_input_extra=teclas_extra[keypad_button_index-1];
    
    if ((!((*keypad_input=='=')&!keypad_ll_len))){

		if ((*keypad_input == '=')&(!second_keypad)){
			equals_flag=1;
		}
        // Keypad has been debounced
		if (*keypad_input != 'x'){
            // Clear conditions
			if (!keypad_ll_len && !ask_for_range){
                lcd_clear();
            }               
            if(!second_keypad||strcmp(teclas_extra[keypad_button_index-1], "x")||(plot_mode)) {
			    // Process first keypad
                if ((strcmp(teclas_extra[keypad_button_index-1], "d"))|!second_keypad){
			        if (!second_keypad){
				        if (*keypad_input!='='){
					        char * s_disp = (char *)malloc(2);
					        if (s_disp == NULL) errorHalt("Allocation\n");
					        *s_disp = *keypad_input; *(s_disp+1) = '\0';
					        lcd_print(s_disp);
					        free(s_disp);
				        }
				        keypad_ll_len++;
				        append(keypad_input, keypad_ll);
			        }
                    // Process second keypad
			        else {
                        if (strcmp(teclas_extra[keypad_button_index-1], "pi")){
                            // NO ES PI
                            lcd_print(teclas_extra[keypad_button_index-1]);
                        }
                        else{
                            write((uint8_t)0);
                        }
                        while(*keypad_input_extra!='\0'){
                        append(keypad_input_extra, keypad_ll);
                        keypad_ll_len++;
                        keypad_input_extra++;
                        }                                    
			        }
                }                
                else {
                    *keypad_input='=';
                    append(keypad_input, keypad_ll);
                    keypad_ll_len++;
                    char * operation = decode(keypad_ll, keypad_ll_len);
                    lcd_clear();
                    free(operation);
                    keypad_ll_len=0;
                }            
            }            
		}
	}
}

void errorHalt(char* msg) {
#ifdef SERIAL_DEBUG
    USART_Transmit_String("Error: ");
    USART_Transmit_String(msg);
    USART_Transmit_String("\n");
#endif
    LED_OFF();
    while(true) {
        LED_ON();
        _delay_ms(100);
        LED_OFF();
        _delay_ms(100);
    }
}

void lcd_moveCursor(uint8_t x, uint8_t y) {
    lcd_setCursor(x, y);
    lcd_pos[0] = x; lcd_pos[1] = y;
}
