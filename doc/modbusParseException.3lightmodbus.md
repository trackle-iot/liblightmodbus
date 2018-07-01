# modbusParseException 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusParseException** - parse exception frame returned by slave device.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`uint8_t modbusParseException( ModbusMaster *status, union ModbusParser *parser );`

## DESCRIPTION
The **modbusParseException** function parses exception frame stored in *parser* union, and leaves results in *status.exception*. The function returns an error code on exit, which unusually is `MODBUS_ERROR_EXCEPTION` on success (see lightmodbus(3lightmodbus)).

## SEE ALSO
ModbusMaster(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
