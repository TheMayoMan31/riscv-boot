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

void uart_puth(unsigned long input) { //Write hexadecimal from long to output
    char hex_num[16];

    int n = 0;
    if (input != 0) {
        while (input != 0) {
            int hexdigit = input % 16; //Finds digits through integer division of 16
            if (hexdigit < 10) {
                hex_num[n] = hexdigit + 48;
            } else {
                hex_num[n] = hexdigit + 55;
            }

            input = input / 16;
            n++;
        }
    } else {
        for (int i = 0; i < 8; i++) {
            hex_num[i] = '0';
        }
    }

    n--;

    while (n > -1) {
        uart_putc(hex_num[n]);
        n--;
    }
}

char uart_getc(){
	volatile unsigned char *uart = (volatile unsigned char *)0x10000000;
	while ((uart[5] & 0x01) == 0);
	return uart[0];
}
