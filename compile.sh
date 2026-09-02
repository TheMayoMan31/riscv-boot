riscv64-elf-gcc -mcmodel=medany -ffreestanding -c main.c -o main.o

riscv64-elf-gcc -mcmodel=medany -c boot.S -o boot.o

riscv64-elf-gcc -mcmodel=medany -c uart.c -o uart.o

riscv64-elf-ld -T linker.ld uart.o boot.o main.o -o kernel.elf
