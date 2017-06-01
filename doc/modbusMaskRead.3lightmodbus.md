# modbusMaskRead 3lightmodbus "01 June 2017" "v1.3"

## NAME
**modbusMaskRead** - read single bit from an array.

## SYNOPSIS
`#include <lightmodbus/core.h>`

`uint8_t modbusMaskRead( const uint8_t *mask, uint16_t maskLength, uint16_t bit );`

## DESCRIPTION
The **modbusMaskRead** function returns a single bit (from *bit* position) from little-endian array (*mask*) of *maskLength* bytes length. It's useful once whole array is treated like a single bit mask. When the returned value is greater than 1, an error has occurred.

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
