.section .text.boot
.global _start

_start:
    .option push
    .option norelax
    lla sp, stack_top    # Changed from la to lla
    .option pop

    call main
loop:
    j loop

.section .bss
stack_bottom:
    .skip 4096
stack_top:
