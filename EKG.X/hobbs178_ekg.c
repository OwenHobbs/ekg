#include "xc.h"
#include "hobbs178_lcd.h"
#include "hobbs178_uart.h"
#include "stdio.h"

// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1          // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF        // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF          // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF           // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF        // JTAG Port Enable (JTAG port is disabled)

// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI       // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF       // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON       // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME      // Clock Switching and Monitor (Clock switching is enabled, 
                                       // Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL      // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

volatile int enableLCD = 0;

/**
 * Configure pin AN0 as analog input
 * Configure AD1CON registers
 * Setup TMR3
 * Enable AD1 interrupts
 */
void adc_init(void) {
    TRISAbits.TRISA0 = 1; // ensure input
    AD1PCFGbits.PCFG0 = 0; // set analog
    
    AD1CON1 = 0x0044; // unsigned integer, convert on timer 3 compare, autosample on conversion completion
    AD1CON2 = 0x0000; // using AVdd, AVss voltage, interrupts at completion of each sequence
    AD1CON3 = 0x0101; // conversion clock at 2Tcy, sample time at 1Tad
    AD1CHS = 0x0000; // configure input channels
    AD1CSSL = 0x0000; // no scanning
    
    // Setup timer
    TMR3 = 0;
    T3CON = 0;
    T3CONbits.TCKPS = 0b10;
    // Default period
    PR3 = 15624; // 1/16 seconds so 16 samples/second
    T3CONbits.TON = 1;
    
    IFS0bits.AD1IF = 0; // clear A/D conversion interrupt
    IEC0bits.AD1IE = 1; // enable A/D conversion interrupt
    AD1CON1bits.ADON = 1; // turn on A/D
}

/**
 * Transmit readings over UART
 */
void __attribute__((interrupt, auto_psv)) _ADC1Interrupt(void) {
    IFS0bits.AD1IF = 0;
    
    unsigned int adcValue = ADC1BUF0; // 10-bit value
    UART_tx(adcValue & 0xff); // send LSB first
    UART_tx((adcValue >> 8) & 0b00000011); // send two most significant bits
    
    if (enableLCD && !U1STAbits.URXDA) // important bug fix: only update display if there is no UART
        updateDisplay(PR3, adcValue);
}

void updateDisplay(unsigned int period, unsigned int adcValue) {
    char adStr[11]; // 10 character display + 1 null terminator
    
    sprintf(adStr, "PR3: %05u", period);
    lcd_setCursor(0, 0);
    lcd_printStr(adStr);
    
    sprintf(adStr, "%6.4f V", (3.3 / 1024) * adcValue);
    lcd_setCursor(0, 2);
    lcd_printStr(adStr);
}

int main(void) {
    // Initialize PIC24
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0x9fff;
    
    // Setup display
    lcd_setup();
    lcd_init(42);
    
    // Setup UART
    UART_init();
    
    // Setup ADC
    adc_init();
    
    // Listen for requests
    while (1) {
        unsigned char code = UART_rx();
        switch (code) {
            case 0b01010101: {
                // TODO: move after UART calls?
                T3CONbits.TON = 0;
                AD1CON1bits.ADON = 0;
                IFS0bits.AD1IF = 0;
                
                unsigned char newPeriodLSB = UART_rx();
                unsigned char newPeriodMSB = UART_rx();
                
                TMR3 = 0;
                PR3 = ((newPeriodMSB << 8) | newPeriodLSB) & 0xffff;
                
                T3CONbits.TON = 1;
                AD1CON1bits.ADON = 1;
                break;
            }
            case 0b00001100: {
                // Toggle LCD
                enableLCD = ~enableLCD;
                lcd_cmd(0x01); // clear display
            }
        }
    }
    
    return 0;
}
