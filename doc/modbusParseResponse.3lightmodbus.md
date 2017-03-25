# modbusParseResponse 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusParseResponse** - parse response frame returned by slave device.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`uint8_t modbusParseResponse( ModbusMaster *status );`

## DESCRIPTION
The **modbusParseResponse** function parses request frame located in *status.response.frame* of *status.response.length* bytes length. Results are written into *status.data*, when function exits. The function returns an error code on exit (see lightmodbus(3lightmodbus)).

It is important to mention, that parsing responses requires having unchanged request frames loaded into *status.request*.

## SEE ALSO
lightmodbus(3lightmodbus), ModbusMaster(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
