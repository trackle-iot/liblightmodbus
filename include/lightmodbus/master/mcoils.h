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

#ifndef LIGHTMODBUS_MASTER_COILS_H
#define LIGHTMODBUS_MASTER_COILS_H

#include <inttypes.h>
#include "mtypes.h"

//Functions for building requests
#define modbusBuildRequest01( status, addres, firstCoil, coilCount ) modbusBuildRequest0102( status, 01, addres, firstCoil, coilCount )
#define modbusBuildRequest02( status, addres, firstCoil, coilCount ) modbusBuildRequest0102( status, 02, addres, firstCoil, coilCount )
extern uint8_t modbusBuildRequest0102( ModbusMaster *status, uint8_t function, uint8_t address, uint16_t firstCoil, uint16_t coilCount );
extern uint8_t modbusBuildRequest05( ModbusMaster *status, uint8_t address, uint16_t coil, uint16_t value );
extern uint8_t modbusBuildRequest15( ModbusMaster *status, uint8_t address, uint16_t firstCoil, uint16_t coilCount, uint8_t *values );

//Functions for parsing responses
#define modbusParseResponse01 modbusParseResponse0102
#define modbusParseResponse02 modbusParseResponse0102
extern uint8_t modbusParseResponse0102( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );
extern uint8_t modbusParseResponse05( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );
extern uint8_t modbusParseResponse15( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );

#endif
