#include "uart.h"
#include "csr.h"
#include "trap.h"
#include "math.h"

void uart_init() {
  volatile unsigned char *uart = (volatile unsigned char *)0x10000000;
  uart[3] = 0x80;
  uart[0] = 0x03;
  uart[1] = 0x00;
  uart[3] = 0x03;
  uart[2] = 0xC7;
}

struct commands {
  int ID;
  char *Name;
};

void dump(unsigned long address, unsigned long length) {
    for (int i = 0; i < length; i++) {
        unsigned char b = *(volatile unsigned char *)(address + i);
        uart_puth(b);
    }
}

void extractCMD(char *input, char *cmd) {
  int i = 0;
  while (input[i] != ' ' && input[i] != '\0') {
    i++;
  }

  for (int j = 0; j < i; j++) {
    cmd[j] = input[j];
  }

  cmd[i] = '\0';
}

bool commandChecker(char *cmdToCheck, char *cmd) {
  int i = 0;
  while (cmdToCheck[i] == cmd[i]) {
    if (cmdToCheck[i] == '\0') {
      return true;
    }
    i++;
  }
  return false;
}

void main() {

  write_csr(mtvec, (unsigned long)trap_entry);

  struct commands Commands[5];
  for (int i = 0; i < 4; i++) {
    Commands[i].ID = i;
    switch (i) {
    case 0:
      Commands[i].Name = "help";
      break;
    case 1:
      Commands[i].Name = "ls";
      break;
    case 2:
      Commands[i].Name = "echo";
      break;
    case 3:
      Commands[i].Name = "cd";
      break;
    case 4:
      Commands[i].Name = "dump";
      break;
    }
  }

  uart_init();
  volatile unsigned char *uart = (volatile unsigned char *)0x10000000;
  char *hello = "Hello World\n";

  char input[20];

  while (true) {
    for (int i = 0; i < 20; i++) {
      input[i] = '\0';
    }

    char char_in = '\0';

    int n = 0;
    char *start = "\n> ";
    uart_puts(start);


    while (char_in != '\r') {
      char_in = uart_getc();
      if (char_in == 8 || char_in == 127) {
        if (n > 0) {
          n--;
          input[n] = '\0';
          uart_putc('\b');
          uart_putc(' ');
          uart_putc('\b');
        }
      } else if (char_in == '\r') {
      } else {
        if (n <= 19) {
          input[n] = char_in;
          uart_putc(input[n]);
          n++;
        }
      }
    }
    uart_putc('\n');

    char cmd[20];
    extractCMD(input, cmd);

    int idCMD = 999;

    for (int i = 0; i < 5; i++) {
      if (commandChecker(cmd, Commands[i].Name)) {
        idCMD = i;
        break;
      }
    }

    switch (idCMD) {
    case 0:
      // help
      uart_puts("Commands: help, ls, echo, cd, dump");
      break;
    case 1:
      // ls
      //  REQUIRES FILE SYSTEM
      break;
    case 2:
      // echo
      uart_puts(&input[5]);
      break;
    case 3:
      // cwrite_csr(mtvec, (unsigned long)trap_entry);d
      // REQUIRES FILE SYSTEM
      break;
    case 4:
        // dump
        char *p = &input[5];
        int index = 0;
        char hex_num[8];
        unsigned long hex = 0;

        int bytes = 8;
        if (*p==48 && *(++p)==120) {
            ++p;
            while (((*p > 47 && *p < 58) || (*p>64 && *p<71)) && index < 8) {
                hex_num[index] = *p;
                p++;
                index++;
            }
            while (*p==' ') {
                p++;
            }


            int byte_temp = 0;
            while(*p > 47 && *p < 58) {
                byte_temp = byte_temp*10 + (*p-48);
                p++;
            }
            if (byte_temp > 0) {
                bytes = byte_temp;
            }

            if (byte_temp > 256) { uart_puts("Too long"); break; }

            //little endian
            for (int i = 0; i<index; i++) {
                if (hex_num[i] > 47 && hex_num[i] < 58) {
                    hex = hex*16 + (hex_num[i]-48);
                } else {
                    hex = hex*16 + (hex_num[i]-55);
                }
            }

            dump(hex, bytes);


        } else {
            uart_puts("Invalid command");
        }

        break;
    }
    }
  while (1);
}
