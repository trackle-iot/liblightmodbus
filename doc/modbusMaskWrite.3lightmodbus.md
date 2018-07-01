# modbusMaskWrite 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusMaskWrite** - write single bit in an array.

## SYNOPSIS
`#include <lightmodbus/core.h>`

`uint8_t modbusMaskWrite( uint8_t *mask, uint16_t maskLength, uint16_t bit, uint8_t value );`

## DESCRIPTION
The **modbusMaskWrite** writes a single bit (on *bit* position) in little-endian array (*mask*) of *maskLength* bytes length to *value* value. All non-zero bit values are assumed to be ones. The routine is very useful once whole array is treated like a single bit mask.  When the returned value is greater than 1, an error has occurred.

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
