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

#ifndef LIGHTMODBUS_EXAMINE
#define LIGHTMODBUS_EXAMINE

#include <inttypes.h>
#include "../core.h"
#include "../libconf.h"

#define MODBUS_EXAMINE_REQUEST  0
#define MODBUS_EXAMINE_RESPONSE 1

typedef struct modbusFrameInfo
{
	uint8_t address; //Slave address
	uint8_t function; //Function
	uint8_t exception; //Exception number
	uint8_t type; //Data type (coil/register and so on)
	uint8_t index; //Register index
	uint8_t count; //Data unit count
	uint8_t access; //Access type - read/write
	uint16_t crc; //CRC

	//Binary data - pointer and length in bytes
	uint8_t *data;
	uint8_t length;
} ModbusFrameInfo;

extern uint8_t modbusExamine( ModbusFrameInfo *info, uint8_t dir, uint8_t *frame, uint8_t length );

#endif
