# modbusSlaveEnd 3lightmodbus "29 July 2016" "v1.2"

## NAME
**modbusSlaveEnd** - free memory used in **ModbusSlave** structure.

## SYNOPSIS
`#include <lightmodbus/slave.h>`

`void modbusSlaveEnd( ModbusSlave *status );`

## DESCRIPTION
The **modbusSlaveEnd** frees data dynamically allocated by **modbusSlaveInit**.

## SEE ALSO
ModbusSlave(3lightmodbus), modbusSlaveInit(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
