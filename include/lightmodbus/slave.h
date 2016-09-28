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

#ifndef LIGHTMODBUS_SLAVE_BASE_H
#define LIGHTMODBUS_SLAVE_BASE_H

#include <inttypes.h>

#include "core.h"
#include "parser.h"
#include "slave/stypes.h"
#include "slave/sregisters.h"
#include "slave/scoils.h"
#include "slave/sdiscreteinputs.h"
#include "slave/sinputregisters.h"

//Enabling modules in compilation process (use makefile to automate this process)
#ifndef LIGHTMODBUS_SLAVE_REGISTERS
#define LIGHTMODBUS_SLAVE_REGISTERS 0
#endif
#ifndef LIGHTMODBUS_SLAVE_COILS
#define LIGHTMODBUS_SLAVE_COILS 0
#endif
#ifndef LIGHTMODBUS_SLAVE_DISCRETE_INPUTS
#define LIGHTMODBUS_SLAVE_DISCRETE_INPUTS 0
#endif
#ifndef LIGHTMODBUS_SLAVE_INPUT_REGISTERS
#define LIGHTMODBUS_SLAVE_INPUT_REGISTERS 0
#endif

//Function prototypes
extern uint8_t modbusBuildException( ModbusSlave *status, uint8_t function, uint8_t exceptionCode ); //Build an exception
extern uint8_t modbusParseRequest( ModbusSlave *status ); //Parse and interpret given modbus frame on slave-side
extern uint8_t modbusSlaveInit( ModbusSlave *status ); //Very basic init of slave side
extern void modbusSlaveEnd( ModbusSlave *status ); //Free memory used by slave

#endif
