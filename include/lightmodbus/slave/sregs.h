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

#ifndef LIGHTMODBUS_SREGS_H
#define LIGHTMODBUS_SREGS_H

#include <inttypes.h>
#include "../libconf.h"
#include "../slave.h"

//Functions needed from other modules
extern uint8_t modbusBuildException( ModbusSlave *status, uint8_t function, uint8_t exceptionCode );

//Functions for parsing requests
#if defined(LIGHTMODBUS_F03S) || defined(LIGHTMODBUS_F04S)
#define modbusParseRequest03 modbusParseRequest0304
#define modbusParseRequest04 modbusParseRequest0304
extern uint8_t modbusParseRequest0304( ModbusSlave *status, ModbusParser *parser );
#endif

#ifdef LIGHTMODBUS_F06S
extern uint8_t modbusParseRequest06( ModbusSlave *status, ModbusParser *parser );
#endif

#ifdef LIGHTMODBUS_F16S
extern uint8_t modbusParseRequest16( ModbusSlave *status, ModbusParser *parser );
#endif

#ifdef LIGHTMODBUS_F22S
extern uint8_t modbusParseRequest22( ModbusSlave *status, ModbusParser *parser );
#endif

#endif
