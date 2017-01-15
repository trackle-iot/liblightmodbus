/*
	liblightmodbus - a lightweight, multiplatform Modbus library
	Copyright (C) 2016	Jacek Wieczorek <mrjjot@gmail.com>

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

#ifndef LIGHTMODBUS_MASTER_TYPES_H
#define LIGHTMODBUS_MASTER_TYPES_H

#include <inttypes.h>
#include "../core.h"

#define MODBUS_HOLDING_REGISTER 0
#define MODBUS_INPUT_REGISTER 1
#define MODBUS_COIL 2
#define MODBUS_DISCRETE_INPUT 4

typedef struct
{
	uint8_t address; //Device address
	uint8_t dataType; //Data type
	uint16_t reg; //Register, coil, input ID
	uint16_t value; //Value of data
} ModbusData;

typedef struct
{
	ModbusData *data; //Data read from slave
	uint8_t dataLength; //Count of data type instances read from slave
	uint8_t finished; //Is parsing finished?
	uint8_t predictedResponseLength; //If everything goes fine, slave will return this amout of data
	struct //Exceptions read are stored in this structure
	{
		uint8_t address; //Device address
		uint8_t function; //In which function exception occured
		uint8_t code; //Exception code
	} exception;
	ModbusFrame request; //Formatted request for slave
	ModbusFrame response; //Response from slave should be put here
} ModbusMaster; //Type containing master device configuration data

#endif
