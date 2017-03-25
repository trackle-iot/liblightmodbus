# modbusSlaveEnd 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusSlaveEnd** - free memory used in **ModbusSlave** structure.

## SYNOPSIS
`#include <lightmodbus/slave.h>`

`uint8_t modbusSlaveEnd( ModbusSlave *status );`

## DESCRIPTION
The **modbusSlaveEnd** routine frees the allocated data in *status* structure.
An error code is returned depending on success of **free** call.

## SEE ALSO
ModbusSlave(3lightmodbus), modbusSlaveInit(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
