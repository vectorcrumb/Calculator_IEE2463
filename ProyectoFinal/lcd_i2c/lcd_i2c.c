#include "lcd_i2c.h"

/// These are Bit-Masks for the special signals and background light
#define PCF_RS  0x01
#define PCF_RW  0x02
#define PCF_EN  0x04
#define PCF_BACKLIGHT 0x08

// Definitions on how the PCF8574 is connected to the LCD
// These are Bit-Masks for the special signals and Background
#define RSMODE_CMD  0
#define RSMODE_DATA 1

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

// modification:
// don't use ports from Arduino, but use ports from Wire

// a nibble is a half Byte

// NEW: http://playground.arduino.cc//Code/LCDAPI
// NEW: setBacklight


void lcd_init(uint8_t addr) {
    _Addr = addr;
    _backlight = 0;
}

void lcd_begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
    // cols ignored !
    _numlines = lines;

    _displayfunction = 0;

    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befor 4.5V so we'll wait 50
    i2c_init();

    // initializing the display
    _write2Wire(0x00, 0x0, false);
    _delay_us(50000);

    // put the LCD into 4 bit mode according to the hitachi HD44780 datasheet figure 26, pg 47
    _sendNibble(0x03, RSMODE_CMD);
    _delay_us(4500); 
    _sendNibble(0x03, RSMODE_CMD);
    _delay_us(4500); 
    _sendNibble(0x03, RSMODE_CMD);
    _delay_us(150);
    // finally, set to 4-bit interface
    _sendNibble(0x02, RSMODE_CMD);

    // finally, set # lines, font size, etc.
    _command(LCD_FUNCTIONSET | _displayfunction);  

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
    lcd_display();

    // clear it off
    lcd_clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    _command(LCD_ENTRYMODESET | _displaymode);
}

void lcd_clear() {
    _command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    _delay_us(2000);  // this command takes a long time!
}

void lcd_home()
{
    _command(LCD_RETURNHOME);  // set cursor position to zero
    _delay_us(2000);  // this command takes a long time!
}

/// Set the cursor to a new position. 
void lcd_setCursor(uint8_t col, uint8_t row)
{
    int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if ( row >= _numlines ) {
        row = _numlines-1;    // we count rows starting w/0
    }
    _command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void lcd_noDisplay() {
    _displaycontrol &= ~LCD_DISPLAYON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_display() {
    _displaycontrol |= LCD_DISPLAYON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void lcd_noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  _command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_cursor() {
  _displaycontrol |= LCD_CURSORON;
  _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void lcd_noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  _command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_blink() {
  _displaycontrol |= LCD_BLINKON;
  _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void lcd_scrollDisplayLeft(void) {
  _command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void lcd_scrollDisplayRight(void) {
  _command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void lcd_leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  _command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void lcd_rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  _command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void lcd_autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  _command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void lcd_noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  _command(LCD_ENTRYMODESET | _displaymode);
}

/// Setting the brightness of the background display light.
/// The backlight can be switched on and off.
/// The current brightness is stored in the private _backlight variable to have it available for further data transfers.
void lcd_setBacklight(uint8_t brightness) {
  _backlight = brightness;
  // send no data but set the background-pin right;
  _write2Wire(0x00, RSMODE_DATA, false);
} // setBacklight

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void lcd_createChar(uint8_t location, uint8_t charmap[]) {
    location &= 0x7; // we only have 8 locations 0-7
    _command(LCD_SETCGRAMADDR | (location << 3));
    for (int i=0; i<8; i++) {
        write(charmap[i]);
    }
}


size_t lcd_print_shift(const char * s, uint8_t row) {
    size_t n = lcd_print(s);
    lcd_setCursor(strlen(s), row);
    return n;
}

size_t lcd_print(const char * s) {
    return lcd_write(s, strlen(s));
}

size_t lcd_write(const char * buffer, size_t size)
{
    size_t n = 0;
    while (size--) {
        if (write(*buffer++)) n++;
        else break;
    }
    return n;
}


/* The write function is needed for derivation from the Print class. */
size_t write(uint8_t value) {
    _send(value, RSMODE_DATA);
    return 1; // assume sucess
}

/* ----- low level functions ----- */
void _command(uint8_t value) {
    _send(value, RSMODE_CMD);
} // _command()

// write either command or data
void _send(uint8_t value, uint8_t mode) {
    // separate the 4 value-nibbles
    uint8_t valueLo = value    & 0x0F;
    uint8_t valueHi = value>>4 & 0x0F;

    _sendNibble(valueHi, mode);
    _sendNibble(valueLo, mode);
} // _send()

// write a nibble / halfByte with handshake
void _sendNibble(uint8_t halfByte, uint8_t mode) {
    _write2Wire(halfByte, mode, true);
     _delay_us(1);    // enable pulse must be >450ns
    _write2Wire(halfByte, mode, false);
     _delay_us(37);   // commands need > 37us to settle
} // _sendNibble

// private function to change the PCF8674 pins to the given value
void _write2Wire(uint8_t halfByte, uint8_t mode, uint8_t enable) {
    // map the given values to the hardware of the I2C schema
    uint8_t i2cData = halfByte << 4;
    if (mode > 0) i2cData |= PCF_RS;
    // PCF_RW is never used.
    if (enable > 0) i2cData |= PCF_EN;
    if (_backlight > 0) i2cData |= PCF_BACKLIGHT;

    i2c_send_start();
    i2c_tx_byte(WRITE_ADDR(_Addr));
    i2c_tx_byte(i2cData);
    i2c_send_stop();
    i2c_halt_module();

} // write2Wire