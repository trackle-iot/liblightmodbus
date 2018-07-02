/*
	liblightmodbus - a lightweight, multiplatform Modbus library
	Copyright (C) 2017 Jacek Wieczorek <mrjjot@gmail.com>

	This file is part of liblightmodbus.

	Liblightmodbus is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Liblightmodbus is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This is main header file that is ought to be included as library */

#ifndef LIGHTMODBUS_H
#define LIGHTMODBUS_H

//Include proper header files
#include <inttypes.h>
#include "libconf.h"

//Some protection
#if defined(LIGHTMODBUS_BIG_ENDIAN) && defined(LIGHTMODBUS_LITTLE_ENDIAN)
#error LIGHTMODBUS_BIG_ENDIAN and LIGHTMODBUS_LITTLE_ENDIAN cannot be used at once!
#endif

//Error codes
typedef enum modbusError
{
	MODBUS_ERROR_OK = 0, //Everything is ok
	MODBUS_ERROR_EXCEPTION = 1, //Slave has thrown an exception (on slave side: check ModbusSlave.lastException, lastParseError)
	MODBUS_ERROR_ALLOC, //Memory allocation problem (eg. system ran out of RAM)
	MODBUS_ERROR_OTHER, //Other reason function was exited (eg. bad function parameter)
	MODBUS_ERROR_NULLPTR, //Null pointer instead some crucial data
	MODBUS_ERROR_PARSE, //Parsing error occurred - check parseError
	MODBUS_ERROR_BUILD, //Building error occurred - check buildError
	MODBUS_OK = MODBUS_ERROR_OK,
} ModbusError;

//Frame processing (buidling/parsing) errors
//These errors are not critical and serve just as additional source of information for user
typedef enum modbusFrameError
{
	MODBUS_FERROR_OK = MODBUS_OK,
	MODBUS_FERROR_CRC, //Invalid CRC
	MODBUS_FERROR_LENGTH, //Invalid frame length
	MODBUS_FERROR_COUNT, //Invalid declared data item count
	MODBUS_FERROR_VALUE, //Illegal data value (e.g. single coil)
	MODBUS_FERROR_RANGE, //Invalid register range
	MODBUS_FERROR_NOSRC, //There's neither callback function nor value array provided for this data type
	MODBUS_FERROR_NOREAD, //No read access to one of regsiters
	MODBUS_FERROR_NOWRITE, //No write access to one of regsiters
	MODBUS_FERROR_NOFUN, //Function not supported
	MODBUS_FERROR_BADFUN, //Requested bad function
	MODBUS_FERROR_NULLFUN, //Function overriden by user with NULL
	MODBUS_FERROR_MISM_FUN, //Function request-response mismatch
	MODBUS_FERROR_MISM_ADDR, //Slave address request-response mismatch
	MODBUS_FERROR_MISM_INDEX, //Index value request-response mismatch
	MODBUS_FERROR_MISM_COUNT, //Count value request-response mismatch
	MODBUS_FERROR_MISM_VALUE, //Data value request-response mismatch
	MODBUS_FERROR_MISM_MASK, //Mask value request-response mismatch
	MODBUS_FERROR_BROADCAST //Received response for broadcast message?

} ModbusFrameError;

//Exception codes (defined by Modbus protocol)
typedef enum modbusExceptionCode
{
	MODBUS_EXCEP_ILLEGAL_FUNCTION = 1,
	MODBUS_EXCEP_ILLEGAL_ADDRESS = 2,
	MODBUS_EXCEP_ILLEGAL_VALUE = 3,
	MODBUS_EXCEP_SLAVE_FAILURE = 4,
	MODBUS_EXCEP_ACK = 5,
	MODBUS_EXCEP_NACK = 7
} ModbusExceptionCode;

//Modbus data types
typedef enum modbusDataType
{
	MODBUS_HOLDING_REGISTER = 1,
	MODBUS_INPUT_REGISTER = 2,
	MODBUS_COIL = 4,
	MODBUS_DISCRETE_INPUT = 8
} ModbusDataType;

#define BITSTOBYTES( n ) ( n != 0 ? ( 1 + ( ( n - 1 ) >> 3 ) ) : 0 )

//Always swaps endianness - modbusSwapEndian
static inline uint16_t modbusSwapEndian( uint16_t data ) { return ( data << 8 ) | ( data >> 8 ); }

//Matches endianness with Modbus - works conditionally - modbusMatchEndian
#ifdef LIGHTMODBUS_BIG_ENDIAN
static inline uint16_t modbusMatchEndian( uint16_t data ) { return data; }
#else
static inline uint16_t modbusMatchEndian( uint16_t data ) { return modbusSwapEndian( data ); }
#endif

//Function prototypes
extern uint8_t modbusMaskRead( const uint8_t *mask, uint16_t maskLength, uint16_t bit );
extern uint8_t modbusMaskWrite( uint8_t *mask, uint16_t maskLength, uint16_t bit, uint8_t value );
extern uint16_t modbusCRC( const uint8_t *data, uint16_t length );

//For user convenience
#include "master.h"
#include "slave.h"

#endif
