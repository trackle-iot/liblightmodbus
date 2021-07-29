CC = gcc
CFLAGS = -Wall -O2 -g -I../include

all: slave master demo

clean:
	rm -f slave master demo

slave: slave.c
	$(CC) $(CFLAGS) $^ -o $@

master: master.c
	$(CC) $(CFLAGS) $^ -o $@

demo: demo.c
	$(CC) $(CFLAGS) $^ -o $@