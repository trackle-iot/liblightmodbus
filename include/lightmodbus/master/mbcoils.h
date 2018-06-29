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

#ifndef LIGHTMODBUS_MBCOILS_H
#define LIGHTMODBUS_MBCOILS_H

#include <inttypes.h>
#include "../libconf.h"
#include "../core.h"
#include "../master.h"

//Functions for building requests
#if defined(LIGHTMODBUS_F01M) || defined(LIGHTMODBUS_F02M)
extern ModbusError modbusBuildRequest0102( ModbusMaster *status, uint8_t function, uint8_t address, uint16_t index, uint16_t count );
static inline ModbusError modbusBuildRequest01( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t count ) 
	{ return modbusBuildRequest0102( (status), 1, (address), (index), (count) ); }
static inline ModbusError modbusBuildRequest02( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t count ) 
	{ return modbusBuildRequest0102( (status), 2, (address), (index), (count) ); }
#endif

#ifdef LIGHTMODBUS_F05M
extern ModbusError modbusBuildRequest05( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t value );
#endif

#ifdef LIGHTMODBUS_F05M
extern ModbusError modbusBuildRequest15( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t count, uint8_t *values );
#endif

#endif
