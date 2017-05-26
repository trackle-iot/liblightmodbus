# modbusMasterEnd 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusMasterEnd** - finish usage of **ModbusMaster** structure.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`uint8_t modbusMasterEnd( ModbusMaster *status );`

## DESCRIPTION
The **modbusMasterEnd** routine frees the allocated data in *status* structure (assuming that no fixed buffer sizes were set during library compilation process).

## SEE ALSO
ModbusMaster(3lightmodbus), modbusMasterInit(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
