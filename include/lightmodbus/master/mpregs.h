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

#ifndef LIGHTMODBUS_MPREGS_H
#define LIGHTMODBUS_MPREGS_H

#include <inttypes.h>
#include "../libconf.h"
#include "../master.h"

//Functions for parsing responses
#if defined(LIGHTMODBUS_F03M) || defined(LIGHTMODBUS_F04M)
#define modbusParseResponse03 modbusParseResponse0304
#define modbusParseResponse04 modbusParseResponse0304
extern uint8_t modbusParseResponse0304( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );
#endif

#ifdef LIGHTMODBUS_F06M
extern uint8_t modbusParseResponse06( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );
#endif

#ifdef LIGHTMODBUS_F16M
extern uint8_t modbusParseResponse16( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );
#endif

#ifdef LIGHTMODBUS_F22M
extern uint8_t modbusParseResponse22( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );
#endif

#endif
