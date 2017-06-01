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

#ifndef LIGHTMODBUS_SCOILS_H
#define LIGHTMODBUS_SCOILS_H

#include <inttypes.h>
#include "../libconf.h"
#include "../slave.h"

//Functions needed from other modules
extern uint8_t modbusBuildException( ModbusSlave *status, uint8_t function, uint8_t exceptionCode );

//Functions for parsing requests
#if defined(LIGHTMODBUS_F01S) || defined(LIGHTMODBUS_F02S)
#define modbusParseRequest01 modbusParseRequest0102
#define modbusParseRequest02 modbusParseRequest0102
extern uint8_t modbusParseRequest0102( ModbusSlave *status, union ModbusParser *parser );
#endif

#ifdef LIGHTMODBUS_F05S
extern uint8_t modbusParseRequest05( ModbusSlave *status, union ModbusParser *parser );
#endif

#ifdef LIGHTMODBUS_F15S
extern uint8_t modbusParseRequest15( ModbusSlave *status, union ModbusParser *parser );
#endif

#endif
