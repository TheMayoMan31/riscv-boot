#include "uart.h"
#include "csr.h"

void trap_handler(void) {
    unsigned long cause = read_csr(mcause);


}
