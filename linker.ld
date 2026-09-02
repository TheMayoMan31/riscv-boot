ENTRY(_start)

SECTIONS {
    . = 0x80000000;
    
    .text : {
        *(.text.boot)
        *(.text)
    }
    
    . = ALIGN(8);
    .rodata : { *(.rodata .rodata.*) }
    
    . = ALIGN(8);
    .data : { *(.data .data.*) }
    
    . = ALIGN(16);
    .bss : { *(.bss .bss.*) }
}
