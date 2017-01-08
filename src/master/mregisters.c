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

uint8_t modbusBuildRequest03( ModbusMaster *status, uint8_t address, uint16_t firstRegister, uint16_t registerCount )
{
	//Build request03 frame, to send it so slave
	//Read multiple holding registers

	//Set frame length
	uint8_t frameLength = 8;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Set output frame length to 0 (in case of interrupts)
	status->request.length = 0;
	status->finished = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( registerCount == 0 || registerCount > 125 || address == 0 )
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
	builder->base.function = 3;
	builder->request03.firstRegister = modbusSwapEndian( firstRegister );
	builder->request03.registerCount = modbusSwapEndian( registerCount );

	//Calculate crc
	builder->request03.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	status->predictedResponseLength = 4 + 1 + ( registerCount << 1 );
	status->finished = 1;

	return 0;
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

	builder->base.address = address;
	builder->base.function = 6;
	builder->request06.reg = modbusSwapEndian( reg );
	builder->request06.value = modbusSwapEndian( value );

	//Calculate crc
	builder->request06.crc = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 8;
	status->finished = 1;

	return 0;
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
	status->finished = 0;
	status->predictedResponseLength = 0;

	//Check values pointer
	if ( values == NULL || registerCount == 0 || registerCount > 123 )
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
	builder->base.function = 16;
	builder->request16.firstRegister = modbusSwapEndian( firstRegister );
	builder->request16.registerCount = modbusSwapEndian( registerCount );
	builder->request16.byteCount = registerCount << 1;

	for ( i = 0; i < registerCount; i++ )
		builder->request16.values[i] = modbusSwapEndian( values[i] );

	builder->request16.values[registerCount] = modbusCRC( builder->frame, frameLength - 2 );

	status->request.length = frameLength;
	if ( address ) status->predictedResponseLength = 4 + 4;
	status->finished = 1;

	return 0;
}

uint8_t modbusParseResponse03( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 03
	//Read multiple holding registers

	//Update frame length
	uint8_t frameLength;
	uint8_t dataok = 1;
	uint8_t i = 0;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL || requestParser == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}
	frameLength = 5 + parser->response03.byteCount;

	//Check frame crc
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->response03.values[ parser->response03.byteCount >> 1 ] )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Check between data sent to slave and received from slave
	dataok &= parser->base.address != 0;
	dataok &= parser->response03.address == requestParser->request03.address;
	dataok &= parser->response03.function == requestParser->request03.function;
	dataok &= parser->response03.byteCount != 0;
	dataok &= parser->response03.byteCount == modbusSwapEndian( requestParser->request03.registerCount ) << 1 ;
	dataok &= parser->response03.byteCount <= 250;

	//If data is bad, abort parsing, and set error flag
	if ( !dataok )
	{
		status->finished = 1;
		return MODBUS_ERROR_FRAME;
	}

	//Allocate memory for ModbusData structures array
	free( status->data );
	status->data = (ModbusData *) malloc( ( parser->response03.byteCount >> 1 ) * sizeof( ModbusData ) );
	if ( status->data == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}

	//Copy received data to output structures array
	for ( i = 0; i < ( parser->response03.byteCount >> 1 ); i++ )
	{
		status->data[i].address = parser->base.address;
		status->data[i].dataType = MODBUS_HOLDING_REGISTER;
		status->data[i].reg = modbusSwapEndian( requestParser->request03.firstRegister ) + i;
		status->data[i].value = modbusSwapEndian( parser->response03.values[i] );
	}

	//Set up data length - response successfully parsed
	status->dataLength = parser->response03.byteCount >> 1;
	status->finished = 1;

	return 0;
}

uint8_t modbusParseResponse06( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 06 (write single holding reg)

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
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->response06.crc )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Check between data sent to slave and received from slave
	dataok &= parser->response06.address == requestParser->request06.address;
	dataok &= parser->response06.function == requestParser->request06.function;
	dataok &= parser->response06.reg == requestParser->request06.reg;
	dataok &= parser->response06.value == requestParser->request06.value;

	//If data is bad abort parsing, and set error flag
	if ( !dataok )
	{
		status->finished = 1;
		return MODBUS_ERROR_FRAME;
	}

	//Swap endianness
	parser->response06.reg = modbusSwapEndian( parser->response06.reg );
	parser->response06.value = modbusSwapEndian( parser->response06.value );

	//Set up new data table
	free( status->data );
	status->data = (ModbusData *) malloc( sizeof( ModbusData ) );
	if ( status->data == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}

	status->data[0].address = parser->base.address;
	status->data[0].dataType = MODBUS_HOLDING_REGISTER;
	status->data[0].reg = parser->response06.reg;
	status->data[0].value = parser->response06.value;

	//Set up data length - response successfully parsed
	status->dataLength = 1;
	status->finished = 1;
	return 0;
}

uint8_t modbusParseResponse16( ModbusMaster *status, union ModbusParser *parser, union ModbusParser *requestParser )
{
	//Parse slave response to request 16 (write multiple holding reg)

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
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->response16.crc )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Check between data sent to slave and received from slave
	dataok &= parser->response16.address == requestParser->request16.address;
	dataok &= parser->response16.function == requestParser->request16.function;
	dataok &= parser->response16.firstRegister == requestParser->request16.firstRegister;
	dataok &= parser->response16.registerCount == requestParser->request16.registerCount;
	dataok &= modbusSwapEndian( parser->response16.registerCount ) <= 123;

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
