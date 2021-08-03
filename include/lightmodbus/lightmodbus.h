/*
	liblightmodbus - a lightweight, header-only, cross-platform Modbus RTU/TCP library
	Copyright (C) 2021 Jacek Wieczorek <mrjjot@gmail.com>

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

// For C++
#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIGHTMODBUS_H
#define LIGHTMODBUS_H

/**
	\def LIGHTMODBUS_FULL
	\brief Configures the library to include all avaiable Modbus functions for both master and slave
*/
#ifdef LIGHTMODBUS_FULL
	#define LIGHTMODBUS_SLAVE_FULL
	#define LIGHTMODBUS_MASTER_FULL
#endif

/**
	\def LIGHTMODBUS_SLAVE_FULL
	\brief Include all functions available for slave
*/
#ifdef LIGHTMODBUS_SLAVE_FULL
	#define LIGHTMODBUS_SLAVE
	#define LIGHTMODBUS_F01S
	#define LIGHTMODBUS_F02S
	#define LIGHTMODBUS_F03S
	#define LIGHTMODBUS_F04S
	#define LIGHTMODBUS_F05S
	#define LIGHTMODBUS_F06S
	#define LIGHTMODBUS_F15S
	#define LIGHTMODBUS_F16S
	#define LIGHTMODBUS_F22S
#endif

/**
	\def LIGHTMODBUS_MASTER_FULL
	\brief Include all functions available for master
*/
#ifdef LIGHTMODBUS_MASTER_FULL
	#define LIGHTMODBUS_MASTER
	#define LIGHTMODBUS_F01M
	#define LIGHTMODBUS_F02M
	#define LIGHTMODBUS_F03M
	#define LIGHTMODBUS_F04M
	#define LIGHTMODBUS_F05M
	#define LIGHTMODBUS_F06M
	#define LIGHTMODBUS_F15M
	#define LIGHTMODBUS_F16M
	#define LIGHTMODBUS_F22M
#endif
#endif

/**
	\def LIGHTMODBUS_SLAVE
	\brief Configures the library to include slave functions.
*/
#ifdef LIGHTMODBUS_SLAVE
	#include "slave.h"
	#include "slave_func.h"
#endif

/**
	\def LIGHTMODBUS_MASTER
	\brief Configures the library to include master functions.
*/
#ifdef LIGHTMODBUS_MASTER
	#include "master.h"
	#include "master_func.h"
#endif

/**
	\def LIGHTMODBUS_DEBUG
	\brief Configures the library to include debug utilties.
*/
#ifdef LIGHTMODBUS_DEBUG
	#include "debug.h"
#endif

/**
	\def LIGHTMODBUS_IMPL
	\brief Includes implementation
	\warning This macro must only be used **exactly once** when including the library.
*/
#ifdef LIGHTMODBUS_IMPL
	#include "base.impl.h"
	#ifdef LIGHTMODBUS_SLAVE
		#include "slave.impl.h"
		#include "slave_func.impl.h"
	#endif

	#ifdef LIGHTMODBUS_MASTER
		#include "master.impl.h"
		#include "master_func.impl.h"
	#endif

	#ifdef LIGHTMODBUS_DEBUG
		#include "debug.impl.h"
	#endif
#endif

// For C++ (closes `extern "C"` )
#ifdef __cplusplus
}
#endif
