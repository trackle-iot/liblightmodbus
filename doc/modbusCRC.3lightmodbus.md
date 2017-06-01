# modbusCRC 3lightmodbus "01 June 2017" "v1.3"

## NAME
**modbusCRC** - calculate Modbus 16-bit CRC checksum of given data portion

## SYNOPSIS
`#include <lightmodbus/core.h>`

`uint16_t modbusCRC( const uint8_t *data, uint16_t length );`

## DESCRIPTION
The **modbusCRC** function calculates and returns cyclic redundancy checksum of *length* bytes starting from *data* pointer. When null *data* pointer is passed, no action is taken and 0 is returned.

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
