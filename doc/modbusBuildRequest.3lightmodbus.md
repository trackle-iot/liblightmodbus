# modbusBuildRequest 3lightmodbus "21 January 2016" "v1.3"

## NAME
**modbusBuildRequest0102**, **modbusBuildRequest01**, **modbusBuildRequest02**, **modbusBuildRequest0304**, **modbusBuildRequest03**, **modbusBuildRequest04**, **modbusBuildRequest05**, **modbusBuildRequest06**, **modbusBuildRequest15**, **modbusBuildRequest16**, **modbusBuildRequest22** - build request for slave device.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`  
	#define modbusBuildRequest01( status, address, index, count ) modbusBuildRequest0102( (status), 1, (address), (index), (count) )
	#define modbusBuildRequest02( status, address, index, count ) modbusBuildRequest0102( (status), 2, (address), (index), (count) )
	#define modbusBuildRequest03( status, address, index, count ) modbusBuildRequest0304( (status), 3, (address), (index), (count) )
	#define modbusBuildRequest04( status, address, index, count ) modbusBuildRequest0304( (status), 4, (address), (index), (count) )
	uint8_t modbusBuildRequest0102( ModbusMaster *status, uint8_t function, uint8_t address, uint16_t index, uint16_t count );
	uint8_t modbusBuildRequest0304( ModbusMaster *status, uint8_t function, uint8_t address, uint16_t index, uint16_t count );
	uint8_t modbusBuildRequest05( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t value );
	uint8_t modbusBuildRequest06( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t value );
	uint8_t modbusBuildRequest15( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t count, uint8_t *values );
	uint8_t modbusBuildRequest16( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t count, uint16_t *values );
`

## DESCRIPTION
The **modbusBuildRequest** functions build request frames ought to be sent to slave device.
Each routine returns an error code described in lightmodbus(3lightmodbus).
On successful exit, the Modbus frame of *status.request.length* bytes length is written to *status.request.frame*. Otherwise the length is set to 0 and null pointer is returned in the *status.request* structure.

**modbusBuildRequest01**, **modbusBuildRequest02**, **modbusBuildRequest03** and **modbusBuildRequest04** are actually macros, later replaced with **modbusBuildRequest0102** and **modbusBuildRequest0304** - see above declarations.

## SEE ALSO
lightmodbus(3lightmodbus), ModbusMaster(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
