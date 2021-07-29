CC = gcc
CFLAGS = -Wall -O2 -g -I../include

all: slave master

clean:
	rm -f slave master

slave: slave.c
	$(CC) $(CFLAGS) $^ -o $@

master: master.c
	$(CC) $(CFLAGS) $^ -o $@