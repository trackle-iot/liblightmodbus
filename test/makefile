CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -I../include

ifneq ($(COVERAGE),)
CFLAGS += --coverage
endif

all:
	$(CC) $(CFLAGS) -o test test.c
	$(CC) $(CFLAGS) -o genrq genrq.c

run-test:
	./test < test-requests.txt > test-result.txt
