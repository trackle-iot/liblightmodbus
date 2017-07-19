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

#include <stdlib.h>
#include <string.h>
#include <lightmodbus/core.h>
#include <lightmodbus/parser.h>
#include <lightmodbus/master.h>
#include <lightmodbus/master/mpcoils.h>

#if defined(LIGHTMODBUS_F01M) || defined(LIGHTMODBUS_F02M)
uint8_t modbusParseResponse0102( ModbusMaster *status, ModbusParser *parser, ModbusParser *requestParser )
{
	//Parse slave response to request 01 (read multiple coils)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL || ( parser->base.function != 1 && parser->base.function != 2 ) )
		return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	//Frame has to be at least 4 bytes long so byteCount can always be accessed in this case
	if ( status->response.length != 5 + parser->response0102.length || status->request.length != 8 ) return MODBUS_ERROR_FRAME;

	uint16_t count = modbusSwapEndian( requestParser->request0102.count );

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address != 0;
	dataok &= parser->base.address == requestParser->base.address;
	dataok &= parser->base.function == requestParser->base.function;
	dataok &= parser->response0102.length != 0;
	dataok &= parser->response0102.length <= 250;
	dataok &= parser->response0102.length == BITSTOBYTES( count );

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	#ifndef LIGHTMODBUS_STATIC_MEM_MASTER_DATA
		status->data.coils = (uint8_t*) calloc( BITSTOBYTES( count ), sizeof( uint8_t ) );
		status->data.regs = (uint16_t*) status->data.coils;
		if ( status->data.coils == NULL ) return MODBUS_ERROR_ALLOC;
	#else
		if ( BITSTOBYTES( count ) * sizeof( uint8_t ) > LIGHTMODBUS_STATIC_MEM_MASTER_DATA ) return MODBUS_ERROR_ALLOC;
		memset( status->data.coils, 0, BITSTOBYTES( count ) * sizeof( uint8_t ) );
	#endif

	status->data.function = parser->base.function;
	status->data.address = parser->base.address;
	status->data.type = parser->base.function == 1 ? MODBUS_COIL : MODBUS_DISCRETE_INPUT;
	status->data.index = modbusSwapEndian( requestParser->request0102.index );
	status->data.count = count;
	memcpy( status->data.coils, parser->response0102.values, parser->response0102.length );
	status->data.length = parser->response0102.length;
	return MODBUS_ERROR_OK;
}
#endif

#ifdef LIGHTMODBUS_F05M
uint8_t modbusParseResponse05( ModbusMaster *status, ModbusParser *parser, ModbusParser *requestParser )
{
	//Parse slave response to request 05 (write single coil)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL ) return MODBUS_ERROR_OTHER;

	//Check frame lengths
	if ( status->response.length != 8 || status->request.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address == requestParser->base.address;
	dataok &= parser->base.function == requestParser->base.function;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	#ifndef LIGHTMODBUS_STATIC_MEM_MASTER_DATA
		status->data.coils = (uint8_t*) calloc( 1, sizeof( uint8_t ) );
		status->data.regs = (uint16_t*) status->data.coils;
		if ( status->data.coils == NULL ) return MODBUS_ERROR_ALLOC;
	#else
		if ( 1 * sizeof( uint8_t ) > LIGHTMODBUS_STATIC_MEM_MASTER_DATA ) return MODBUS_ERROR_ALLOC;
		memset( status->data.coils, 0, 1 * sizeof( uint8_t ) );
	#endif

	status->data.function = 5;
	status->data.address = parser->base.address;
	status->data.type = MODBUS_COIL;
	status->data.index = modbusSwapEndian( requestParser->request05.index );
	status->data.count = 1;
	status->data.coils[0] = parser->response05.value != 0;
	status->data.length = 1;
	return MODBUS_ERROR_OK;
}
#endif

#ifdef LIGHTMODBUS_F15M
uint8_t modbusParseResponse15( ModbusMaster *status, ModbusParser *parser, ModbusParser *requestParser )
{
	//Parse slave response to request 15 (write multiple coils)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL ) return MODBUS_ERROR_OTHER;

	//Check frame lengths
	if ( status->request.length < 7u || status->request.length != 9 + requestParser->request15.length ) return MODBUS_ERROR_FRAME;
	if ( status->response.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address == requestParser->base.address;
	dataok &= parser->base.function == requestParser->base.function;
	dataok &= parser->response15.index == requestParser->request15.index;
	dataok &= parser->response15.count == requestParser->request15.count;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	status->data.address = parser->base.address;
	status->data.function = 15;
	status->data.type = MODBUS_COIL;
	status->data.index = modbusSwapEndian( parser->response15.index );
	status->data.count = modbusSwapEndian( parser->response15.count );
	status->data.length = 0;
	return MODBUS_ERROR_OK;
}
#endif
