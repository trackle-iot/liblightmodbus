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

#ifndef LIGHTMODBUS_MASTER_H
#define LIGHTMODBUS_MASTER_H

#include <inttypes.h>
#include "parser.h"
#include "libconf.h"
#include "lightmodbus.h"

#ifdef LIGHTMODBUS_MASTER_BASE

#if defined( LIGHTMODBUS_MASTER_INVASIVE_PARSING ) && !defined( LIGHTMODBUS_NO_MASTER_DATA_BUFFER )
	#warning LIGHTMODBUS_MASTER_INVASIVE_PARSING has no effect if LIGHTMODBUS_NO_MASTER_DATA_BUFFER is not defined
#endif

#if defined( LIGHTMODBUS_NO_MASTER_DATA_BUFFER ) && !defined( LIGHTMODBUS_EXPERIMENTAL )
	#error Disabling exclusive master data buffer is an experimental feature that may cause problems. Please define LIGHTMODBUS_EXPERIMENTAL to dismiss this error message, but please make sure your system permits unaligned memory acces beforehand.
#endif

//Struct associating user defined parser function with function ID
#ifdef LIGHTMODBUS_MASTER_USER_FUNCTIONS
	struct modbusMaster;
	typedef struct modbusMasterUserFunction
	{
		uint8_t function; //Function code
		ModbusError ( *handler )( struct modbusMaster *status, ModbusParser *parser ); //Pointer to user defined function
	} ModbusMasterUserFunction;
#endif

typedef struct modbusMaster
{
	uint8_t predictedResponseLength; //If everything goes fine, slave will return this amout of data

	struct //Formatted request for slave
	{
		#ifdef LIGHTMODBUS_STATIC_MEM_MASTER_REQUEST
			uint8_t frame[LIGHTMODBUS_STATIC_MEM_MASTER_REQUEST];
		#else
			uint8_t *frame;
		#endif
		uint8_t length;
	} request;

	struct //Response from slave should be put here
	{
		#ifdef LIGHTMODBUS_STATIC_MEM_MASTER_RESPONSE
			uint8_t frame[LIGHTMODBUS_STATIC_MEM_MASTER_RESPONSE];
		#else
			const uint8_t *frame;
		#endif
		uint8_t length;
	} response;

	struct //Data read from slave
	{
		uint8_t address; //Addres of slave
		uint16_t index; //Address of the first element (in slave device)
		uint16_t count; //Count of data units (coils, registers, etc.)
		uint8_t length; //Length of data in bytes
		ModbusDataType type; //Type of data
		uint8_t function; //Function that accessed the data

		#ifdef LIGHTMODBUS_STATIC_MEM_MASTER_DATA
			union
			{
				uint8_t coils[LIGHTMODBUS_STATIC_MEM_MASTER_DATA];
				uint16_t regs[LIGHTMODBUS_STATIC_MEM_MASTER_DATA >> 1];
			};
		#else
			//Two separate pointers are used in case pointer size differed between types (possible on some weird architectures)
			uint8_t *coils; //Received data
			uint16_t *regs; //And the same received data, but converted to uint16_t pointer for convenience
		#endif
	} data;

	struct //Exceptions read are stored in this structure
	{
		uint8_t address; //Device address
		uint8_t function; //In which function exception occured
		uint8_t code; //Exception code
	} exception;

	//User defined functions
	#ifdef LIGHTMODBUS_MASTER_USER_FUNCTIONS
		ModbusMasterUserFunction *userFunctions;
		uint16_t userFunctionCount;
	#endif

} ModbusMaster; //Type containing master device configuration data
#endif

#include "master/mbregs.h"
#include "master/mbcoils.h"

#ifdef LIGHTMODBUS_MASTER_BASE
extern ModbusError modbusParseResponse( ModbusMaster *status );
extern ModbusError modbusMasterInit( ModbusMaster *status );
extern ModbusError modbusMasterEnd( ModbusMaster *status ); //Free memory used by master
#endif

#endif
