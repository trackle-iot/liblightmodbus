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

uint8_t modbusBuildRequest01( ModbusMaster *status, uint8_t address, uint16_t firstCoil, uint16_t coilCount )
{
	//Build request01 frame, to send it so slave
	//Read multiple coils

	//Set frame length
	uint8_t frameLength = 8;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->finished = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( coilCount == 0 || coilCount > 2000 )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) malloc( frameLength );
	if ( status->request.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = 1;
	builder->request01.firstCoil = modbusSwapEndian( firstCoil );
	builder->request01.coilCount = modbusSwapEndian( coilCount );

	//Calculate crc
	builder->request01.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	status->predictedResponseLength = 4 + 2 + ( ( coilCount - 1 ) >> 3 );
	status->finished = 1;

	return 0;
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
	status->finished = 0;
	status->predictedResponseLength = 0;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) malloc( frameLength );
	if ( status->request.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	value = ( value != 0 ) ? 0xFF00 : 0x0000;

	builder->base.address = address;
	builder->base.function = 5;
	builder->request05.coil = modbusSwapEndian( coil );
	builder->request05.value = modbusSwapEndian( value );

	//Calculate crc
	builder->request01.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	status->predictedResponseLength = 8;
	status->finished = 1;

	return 0;
}

uint8_t modbusBuildRequest15( ModbusMaster *status, uint8_t address, uint16_t firstCoil, uint16_t coilCount, uint8_t *values )
{
	//Build request15 frame, to send it so slave
	//Write multiple coils

	//Set frame length
	uint8_t frameLength = 10 + ( ( coilCount - 1 ) >> 3 );
	uint8_t i = 0;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->finished = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( values == NULL || coilCount == 0 || coilCount > 1968 )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}

	if ( coilCount > 256 ) return 1;

	//Reallocate memory for final frame
	free( status->request.frame );
	status->request.frame = (uint8_t *) malloc( frameLength );
	if ( status->request.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	union ModbusParser *builder = (union ModbusParser *) status->request.frame;

	builder->base.address = address;
	builder->base.function = 15;
	builder->request15.firstCoil = modbusSwapEndian( firstCoil );
	builder->request15.coilCount = modbusSwapEndian( coilCount );
	builder->request15.byteCount = 1 + ( ( coilCount - 1 ) >> 3 );

	for ( i = 0; i < builder->request15.byteCount; i++ )
		builder->request15.values[i] = values[i];

	*( (uint16_t*)( builder->frame + frameLength - 2 ) ) = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	status->predictedResponseLength = 4 + 4;
	status->finished = 1;

	return 0;
}

uint8_t modbusParseResponse01( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 01 (read multiple coils)

	//Update frame length
	uint8_t frameLength;
	uint8_t dataok = 1;
	uint8_t i = 0;

	//Check if given pointers are valid
	if ( status == NULL  ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL || requestParser == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}
	frameLength = 5 + parser->response01.byteCount;

	//Check frame crc
	if ( modbusCRC( parser->frame, frameLength - 2 ) != *( (uint16_t*)( parser->response01.values + parser->response01.byteCount ) ) )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address == requestParser->base.address;
	dataok &= parser->base.function == requestParser->base.function;
	dataok &= parser->response01.byteCount != 0;
	dataok &= parser->response01.byteCount <= 250;
	dataok &= parser->response01.byteCount == 1 + ( ( modbusSwapEndian( requestParser->request01.coilCount ) - 1 ) >> 3 );

	//If data is bad abort parsing, and set error flag
	if ( !dataok )
	{
		status->finished = 1;
		return MODBUS_ERROR_FRAME;
	}

	free( status->data );
	status->data = (ModbusData *) malloc( sizeof( ModbusData ) * modbusSwapEndian( requestParser->request01.coilCount ) );
	if ( status->data == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}

	for ( i = 0; i < modbusSwapEndian( requestParser->request01.coilCount ); i++ )
	{
		status->data[i].address = parser->base.address;
		status->data[i].dataType = MODBUS_COIL;
		status->data[i].reg = modbusSwapEndian( requestParser->request01.firstCoil ) + i;
		status->data[i].value = modbusMaskRead( parser->response01.values, parser->response01.byteCount, i );
		if ( status->data[i].value == 255 )
		{
			status->finished = 1;
			return MODBUS_ERROR_OTHER;
		}
	}

	//Set up data length - response successfully parsed
	status->dataLength = modbusSwapEndian( requestParser->request01.coilCount );
	status->finished = 1;

	return 0;
}

uint8_t modbusParseResponse05( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 05 (write single coil)

	//Update frame length
	uint8_t frameLength = 8;
	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL || requestParser == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}

	//Check frame crc
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->response05.crc )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Check between data sent to slave and received from slave
	dataok &= ( parser->base.address == requestParser->base.address );
	dataok &= ( parser->base.function == requestParser->base.function );

	//If data is bad abort parsing, and set error flag
	if ( !dataok )
	{
		status->finished = 1;
		return MODBUS_ERROR_FRAME;
	}

	free( status->data );
	status->data = (ModbusData *) malloc( sizeof( ModbusData ) );
	if ( status->data == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}

	status->data[0].address = parser->base.address;
	status->data[0].dataType = MODBUS_COIL;
	status->data[0].reg = modbusSwapEndian( requestParser->request05.coil );
	status->data[0].value = parser->response05.value != 0;

	//Set up data length - response successfully parsed
	status->dataLength = 1;
	status->finished = 1;
	return 0;
}

uint8_t modbusParseResponse15( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 15 (write multiple coils)

	//Update frame length
	uint8_t frameLength = 8;
	uint8_t dataok = 1;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL || requestParser == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}

	//Check frame crc
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->response15.crc )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}


	//Check between data sent to slave and received from slave
	dataok &= parser->base.address == requestParser->base.address;
	dataok &= parser->base.function == requestParser->base.function;
	dataok &= parser->response15.firstCoil == requestParser->request15.firstCoil;
	dataok &= parser->response15.coilCount == requestParser->request15.coilCount;

	//If data is bad abort parsing, and set error flag
	if ( !dataok )
	{
		status->finished = 1;
		return MODBUS_ERROR_FRAME;
	}

	//Set up data length - response successfully parsed
	status->dataLength = 0;
	status->finished = 1;
	return 0;
}
