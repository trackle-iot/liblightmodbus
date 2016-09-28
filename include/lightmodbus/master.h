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

#ifndef LIGHTMODBUS_MASTER_BASE_H
#define LIGHTMODBUS_MASTER_BASE_H

#include <inttypes.h>

#include "core.h"
#include "parser.h"
#include "master/mtypes.h"
#include "master/mregisters.h"
#include "master/mcoils.h"
#include "master/mdiscreteinputs.h"
#include "master/minputregisters.h"

//Enabling modules in compilation process (use makefile to automate this process)
#ifndef LIGHTMODBUS_MASTER_REGISTERS
#define LIGHTMODBUS_MASTER_REGISTERS 0
#endif
#ifndef LIGHTMODBUS_MASTER_COILS
#define LIGHTMODBUS_MASTER_COILS 0
#endif
#ifndef LIGHTMODBUS_MASTER_DISCRETE_INPUTS
#define LIGHTMODBUS_MASTER_DISCRETE_INPUTS 0
#endif
#ifndef LIGHTMODBUS_MASTER_INPUT_REGISTERS
#define LIGHTMODBUS_MASTER_INPUT_REGISTERS 0
#endif

extern uint8_t modbusParseResponse( ModbusMaster *status );
extern uint8_t modbusMasterInit( ModbusMaster *status );
extern void modbusMasterEnd( ModbusMaster *status ); //Free memory used by master

#endif
