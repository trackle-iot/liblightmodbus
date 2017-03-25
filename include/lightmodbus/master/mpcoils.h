/*
	liblightmodbus - a lightweight, multiplatform Modbus library
	Copyright (C) 2016 Jacek Wieczorek <mrjjot@gmail.com>

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

#ifndef LIGHTMODBUS_MPCOILS_H
#define LIGHTMODBUS_MPCOILS_H

#include <inttypes.h>
#include "../master.h"

//Functions for parsing responses
#define modbusParseResponse01 modbusParseResponse0102
#define modbusParseResponse02 modbusParseResponse0102
extern uint8_t modbusParseResponse0102( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );
extern uint8_t modbusParseResponse05( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );
extern uint8_t modbusParseResponse15( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );

#endif
