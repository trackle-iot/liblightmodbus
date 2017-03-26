# modbusSwapEndian 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusSwapEndian** - change given 16-bit integer's endianness.

## SYNOPSIS
`#include <lightmodbus/core.h>`

`uint16_t modbusSwapEndian( uint16_t data );`

## DESCRIPTION
The **modbusSwapEndian** function returns the same 16-bit portion of data, it is given, but with bytes order swapped. Function is included, because most PCs use little-endian architecture, while Modbus protocol tends to use big-endian data format.   

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
