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
#include <lightmodbus/core.h>
#include <lightmodbus/parser.h>
#include <lightmodbus/master.h>
#include <lightmodbus/master/mpregs.h>

#if defined(LIGHTMODBUS_F03M) || defined(LIGHTMODBUS_F04M)
uint8_t modbusParseResponse0304( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 03
	//Read multiple holding registers

	uint8_t dataok = 1;
	uint8_t i = 0;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL || ( parser->base.function != 3 && parser->base.function != 4 ) )
		return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	//Frame has to be at least 4 bytes long so byteCount can always be accessed in this case
	if ( status->response.length != 5 + parser->response0304.length || status->request.length != 8 ) return MODBUS_ERROR_FRAME;

	uint16_t count = modbusSwapEndian( requestParser->request0304.count );

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address != 0;
	dataok &= parser->response0304.address == requestParser->request0304.address;
	dataok &= parser->response0304.function == requestParser->request0304.function;
	dataok &= parser->response0304.length != 0;
	dataok &= parser->response0304.length == count << 1 ;
	dataok &= parser->response0304.length <= 250;

	//If data is bad, abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	#ifndef LIGHTMODBUS_STATIC_MEM_MASTER_DATA
		//Allocate memory for ModbusData structures array
		status->data.coils = (uint8_t*) calloc( parser->response0304.length >> 1, sizeof( uint16_t ) );
		status->data.regs = (uint16_t*) status->data.coils;
		if ( status->data.coils == NULL ) return MODBUS_ERROR_ALLOC;
	#else
		if ( ( parser->response0304.length >> 1 ) * sizeof( uint16_t ) > LIGHTMODBUS_STATIC_MEM_MASTER_DATA ) return MODBUS_ERROR_ALLOC;
	#endif

	status->data.address = parser->base.address;
	status->data.function = parser->base.function;
	status->data.type = parser->base.function == 3 ? MODBUS_HOLDING_REGISTER : MODBUS_INPUT_REGISTER;
	status->data.index = modbusSwapEndian( requestParser->request0304.index );
	status->data.count = count;

	//Copy received data (with swapping endianness)
	for ( i = 0; i < count; i++ )
		status->data.regs[i] = modbusSwapEndian( parser->response0304.values[i] );

	status->data.length = parser->response0304.length;
	return MODBUS_ERROR_OK;
}
#endif

#ifdef LIGHTMODBUS_F06M
uint8_t modbusParseResponse06( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 06 (write single holding reg)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL ) return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	//Frame has to be at least 4 bytes long so byteCount can always be accessed in this case
	if ( status->response.length != 8 || status->request.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->response06.address == requestParser->request06.address;
	dataok &= parser->response06.function == requestParser->request06.function;
	dataok &= parser->response06.index == requestParser->request06.index;
	dataok &= parser->response06.value == requestParser->request06.value;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	#ifndef LIGHTMODBUS_STATIC_MEM_MASTER_DATA
		//Set up new data table
		status->data.coils = (uint8_t*) calloc( 1, sizeof( uint16_t ) );
		status->data.regs = (uint16_t*) status->data.coils;
		if ( status->data.coils == NULL ) return MODBUS_ERROR_ALLOC;
	#else
		if ( 1 * sizeof( uint16_t ) > LIGHTMODBUS_STATIC_MEM_MASTER_DATA ) return MODBUS_ERROR_ALLOC;
	#endif

	status->data.function = 6;
	status->data.address = parser->base.address;
	status->data.type = MODBUS_HOLDING_REGISTER;
	status->data.index = modbusSwapEndian( parser->response06.index );
	status->data.count = 1;
	status->data.regs[0] = modbusSwapEndian( parser->response06.value );
	status->data.length = 2;
	return MODBUS_ERROR_OK;
}
#endif

#ifdef LIGHTMODBUS_F16M
uint8_t modbusParseResponse16( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 16 (write multiple holding reg)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL ) return MODBUS_ERROR_OTHER;

	//Check frame lengths
	if ( status->request.length < 7u || status->request.length != 9 + requestParser->request16.length ) return MODBUS_ERROR_FRAME;
	if ( status->response.length != 8 ) return MODBUS_ERROR_FRAME;

	uint16_t count = modbusSwapEndian( parser->response16.count );

	//Check between data sent to slave and received from slave
	dataok &= parser->response16.address == requestParser->request16.address;
	dataok &= parser->response16.function == requestParser->request16.function;
	dataok &= parser->response16.index == requestParser->request16.index;
	dataok &= parser->response16.count == requestParser->request16.count;
	dataok &= count <= 123;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	//Set up data length - response successfully parsed
	status->data.address = parser->base.address;
	status->data.function = 16;
	status->data.type = MODBUS_HOLDING_REGISTER;
	status->data.index = modbusSwapEndian( parser->response16.index );
	status->data.count = count;
	#ifndef LIGHTMODBUS_STATIC_MEM_MASTER_DATA
		status->data.regs = NULL;
	#endif
	status->data.length = 0;
	return MODBUS_ERROR_OK;
}
#endif

#ifdef LIGHTMODBUS_F22M
uint8_t modbusParseResponse22( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 22 (mask write single holding reg)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL ) return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	//Frame has to be at least 4 bytes long so byteCount can always be accessed in this case
	if ( status->response.length != 10 || status->request.length != 10 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->response22.address == requestParser->request22.address;
	dataok &= parser->response22.function == requestParser->request22.function;
	dataok &= parser->response22.index == requestParser->request22.index;
	dataok &= parser->response22.andmask == requestParser->request22.andmask;
	dataok &= parser->response22.ormask == requestParser->request22.ormask;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	//Set up new data table
	status->data.address = parser->base.address;
	status->data.function = 22;
	status->data.type = MODBUS_HOLDING_REGISTER;
	status->data.index = modbusSwapEndian( parser->response22.index );
	status->data.count = 1;
	status->data.length = 0;
	return MODBUS_ERROR_OK;
}
#endif
