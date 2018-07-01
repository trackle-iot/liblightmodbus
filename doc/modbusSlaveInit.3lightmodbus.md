# modbusSlaveInit 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusSlaveInit** - setup **ModbusSlave** structure for use.

## SYNOPSIS
`#include <lightmodbus/slave.h>`

`uint8_t modbusSlaveInit( ModbusSlave *status );`

## DESCRIPTION
The **modbusSlaveInit** function sets crucial members of **ModbuSlave** structure, enabling its use by the rest of the library. This function should always be called before using any other Modbus slave side functions.
The function returns an error code on exit (see lightmodbus(3lightmodbus)).

Structure can later be cleaned up with **modbusSlaveEnd**.

## SEE ALSO
ModbusSlave(3lightmodbus), modbusSlaveEnd(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
