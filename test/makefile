CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -I../include -I../include/lightmodbus

ifneq ($(COVERAGE),)
CFLAGS += --coverage
endif

all: impl.c
	$(CC) $(CFLAGS) -o test test.c impl.c
	$(CC) $(CFLAGS) -o genrq genrq.c impl.c

impl.c: $(wildcard ../include/lightmodbus/*.*)
	awk 1 ../include/lightmodbus/*.impl.h > impl.c

run-test:
	./test < test-requests.txt > test-result.txt
