# modbusMasterEnd 3lightmodbus "29 July 2016" "v1.2"

## NAME
**modbusMasterEnd** - free memory used in **ModbusMaster** structure.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`void modbusMasterEnd( ModbusMaster *status );`

## DESCRIPTION
The **modbusMasterEnd** frees data dynamically allocated by **modbusMasterInit**.

## SEE ALSO
ModbusMaster(3lightmodbus), modbusMasterInit(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
