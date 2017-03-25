# modbusMasterEnd 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusMasterEnd** - free memory used in **ModbusMaster** structure.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`uint8_t modbusMasterEnd( ModbusMaster *status );`

## DESCRIPTION
The **modbusMasterEnd** routine frees the allocated data in *status* structure.
An error code is returned depending on success of **free** call.

## SEE ALSO
ModbusMaster(3lightmodbus), modbusMasterInit(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
