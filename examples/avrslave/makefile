MCU = atmega328p
F_CPU = 16000000L

CC = avr-gcc
DEFS = -DF_CPU=$(F_CPU) -DNDEBUG
CFLAGS = -I../../include -Os -mmcu=$(MCU) -flto $(DEFS)

all: avrslave.elf makefile

avrslave.elf: avrslave.c makefile
	$(CC) $(CFLAGS) avrslave.c -o avrslave.elf

prog: avrslave.elf
	avrdude -c usbasp -p $(MCU) -U flash:w:avrslave.elf