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

#include <lightmodbus/core.h>
#include <lightmodbus/parser.h>
#include <lightmodbus/addons/examine.h>
#include <string.h>
#include <stddef.h>

//Examines Modbus frame and returns information in ModbusFrameInfo
//This function doesn't perform ANY data checking apart from CRC check
//Please keep in mind, that if return value is different from MODBUS_ERROR_OK
//data returned in the structure is worthless
uint8_t modbusExamine( ModbusFrameInfo *info, uint8_t dir, const uint8_t *frame, uint8_t length )
{
	union modbusParser *parser;

	//Null pointer check
	if ( info == NULL || frame == NULL || length == 0 ) return MODBUS_ERROR_OTHER;

	//Initial struct clenup
	memset( info, 0, sizeof( struct modbusFrameInfo ) );
	info->data = NULL; //This is for weird patforms that don't consider 0 to be NULL

	parser = (union modbusParser*) frame;

	//CRC check (should satisfy both little and big endian platforms - see modbusCRC)
	info->crc = modbusCRC( frame, length - 2 );
	if ( info->crc != ( frame[length - 2] | ( frame[length - 1] << 8 ) ) ) return MODBUS_ERROR_CRC;

	//Copy basic information
	info->address = parser->base.address;
	info->function = parser->base.function;

	//Determine data type - filter out exception bit
	switch ( info->function & 127 )
	{
		//Discrete inputs
		case 2:
			info->type = MODBUS_DISCRETE_INPUT;
			break;

		//Input registers
		case 4:
			info->type = MODBUS_INPUT_REGISTER;
			break;

		//Coils
		case 01:
		case 05:
		case 15:
			info->type = MODBUS_COIL;
			break;

		//Holding registers
		case 03:
		case 06:
		case 16:
		case 22:
			info->type = MODBUS_HOLDING_REGISTER;
			break;

		//Unknown data type
		default:
			return MODBUS_ERROR_OTHER;
			break;
	}

	//Interpret exception - we don't even care about data direction
	if ( length == 5 )
	{
		info->exception = parser->exception.code;
		return MODBUS_ERROR_OK;
	}

	//TODO parse requests and responses
	if ( dir == MODBUS_EXAMINE_REQUEST )
	{
		switch ( info->function )
		{
			//Reading multiple coils/discrete inputs
			case 01:
			case 02:
				info->index = modbusSwapEndian( parser->request0102.index );
				info->count = modbusSwapEndian( parser->request0102.count );
				info->access = MODBUS_EXAMINE_READ;
				break;

			//Reading multiple holding/input registers
			case 03:
			case 04:
				info->index = modbusSwapEndian( parser->request0304.index );
				info->count = modbusSwapEndian( parser->request0304.count );
				info->access = MODBUS_EXAMINE_READ;
				break;

			//Write single coil
			case 05:
				info->index = modbusSwapEndian( parser->request05.index );
				info->data = &parser->request06.value;
				info->length = 2;
				info->count = 1;
				info->access = MODBUS_EXAMINE_WRITE;
				break;

			//Write single holding register
			case 06:
				info->index = modbusSwapEndian( parser->request06.index );
				info->data = &parser->request06.value;
				info->length = 2;
				info->count = 1;
				info->access = MODBUS_EXAMINE_WRITE;
				break;

			//Write multiple coils
			case 15:
				info->index = modbusSwapEndian( parser->request15.index );
				info->count = modbusSwapEndian( parser->request15.count );
				info->data = parser->request15.values;
				info->length = parser->request15.length;
				info->access = MODBUS_EXAMINE_WRITE;
				break;


		}
	}
	else if ( MODBUS_EXAMINE_RESPONSE )
	{

	}
	else return MODBUS_ERROR_OTHER;

	return MODBUS_ERROR_OK;
}
