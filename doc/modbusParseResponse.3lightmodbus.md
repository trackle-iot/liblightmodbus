# modbusParseResponse 3lightmodbus "25 March 2017" "v1.3"

## NAME
**modbusParseResponse** - parse response frame returned by slave device.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`uint8_t modbusParseResponse( ModbusMaster *status );`

## DESCRIPTION
The **modbusParseResponse** function parses request frame located in *status.response.frame* of *status.response.length* bytes length. Results are written into *status.data*, when the function exits. It also returns an error code (see lightmodbus(3lightmodbus)).

In case the received frame is an exception, no data will be loaded into *status.data*, but *status.exception* will be filled information about the exception. In such case `MODBUS_ERROR_EXCEPTION` is returned.

It is important to mention, that parsing responses often requires having unchanged request frames loaded into *status.request*.

## SEE ALSO
lightmodbus(3lightmodbus), ModbusMaster(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
