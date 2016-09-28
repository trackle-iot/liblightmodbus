# modbusParseRequest 3lightmodbus "4 August 2016" "v1.2"

## NAME
**modbusParseRequest**, **modbusParseRequest01**, **modbusParseRequest02**, **modbusParseRequest03**, **modbusParseRequest04**, **modbusParseRequest05**, **modbusParseRequest06**, **modbusParseRequest15**, **modbusParseRequest16** - parse request frame sent in by master device.

## SYNOPSIS
`#include <lightmodbus/slave.h>`

`  
	uint8_t modbusParseRequest( ModbusSlave *status );
	uint8_t modbusParseRequest01( ModbusSlave *status, union ModbusParser *parser );
	uint8_t modbusParseRequest02( ModbusSlave *status, union ModbusParser *parser );
	uint8_t modbusParseRequest03( ModbusSlave *status, union ModbusParser *parser );
	uint8_t modbusParseRequest04( ModbusSlave *status, union ModbusParser *parser );
	uint8_t modbusParseRequest05( ModbusSlave *status, union ModbusParser *parser );
	uint8_t modbusParseRequest06( ModbusSlave *status, union ModbusParser *parser );
	uint8_t modbusParseRequest15( ModbusSlave *status, union ModbusParser *parser );
	uint8_t modbusParseRequest16( ModbusSlave *status, union ModbusParser *parser );
`

## DESCRIPTION
The **modbusParseRequest** function parses request frame located in *status.request*. Response is automatically written to *status.response*, unless request
was broadcast.
When finished, an error code is returned (described in lightmodbus(3lightmodbus)) and *status.finished* is set to 1.

**modbusParseRequest01**, **modbusParseRequest02**, and so on can only parse specific requests, while **modbusParseRequest** automatically picks one of them. Keep in mind, that calling them directly is unsafe.

## SEE ALSO
lightmodbus(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
