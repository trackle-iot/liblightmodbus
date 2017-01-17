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

uint8_t modbusBuildRequest0304( ModbusMaster *status, uint8_t function, uint8_t address, uint16_t firstRegister, uint16_t registerCount )
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
	if ( registerCount == 0 || registerCount > 125 || address == 0 ) return MODBUS_ERROR_OTHER;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) );
	if ( status->request.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = function;
	builder->request0304.firstRegister = modbusSwapEndian( firstRegister );
	builder->request0304.registerCount = modbusSwapEndian( registerCount );

	//Calculate crc
	builder->request0304.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	status->predictedResponseLength = 4 + 1 + ( registerCount << 1 );
	return MODBUS_ERROR_OK;
}

uint8_t modbusBuildRequest06( ModbusMaster *status, uint8_t address, uint16_t reg, uint16_t value )
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
	builder->request06.reg = modbusSwapEndian( reg );
	builder->request06.value = modbusSwapEndian( value );

	//Calculate crc
	builder->request06.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 8;
	return MODBUS_ERROR_OK;
}

uint8_t modbusBuildRequest16( ModbusMaster *status, uint8_t address, uint16_t firstRegister, uint16_t registerCount, uint16_t *values )
{
	//Build request16 frame, to send it so slave
	//Write multiple holding registers

	//Set frame length
	uint8_t frameLength = 9 + ( registerCount << 1 );
	uint8_t i = 0;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( values == NULL || registerCount == 0 || registerCount > 123 ) return MODBUS_ERROR_OTHER;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) );
	if ( status->request.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = 16;
	builder->request16.firstRegister = modbusSwapEndian( firstRegister );
	builder->request16.registerCount = modbusSwapEndian( registerCount );
	builder->request16.byteCount = registerCount << 1;

	for ( i = 0; i < registerCount; i++ )
		builder->request16.values[i] = modbusSwapEndian( values[i] );

	builder->request16.values[registerCount] = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 4 + 4;

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
	if ( status->response.length != 5 + parser->response0304.byteCount || status->request.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address != 0;
	dataok &= parser->response0304.address == requestParser->request0304.address;
	dataok &= parser->response0304.function == requestParser->request0304.function;
	dataok &= parser->response0304.byteCount != 0;
	dataok &= parser->response0304.byteCount == modbusSwapEndian( requestParser->request0304.registerCount ) << 1 ;
	dataok &= parser->response0304.byteCount <= 250;

	//If data is bad, abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	//Allocate memory for ModbusData structures array
	status->data.regs = (uint16_t*)( status->data.data = calloc( parser->response0304.byteCount >> 1, sizeof( uint16_t ) ) );
	if ( status->data.data == NULL ) return MODBUS_ERROR_ALLOC;
	status->data.address = parser->base.address;
	status->data.type = parser->base.function == 3 ? MODBUS_HOLDING_REGISTER : MODBUS_INPUT_REGISTER;
	status->data.first = modbusSwapEndian( requestParser->request0304.firstRegister );
	status->data.count = modbusSwapEndian( requestParser->request0304.registerCount );

	//Copy received data
	for ( i = 0; i < ( parser->response0304.byteCount >> 1 ); i++ )
		status->data.regs[i] = modbusSwapEndian( parser->response0304.values[i] );

	status->data.length = parser->response0304.byteCount;
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
	dataok &= parser->response06.reg == requestParser->request06.reg;
	dataok &= parser->response06.value == requestParser->request06.value;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	//Swap endianness
	modbusSwapEndian( parser->response06.reg );
	modbusSwapEndian( parser->response06.value );

	//Set up new data table
	status->data.regs = (uint16_t*)( status->data.data =  calloc( 1, sizeof( uint16_t ) ) );
	if ( status->data.data == NULL ) return MODBUS_ERROR_ALLOC;

	status->data.address = parser->base.address;
	status->data.type = MODBUS_HOLDING_REGISTER;
	status->data.first = modbusSwapEndian( parser->response06.reg );
	status->data.count = 1;
	( (uint16_t*) status->data.data )[0] = modbusSwapEndian( parser->response06.value );
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
	if ( status->request.length < 7u || status->request.length != 9 + requestParser->request16.byteCount ) return MODBUS_ERROR_FRAME;
	if ( status->response.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->response16.address == requestParser->request16.address;
	dataok &= parser->response16.function == requestParser->request16.function;
	dataok &= parser->response16.firstRegister == requestParser->request16.firstRegister;
	dataok &= parser->response16.registerCount == requestParser->request16.registerCount;
	dataok &= modbusSwapEndian( parser->response16.registerCount ) <= 123;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	//Set up data length - response successfully parsed
	status->data.address = parser->base.address;
	status->data.type = MODBUS_HOLDING_REGISTER;
	status->data.first = modbusSwapEndian( parser->response16.firstRegister );
	status->data.count = modbusSwapEndian( parser->response16.registerCount );
	status->data.regs = NULL;
	status->data.length = 0;
	return MODBUS_ERROR_OK;
}
