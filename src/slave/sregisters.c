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
#include <lightmodbus/slave/stypes.h>
#include <lightmodbus/slave/sregisters.h>

uint8_t modbusParseRequest03( ModbusSlave *status, union ModbusParser *parser )
{
	//Read multiple holding registers
	//Using data from union pointer

	//Update frame length
	uint8_t frameLength = 8;
	uint8_t i = 0;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}

	//Check frame crc
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->request03.crc )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Don't do anything when frame is broadcasted
	if ( parser->base.address == 0 )
	{
		status->finished = 1;
		return 0;
	}

	//Swap endianness of longer members (but not crc)
	parser->request03.firstRegister = modbusSwapEndian( parser->request03.firstRegister );
	parser->request03.registerCount = modbusSwapEndian( parser->request03.registerCount );

	//Check if reg is in valid range
	if ( parser->request03.registerCount == 0 )
	{
		//Illegal data value error
		return modbusBuildException( status, 0x03, MODBUS_EXCEP_ILLEGAL_VAL );
	}

	if ( parser->request03.firstRegister >= status->registerCount || (uint32_t) parser->request03.firstRegister + (uint32_t) parser->request03.registerCount > (uint32_t) status->registerCount )
	{
		//Illegal data address exception
		return modbusBuildException( status, 0x03, MODBUS_EXCEP_ILLEGAL_ADDR );
	}

	//Respond
	frameLength = 5 + ( parser->request03.registerCount << 1 );

	status->response.frame = (uint8_t *) malloc( frameLength ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	memset( status->response.frame, 0, frameLength ); //Empty response frame
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//Set up basic response data
	builder->response03.address = status->address;
	builder->response03.function = parser->request03.function;
	builder->response03.byteCount = parser->request03.registerCount << 1;

	//Copy registers to response frame
	for ( i = 0; i < parser->request03.registerCount; i++ )
		builder->response03.values[i] = modbusSwapEndian( status->registers[parser->request03.firstRegister + i] );

	//Calculate crc
	builder->response03.values[parser->request03.registerCount] = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	status->finished = 1;

	return 0;
}

uint8_t modbusParseRequest06( ModbusSlave *status, union ModbusParser *parser )
{
	//Write single holding reg
	//Using data from union pointer

	//Update frame length
	uint8_t frameLength = 8;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}

	//Check frame crc
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->request06.crc )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Swap endianness of longer members (but not crc)
	parser->request06.reg = modbusSwapEndian( parser->request06.reg );
	parser->request06.value = modbusSwapEndian( parser->request06.value );

	//Check if reg is in valid range
	if ( parser->request06.reg >= status->registerCount )
	{
		//Illegal data address exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 0x06, MODBUS_EXCEP_ILLEGAL_ADDR );
		status->finished = 1;
		return 0;
	}

	//Check if reg is allowed to be written
	if ( modbusMaskRead( status->registerMask, status->registerMaskLength, parser->request06.reg ) == 1 )
	{
		//Slave failure exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 0x06, MODBUS_EXCEP_SLAVE_FAIL );
		status->finished = 1;
		return 0;
	}

	//Respond
	frameLength = 8;

	status->response.frame = (uint8_t *) malloc( frameLength ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	memset( status->response.frame, 0, frameLength ); //Empty response frame
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//After all possible exceptions, write reg
	status->registers[parser->request06.reg] = parser->request06.value;

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 )
	{
		status->finished = 1;
		return 0;
	}

	//Set up basic response data
	builder->response06.address = status->address;
	builder->response06.function = parser->request06.function;
	builder->response06.reg = modbusSwapEndian( parser->request06.reg );
	builder->response06.value = modbusSwapEndian( status->registers[parser->request06.reg] );

	//Calculate crc
	builder->response06.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	status->finished = 1;

	return 0;
}

uint8_t modbusParseRequest16( ModbusSlave *status, union ModbusParser *parser )
{
	//Write multiple holding registers
	//Using data from union pointer

	//Update frame length
	uint8_t i = 0;
	uint8_t frameLength;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}
	frameLength = 9 + parser->request16.byteCount;

	//Check frame crc
	//Shifting is used instead of dividing for optimisation on smaller devices (AVR)
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->request16.values[parser->request16.byteCount >> 1] )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Check if bytes or registers count isn't 0
	if ( parser->request16.byteCount == 0 || parser->request16.registerCount == 0 )
	{
		//Illegal data value error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 0x10, MODBUS_EXCEP_ILLEGAL_VAL );
		status->finished = 1;
		return 0;
	}

	//Swap endianness of longer members (but not crc)
	parser->request16.firstRegister = modbusSwapEndian( parser->request16.firstRegister );
	parser->request16.registerCount = modbusSwapEndian( parser->request16.registerCount );

	//Check if bytes count *2 is equal to registers count
	if ( parser->request16.registerCount != ( parser->request16.byteCount >> 1 ) )
	{
		//Illegal data value error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 0x10, MODBUS_EXCEP_ILLEGAL_VAL );
		status->finished = 1;
		return 0;
	}

	if ( parser->request16.firstRegister >= status->registerCount || (uint32_t) parser->request16.firstRegister + (uint32_t) parser->request16.registerCount > (uint32_t) status->registerCount )
	{
		//Illegal data address error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 0x10, MODBUS_EXCEP_ILLEGAL_ADDR );
		status->finished = 1;
		return 0;
	}

	//Check for write protection
	for ( i = 0; i < parser->request16.registerCount; i++ )
		if ( modbusMaskRead( status->registerMask, status->registerMaskLength, parser->request16.firstRegister + i ) == 1 )
		{
			//Slave failure exception
			if ( parser->base.address != 0 ) return modbusBuildException( status, 0x10, MODBUS_EXCEP_SLAVE_FAIL );
			status->finished = 1;
			return 0;
		}

	//Respond
	frameLength = 8;

	status->response.frame = (uint8_t *) malloc( frameLength ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	memset( status->response.frame, 0, frameLength ); //Empty response frame
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;


	//After all possible exceptions, write values to registers
	for ( i = 0; i < parser->request16.registerCount; i++ )
		status->registers[parser->request16.firstRegister + i] = modbusSwapEndian( parser->request16.values[i] );

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 )
	{
		status->finished = 1;
		return 0;
	}

	//Set up basic response data
	builder->response16.address = status->address;
	builder->response16.function = parser->request16.function;
	builder->response16.firstRegister = modbusSwapEndian( parser->request16.firstRegister );
	builder->response16.registerCount = modbusSwapEndian( parser->request16.registerCount );

	//Calculate crc
	builder->response16.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	status->finished = 1;

	return 0;
}
