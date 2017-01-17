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

#include <lightmodbus/core.h>
#include <lightmodbus/parser.h>
#include <lightmodbus/master/mtypes.h>
#include <lightmodbus/master/mregisters.h>

uint8_t modbusBuildRequest0304( ModbusMaster *status, uint8_t function, uint8_t address, uint16_t index, uint16_t count )
{
	//Build request03 frame, to send it so slave
	//Read multiple holding registers

	//Set frame length
	uint8_t frameLength = 8;

	//Check if given pointer is valid
	if ( status == NULL || ( function != 3 && function != 4 ) ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( count == 0 || count > 125 || address == 0 ) return MODBUS_ERROR_OTHER;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) );
	if ( status->request.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = function;
	builder->request0304.index = modbusSwapEndian( index );
	builder->request0304.count = modbusSwapEndian( count );

	//Calculate crc
	builder->request0304.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	status->predictedResponseLength = 4 + 1 + ( count << 1 );
	return MODBUS_ERROR_OK;
}

uint8_t modbusBuildRequest06( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t value )
{
	//Build request06 frame, to send it so slave
	//Write single holding reg

	//Set frame length
	uint8_t frameLength = 8;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->predictedResponseLength = 0;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) );
	if ( status->request.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = 6;
	builder->request06.index = modbusSwapEndian( index );
	builder->request06.value = modbusSwapEndian( value );

	//Calculate crc
	builder->request06.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 8;
	return MODBUS_ERROR_OK;
}

uint8_t modbusBuildRequest16( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t count, uint16_t *values )
{
	//Build request16 frame, to send it so slave
	//Write multiple holding registers

	//Set frame length
	uint8_t frameLength = 9 + ( count << 1 );
	uint8_t i = 0;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( values == NULL || count == 0 || count > 123 ) return MODBUS_ERROR_OTHER;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) );
	if ( status->request.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = 16;
	builder->request16.index = modbusSwapEndian( index );
	builder->request16.count = modbusSwapEndian( count );
	builder->request16.length = count << 1;

	for ( i = 0; i < count; i++ )
		builder->request16.values[i] = modbusSwapEndian( values[i] );

	builder->request16.values[count] = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 4 + 4;

	return MODBUS_ERROR_OK;
}

uint8_t modbusBuildRequest22( ModbusMaster *status, uint8_t address, uint16_t index, uint16_t andmask, uint16_t ormask )
{
	//Build request22 frame, to send it so slave
	//Mask write single holding reg

	//Set frame length
	uint8_t frameLength = 10;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->predictedResponseLength = 0;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) );
	if ( status->request.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = 22;
	builder->request22.index = modbusSwapEndian( index );
	builder->request22.andmask = modbusSwapEndian( andmask );
	builder->request22.ormask = modbusSwapEndian( ormask );

	//Calculate crc
	builder->request22.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 10;
	return MODBUS_ERROR_OK;
}

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

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address != 0;
	dataok &= parser->response0304.address == requestParser->request0304.address;
	dataok &= parser->response0304.function == requestParser->request0304.function;
	dataok &= parser->response0304.length != 0;
	dataok &= parser->response0304.length == modbusSwapEndian( requestParser->request0304.count ) << 1 ;
	dataok &= parser->response0304.length <= 250;

	//If data is bad, abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	//Allocate memory for ModbusData structures array
	status->data.coils = (uint8_t*) calloc( parser->response0304.length >> 1, sizeof( uint16_t ) );
	status->data.regs = (uint16_t*) status->data.coils;
	if ( status->data.coils == NULL ) return MODBUS_ERROR_ALLOC;
	status->data.address = parser->base.address;
	status->data.function = parser->base.function;
	status->data.type = parser->base.function == 3 ? MODBUS_HOLDING_REGISTER : MODBUS_INPUT_REGISTER;
	status->data.index = modbusSwapEndian( requestParser->request0304.index );
	status->data.count = modbusSwapEndian( requestParser->request0304.count );

	//Copy received data (with swapping endianness)
	for ( i = 0; i < ( parser->response0304.length >> 1 ); i++ )
		status->data.regs[i] = modbusSwapEndian( parser->response0304.values[i] );

	status->data.length = parser->response0304.length;
	return MODBUS_ERROR_OK;
}

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

	//Set up new data table
	status->data.coils = (uint8_t*) calloc( 1, sizeof( uint16_t ) );
	status->data.regs = (uint16_t*) status->data.coils;
	if ( status->data.coils == NULL ) return MODBUS_ERROR_ALLOC;
	status->data.function = 6;
	status->data.address = parser->base.address;
	status->data.type = MODBUS_HOLDING_REGISTER;
	status->data.index = modbusSwapEndian( parser->response06.index );
	status->data.count = 1;
	status->data.regs[0] = modbusSwapEndian( parser->response06.value );
	status->data.length = 2;
	return MODBUS_ERROR_OK;
}

uint8_t modbusParseResponse16( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 16 (write multiple holding reg)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL ) return MODBUS_ERROR_OTHER;

	//Check frame lengths
	if ( status->request.length < 7u || status->request.length != 9 + requestParser->request16.length ) return MODBUS_ERROR_FRAME;
	if ( status->response.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->response16.address == requestParser->request16.address;
	dataok &= parser->response16.function == requestParser->request16.function;
	dataok &= parser->response16.index == requestParser->request16.index;
	dataok &= parser->response16.count == requestParser->request16.count;
	dataok &= modbusSwapEndian( parser->response16.count ) <= 123;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	//Set up data length - response successfully parsed
	status->data.address = parser->base.address;
	status->data.function = 16;
	status->data.type = MODBUS_HOLDING_REGISTER;
	status->data.index = modbusSwapEndian( parser->response16.index );
	status->data.count = modbusSwapEndian( parser->response16.count );
	status->data.regs = NULL;
	status->data.length = 0;
	return MODBUS_ERROR_OK;
}

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
