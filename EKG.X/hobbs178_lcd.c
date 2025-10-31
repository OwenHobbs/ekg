#include "xc.h"
#include "hobbs178_lcd.h"

/**
 * Initialize PIC24FJ64GA002 output pins,
 * reset LCD, and configure I2C.
 */
void lcd_setup(void) {    
    // Configure output pins
    TRISBbits.TRISB5 = 0; // LED
    TRISBbits.TRISB6 = 0; // LCD pin 20 (~reset)
    
    // Set trigger pin to off
    LATBbits.LATB5 = 0;
    
    // Reset the display
    LATBbits.LATB6 = 0;
    asm("repeat #15998"); // wait 1 ms for display to reset
    asm("nop");
    LATBbits.LATB6 = 1;
    
    // Configure I2C
    I2C1CONbits.I2CEN = 0;
    I2C1BRG = 157; // operate at 100 kHz
    I2C1CONbits.DISSLW = 0; // enable slew rate control
    I2C1CONbits.A10M = 0; // use 7-bit slave address
    IFS1bits.MI2C1IF = 0; // reset master event interrupt flag
    I2C1CONbits.I2CEN = 1;
}

/**
 * Helper function for I2C transaction with LCD.
 * Used by other functions.
 * @param addressByte
 * @param controlByte
 * @param commandByte
 */
void lcd_packet(char addressByte, char controlByte, char commandByte) {
    // Turn on LED for analyzer
    LATBbits.LATB5 = 1;
    
    // Send START bit
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);
    
    // Address frame/byte
    IFS1bits.MI2C1IF = 0;
    I2C1TRN = addressByte; // 7-bit slave address and R/~W bit
    while (!IFS1bits.MI2C1IF || I2C1STATbits.TBF || I2C1STATbits.ACKSTAT);
    
    // Control frame/byte
    IFS1bits.MI2C1IF = 0;
    I2C1TRN = controlByte;
    while (!IFS1bits.MI2C1IF || I2C1STATbits.TBF || I2C1STATbits.ACKSTAT);
    
    // Command/Data frame/byte
    IFS1bits.MI2C1IF = 0;
    I2C1TRN = commandByte;
    while (!IFS1bits.MI2C1IF || I2C1STATbits.TBF || I2C1STATbits.ACKSTAT);
    
    // Send STOP bit
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
    
    // Turn off LED for analyzer
    LATBbits.LATB5 = 0;
}

/**
 * Helper function to send command to LCD.
 * See data sheet for commands. 
 * @param commandByte
 */
void lcd_cmd(char commandByte) {
    // addressByte: 0111 110 0->write
    // pin SA0 = 0
    lcd_packet(0b01111000, 0b00000000, commandByte);
}

/**
 * Send commands to LCD for initialization.
 * Contrast can be set as desired.
 * @param contrast range 0-63
 */
void lcd_init(char contrast) {
    // From Initialization Example in data sheet
    
    lcd_cmd(0x3A); // function set RE = 1
    lcd_cmd(0x09); // 4 line display
    lcd_cmd(0x06); // bottom view
    lcd_cmd(0x1E); // BS1=1
    
    lcd_cmd(0x39); // function set RE = 0, IS = 1
    lcd_cmd(0x1B); // BS0=1
    lcd_cmd(0x6E); // Divider on and set value
    // 9.14 Power/ Icon Control/ Contrast set
    // Upper nibble 0b0101 = 0x5
    // DB3: ICON display off 0 
    // DB2: DCDC converter on 1
    // DB1: C5
    // DB0: C4 
    lcd_cmd(0x54 | ((contrast & 0x30) >> 4));
    // 9.16 Contrast Set
    // Upper nibble 0b0111 = 0x7
    // Lower nibble is C3,C2,C1,C0
    lcd_cmd(0x70 | (contrast & 0x0F));
    
    lcd_cmd(0x38); // function set RE = 0
    lcd_cmd(0x0F); // display on, cursor on, blink on
    
    // Configure screen for two-line, double height mode
    
    lcd_cmd(0x3a); // 0011 1010
    lcd_cmd(0x09); // 0000 1001
    lcd_cmd(0x1a); // 0001 1010
    lcd_cmd(0x3c); // 0011 1100
    lcd_cmd(0x01); // 0000 0001 clear display
}

/**
 * Set the position of where to print next char
 * @param x range 0-9 for columns
 * @param y 0 for top row, 2 for bottom row
 */
void lcd_setCursor(char x, char y) {
    // 8-bit command: 0b1yyyxxxx
    lcd_cmd(((y << 4) | x) | 0x80);
}

/**
 * Send one character to display.
 * Hardware will automatically increment cursor.
 * @param myChar
 */
void lcd_printChar(char myChar) {
    // controlByte
    // bit 7: Co = 0
    // bit 6: RS or D/~C (data or not command) = 1
    lcd_packet(0b01111000, 0b01000000, myChar);
}

/**
 * Print each character of a string until null terminator.
 * @param str null terminated C string
 */
void lcd_printStr(const char *str) {
    unsigned int offset = 0;
    while (str[offset] != '\0') {
        lcd_printChar(str[offset]);
        offset++;
    }
}