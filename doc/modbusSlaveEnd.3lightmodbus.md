# modbusSlaveEnd 3lightmodbus "21 January 2016" "v1.3"

## NAME
**modbusSlaveEnd** - free memory used in **ModbusSlave** structure.

## SYNOPSIS
`#include <lightmodbus/slave.h>`

`uint8_t modbusSlaveEnd( ModbusSlave *status );`

## DESCRIPTION
The **modbusSlaveEnd** frees data allocated by **modbusSlaveInit**.

## SEE ALSO
ModbusSlave(3lightmodbus), modbusSlaveInit(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
