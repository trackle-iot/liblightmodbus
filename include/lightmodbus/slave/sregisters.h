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

#ifndef LIGHTMODBUS_SLAVE_REGISTERS_H
#define LIGHTMODBUS_SLAVE_REGISTERS_H

#include <inttypes.h>
#include "stypes.h"

//Functions needed from other modules
extern uint8_t modbusBuildException( ModbusSlave *status, uint8_t function, uint8_t exceptionCode );

//Functions for parsing requests
extern uint8_t modbusParseRequest03( ModbusSlave *status, union ModbusParser *parser );
extern uint8_t modbusParseRequest06( ModbusSlave *status, union ModbusParser *parser );
extern uint8_t modbusParseRequest16( ModbusSlave *status, union ModbusParser *parser );

#endif
