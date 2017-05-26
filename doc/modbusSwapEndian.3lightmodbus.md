# modbusSwapEndian 3lightmodbus "27 May 2017" "v1.3"

## NAME
**modbusSwapEndian** - change given 16-bit integer's endianness.

## SYNOPSIS
`#include <lightmodbus/core.h>`

`uint16_t modbusSwapEndian( uint16_t data );`

## DESCRIPTION
The **modbusSwapEndian** macro returns the same 16-bit portion of data it is given, but with bytes order swapped. It is included in the library, because most PCs use little-endian architecture, while Modbus protocol tends to use big-endian data format. It is important to mention, that when library is built on a big-endian platform (**LIGHTMODBUS_BIG_ENDIAN** is defined), the macro just returns the value it is given.  

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
