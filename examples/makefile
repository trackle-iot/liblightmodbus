CC = gcc
CFLAGS = -Wall -O2 -g -I../include

all: slave demo

clean:
	rm -f slave demo

slave: slave.c
	$(CC) $(CFLAGS) $^ -o $@

demo: demo.c
	$(CC) $(CFLAGS) $^ -o $@