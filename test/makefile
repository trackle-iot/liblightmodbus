CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -I../include -I../include/lightmodbus -DLIGHTMODBUS_FULL

ifneq ($(COVERAGE),)
CFLAGS += --coverage
endif

IMPL = \
	-x c ../include/lightmodbus/base.impl.h \
	-x c ../include/lightmodbus/slave.impl.h \
	-x c ../include/lightmodbus/slave_func.impl.h \
	-x c ../include/lightmodbus/master.impl.h \
	-x c ../include/lightmodbus/master_func.impl.h

all: 
	$(CC) $(CFLAGS) -o test test.c $(IMPL)
	$(CC) $(CFLAGS) -o genrq genrq.c $(IMPL)

run-test:
	./test < test-requests.txt > test-result.txt
