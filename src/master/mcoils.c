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
#include <lightmodbus/master/mcoils.h>

uint8_t modbusBuildRequest0102( ModbusMaster *status, uint8_t function, uint8_t address, uint16_t firstCoil, uint16_t coilCount )
{
	//Build request01 frame, to send it so slave
	//Read multiple coils

	//Set frame length
	uint8_t frameLength = 8;

	//Check if given pointer is valid
	if ( status == NULL || ( function != 1 && function != 2 ) ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( coilCount == 0 || coilCount > 2000 || address == 0 ) return MODBUS_ERROR_OTHER;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) );
	if ( status->request.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = function;
	builder->request0102.firstCoil = modbusSwapEndian( firstCoil );
	builder->request0102.coilCount = modbusSwapEndian( coilCount );

	//Calculate crc
	builder->request0102.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	status->predictedResponseLength = 4 + 1 + BITSTOBYTES( coilCount );

	return MODBUS_ERROR_OK;
}

uint8_t modbusBuildRequest05( ModbusMaster *status, uint8_t address, uint16_t coil, uint16_t value )
{
	//Build request05 frame, to send it so slave
	//Write single coil

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

	value = ( value != 0 ) ? 0xFF00 : 0x0000;

	builder->base.address = address;
	builder->base.function = 5;
	builder->request05.coil = modbusSwapEndian( coil );
	builder->request05.value = modbusSwapEndian( value );

	//Calculate crc
	builder->request05.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 8;

	return MODBUS_ERROR_OK;
}

uint8_t modbusBuildRequest15( ModbusMaster *status, uint8_t address, uint16_t firstCoil, uint16_t coilCount, uint8_t *values )
{
	//Build request15 frame, to send it so slave
	//Write multiple coils

	//Set frame length
	uint8_t frameLength = 9 + BITSTOBYTES( coilCount );
	uint8_t i = 0;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( values == NULL || coilCount == 0 || coilCount > 1968 ) return MODBUS_ERROR_OTHER;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) );
	if ( status->request.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = 15;
	builder->request15.firstCoil = modbusSwapEndian( firstCoil );
	builder->request15.coilCount = modbusSwapEndian( coilCount );
	builder->request15.byteCount = BITSTOBYTES( coilCount );

	for ( i = 0; i < builder->request15.byteCount; i++ )
		builder->request15.values[i] = values[i];

	*( (uint16_t*)( builder->frame + frameLength - 2 ) ) = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 4 + 4;

	return MODBUS_ERROR_OK;
}

uint8_t modbusParseResponse0102( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 01 (read multiple coils)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL || ( parser->base.function != 1 && parser->base.function != 2 ) )
		return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	//Frame has to be at least 4 bytes long so byteCount can always be accessed in this case
	if ( status->response.length != 5 + parser->response0102.byteCount || status->request.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address != 0;
	dataok &= parser->base.address == requestParser->base.address;
	dataok &= parser->base.function == requestParser->base.function;
	dataok &= parser->response0102.byteCount != 0;
	dataok &= parser->response0102.byteCount <= 250;
	dataok &= parser->response0102.byteCount == BITSTOBYTES( modbusSwapEndian( requestParser->request0102.coilCount ) );

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	status->data.coils = (uint8_t*) calloc( BITSTOBYTES( modbusSwapEndian( requestParser->request0102.coilCount ) ), sizeof( uint8_t ) );
	status->data.regs = (uint16_t*) status->data.coils;
	if ( status->data.coils == NULL ) return MODBUS_ERROR_ALLOC;

	status->data.address = parser->base.address;
	status->data.type = parser->base.function == 1 ? MODBUS_COIL : MODBUS_DISCRETE_INPUT;
	status->data.index = modbusSwapEndian( requestParser->request0102.firstCoil );
	status->data.count = modbusSwapEndian( requestParser->request0102.coilCount );
	memcpy( status->data.coils, parser->response0102.values, parser->response0102.byteCount );
	status->data.length = parser->response0102.byteCount;
	return MODBUS_ERROR_OK;
}

uint8_t modbusParseResponse05( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 05 (write single coil)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL ) return MODBUS_ERROR_OTHER;

	//Check frame lengths
	if ( status->response.length != 8 || status->request.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= ( parser->base.address == requestParser->base.address );
	dataok &= ( parser->base.function == requestParser->base.function );

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	status->data.coils = (uint8_t*) calloc( 1, sizeof( uint8_t ) );
	status->data.regs = (uint16_t*) status->data.coils;
	if ( status->data.coils == NULL ) return MODBUS_ERROR_ALLOC;
	status->data.address = parser->base.address;
	status->data.type = MODBUS_COIL;
	status->data.index = modbusSwapEndian( requestParser->request05.coil );
	status->data.count = 1;
	status->data.coils[0] = parser->response05.value != 0;
	status->data.length = 1;
	return MODBUS_ERROR_OK;
}

uint8_t modbusParseResponse15( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 15 (write multiple coils)

	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL || requestParser == NULL ) return MODBUS_ERROR_OTHER;

	//Check frame lengths
	if ( status->request.length < 7u || status->request.length != 9 + requestParser->request15.byteCount ) return MODBUS_ERROR_FRAME;
	if ( status->response.length != 8 ) return MODBUS_ERROR_FRAME;

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address == requestParser->base.address;
	dataok &= parser->base.function == requestParser->base.function;
	dataok &= parser->response15.firstCoil == requestParser->request15.firstCoil;
	dataok &= parser->response15.coilCount == requestParser->request15.coilCount;

	//If data is bad abort parsing, and set error flag
	if ( !dataok ) return MODBUS_ERROR_FRAME;

	status->data.address = parser->base.address;
	status->data.type = MODBUS_COIL;
	status->data.index = modbusSwapEndian( parser->response15.firstCoil );
	status->data.count = modbusSwapEndian( parser->response15.coilCount );
	status->data.length = 0;
	return MODBUS_ERROR_OK;
}
