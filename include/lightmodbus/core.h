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

#ifndef LIGHTMODBUS_BASE_H
#define LIGHTMODBUS_BASE_H

/* This is main header file that is ought to be included as library */

//Include proper header files
#include <inttypes.h>
#include "libconf.h"

//Error codes
typedef enum modbusError
{
	MODBUS_ERROR_OK = 0, //Everything is ok
	MODBUS_ERROR_EXCEPTION = 1, //Slave has thrown an exception and it's stored in status->exception
	MODBUS_ERROR_BAD_FUNCTION = 2, //Slave/master did not parse frame, because the function is not supported (in case of slave, exception is also thrown)
	MODBUS_ERROR_CRC = 4, //Invalid CRC error
	MODBUS_ERROR_ALLOC = 8, //Memory allocation problem (eg. system ran out of RAM)
	MODBUS_ERROR_OTHER = 16, //Other reason function was exited (eg. bad function parameter)
	MODBUS_ERROR_FRAME = 32, //Frame contained incorrect data, and exception could not be thrown (eg. bytes count != reg count * 2 in slave's response)
	MODBUS_OK = MODBUS_ERROR_OK
} ModbusError;

//Exception codes
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

#endif
