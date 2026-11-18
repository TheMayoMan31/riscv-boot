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
		i = i - (2*i); //Turns the number positive
	}

	int n = 0;

	if (i != 0) {
	    while (i > 0) {
		    char_stack[n] = i%10 + 48; //Finds digit by find i remainder 10 to leave digit then dividing by 10 for the next digit
			i = i/10;
		    n++;
	    }

	    n--;
	} else {
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
void uart_puth(unsigned long input) {
    char hex_num[16];

    for (int i = 0; i < 16; i++) {
        hex_num[i] = '0';
    }

    int pos = 15;
    while (input != 0) {
        int hexdigit = input % 16;
        if (hexdigit < 10) {
            hex_num[pos] = hexdigit + 48;
        } else {
            hex_num[pos] = hexdigit + 55;
        }
        input = input / 16;
        pos--;
    }

    for (int i = 0; i < 16; i++) {    // 3. print all 16 columns, always
        uart_putc(hex_num[i]);
    }
}

char uart_getc(){
	volatile unsigned char *uart = (volatile unsigned char *)0x10000000;
	while ((uart[5] & 0x01) == 0);
	return uart[0];
}
