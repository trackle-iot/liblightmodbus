all:
	gcc -o demo demo.c -Wall -Wextra -fsanitize=address -fsanitize=undefined -fsanitize=alignment -Iinclude -Wno-unused-parameter


avr-test:
	avr-gcc -Os -c avrtest.c -Wall -mmcu=atmega328 -Iinclude -o avrtest.o
	avr-size -C --mcu=atmega328 avrtest.o