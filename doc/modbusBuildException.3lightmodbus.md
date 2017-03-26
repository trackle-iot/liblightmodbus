# modbusBuildException 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusBuildException** - build Modbus exception frame.

## SYNOPSIS
`#include <lightmodbus/slave.h>`

`uint8_t modbusBuildException( ModbusSlave *status, uint8_t function, uint8_t code )`

## DESCRIPTION
The **modbusBuildException** function builds Modbus exception frame, later located in *status.response*.
The *function* value is the number of function that throws an exception. The *code* value is the exception code to be thrown.
Unlike other routines included in the library, normally **modbusBuildException** function returns `MODBUS_ERROR_EXCEPTION` on successful exit.

## SEE ALSO
modbusParseException(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
