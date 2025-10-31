#ifndef HOBBS178_UART
#define HOBBS178_UART

#ifdef	__cplusplus
extern "C" {
#endif  
    
void UART_init(void);
unsigned char UART_rx(void);
void UART_tx(unsigned char data);

#ifdef	__cplusplus
}
#endif

#endif	/* HOBBS178_UART */