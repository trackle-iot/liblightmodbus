# modbusMasterInit 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusMasterInit** - setup **ModbusMaster** structure for use.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`uint8_t modbusMasterInit( ModbusMaster *status );`

## DESCRIPTION
The **modbusMasterInit** function sets crucial members of **ModbusMaster** structure. This function should always be called before using any other Modbus master side functions.
The function returns an error code on exit (see lightmodbus(3lightmodbus)).

Assuming that no fixed sizes buffers were set up during compilation, memory can be later freed with **modbusMasterEnd**.

## SEE ALSO
ModbusMaster(3lightmodbus), modbusMasterEnd(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
