void uart_putc(char c) {
	volatile unsigned char *uart = (volatile unsigned char *)0x10000000;
	while ((uart[5] & 0x20) == 0);
        uart[0] = c;	
}

void uart_puts(char *s) {
	while(*s) {
		uart_putc(*s++);	
     	}		
}
	
void uart_puti(int i) {
	int max_size = 21;
	char char_stack[max_size];
	
	char sign = ' ';
	if (i < 0) {
		sign = '-';
		i = i - (2*i);
	}
		
	int n = 0;
	while (i>0) {
		char_stack[n] = i%10 + 48;
		i = i/10;
		n++;
	}

	n--; 	

	if (i == 0) {
		char_stack[0] = '0';
		n++;
	}

	if (sign == '-') {
		uart_putc('-');
	}

	while (n > -1) {
		uart_putc(char_stack[n]);
		n--;
	}
	
 	
}

char uart_getc(){
	volatile unsigned char *uart = (volatile unsigned char *)0x10000000;
	while ((uart[5] & 0x01) == 0);
	return uart[0];
}
