# modbusParseRequest 3lightmodbus "25 March 2016" "v1.3"

## NAME
**modbusParseRequest** - parse request frame sent in by master device.

## SYNOPSIS
`#include <lightmodbus/slave.h>`

`uint8_t modbusParseRequest( ModbusSlave *status );`

## DESCRIPTION
The **modbusParseRequest** function parses request frame located in *status.request.frame* of *status.request.length* bytes length. Unless the request is a broadcast, the response is automatically written to *status.response* structure.
An error code is returned on exit (described in lightmodbus(3lightmodbus)).

## SEE ALSO
lightmodbus(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
