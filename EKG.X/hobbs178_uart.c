#include "xc.h"
#include "hobbs178_uart.h"

void UART_init(void) {
        // Unlock registers
    __builtin_write_OSCCONL(OSCCON & 0xBF);
    // Map output peripheral U1TX to RP15
    RPOR7bits.RP15R = 3;
    // Map input peripheral U1RX to RP13
    RPINR18bits.U1RXR = 13;
    // Lock registers
    __builtin_write_OSCCONL(OSCCON | 0x40);
    
    // U1BRG = 103; // 9600 baud
    U1BRG = 34; // 115200 baud
    U1MODEbits.UEN = 0; // RXD, TXD only
    U1MODEbits.BRGH = 1; // only for 115200 baud
    U1MODEbits.PDSEL = 0; // 8-bits, no parity
    U1MODEbits.STSEL = 0; // 1 stop bit
    U1MODEbits.UARTEN = 1; // enable UART1
    
    U1STAbits.UTXEN = 1; // enable TX
}

unsigned char UART_rx(void) {
	while (!U1STAbits.URXDA); // wait until is RX buffer full
	return U1RXREG; // return data
}

void UART_tx(unsigned char data) {
	while (U1STAbits.UTXBF); // wait if TX buffer (want open spot)
	U1TXREG = data; // send data
}
