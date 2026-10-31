// csr.h
#ifndef CSR_H
#define CSR_H

#define read_csr(reg) ({                          \
    unsigned long __v;                            \
    asm volatile("csrr %0, " #reg : "=r"(__v));   \
    __v;                                          \
})

#define write_csr(reg, val) ({                    \
    asm volatile("csrw " #reg ", %0" : : "r"(val)); \
})

#endif
