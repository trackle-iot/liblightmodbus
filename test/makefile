CC = gcc
DEFS = -DLIGHTMODBUS_FULL -DLIGHTMODBUS_SLAVE -DLIGHTMODBUS_F01S -DLIGHTMODBUS_F02S -DLIGHTMODBUS_F03S -DLIGHTMODBUS_F04S -DLIGHTMODBUS_F05S -DLIGHTMODBUS_F06S -DLIGHTMODBUS_F15S -DLIGHTMODBUS_F16S -DLIGHTMODBUS_F22S

ifneq ($(COVERAGE),)
COVERAGE_FLAGS += --coverage
endif

CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -I../include -I../include/lightmodbus $(DEFS)

IMPL = \
	-x c ../include/lightmodbus/base.impl.h \
	-x c ../include/lightmodbus/slave.impl.h \
	-x c ../include/lightmodbus/slave_func.impl.h \
	-x c ../include/lightmodbus/master.impl.h \
	-x c ../include/lightmodbus/master_func.impl.h

all: 
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -o test test.c $(IMPL)
	$(CC) $(CFLAGS) -o genrq genrq.c $(IMPL)

run-test:
	./test < test-requests.txt > test-result.txt
