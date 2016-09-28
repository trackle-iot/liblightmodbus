/*
    liblightmodbus - a lightweight, multiplatform Modbus library
    Copyright (C) 2016  Jacek Wieczorek <mrjjot@gmail.com>

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

#ifndef LIGHTMODBUS_MASTER_INPUT_REGISTERS_H
#define LIGHTMODBUS_MASTER_INPUT_REGISTERS_H

#include <inttypes.h>
#include "mtypes.h"

//Functions for building requests
extern uint8_t modbusBuildRequest04( ModbusMaster *status, uint8_t address, uint16_t firstRegister, uint16_t registerCount );

//Functions for parsing responses
extern uint8_t modbusParseResponse04( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser );

#endif
