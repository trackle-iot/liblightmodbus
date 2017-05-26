# modbusParseRequest 3lightmodbus "25 March 2016" "v1.3"

## NAME
**modbusParseRequest** - parse request frame sent in by master device.

## SYNOPSIS
`#include <lightmodbus/slave.h>`

`uint8_t modbusParseRequest( ModbusSlave *status );`

## DESCRIPTION
The **modbusParseRequest** function parses request frame located in *status.request.frame* of *status.request.length* bytes length. The response (of *status.response.length* bytes) is automatically written to *status.response* structure. Length of 0 bytes means that response isn't necessary (broadcast request) or that something has gone wrong.
An error code is returned on exit (described in lightmodbus(3lightmodbus)).

## SEE ALSO
lightmodbus(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
