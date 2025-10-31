#ifndef HOBBS178_LCD
#define HOBBS178_LCD

#ifdef	__cplusplus
extern "C" {
#endif  
    
void lcd_setup(void);
void lcd_packet(char addressByte, char controlByte, char commandByte);
void lcd_cmd(char commandByte);
void lcd_init(char contrast);
void lcd_setCursor(char x, char y);
void lcd_printChar(char myChar);
void lcd_printStr(const char *str);

#ifdef	__cplusplus
}
#endif

#endif	/* HOBBS178_LCD */