#include "uart.h"
#include "csr.h"

void trap_handler(void) {
    unsigned long cause = read_csr(mcause);

    uart_puti(cause);

    unsigned long mepc_edit = read_csr(mepc) + 4;
    write_csr(mepc, mepc_edit);
}
