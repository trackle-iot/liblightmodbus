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

uint8_t modbusParseRequest0304( ModbusSlave *status, union ModbusParser *parser )
{
	//Read multiple holding registers or input registers
	//Using data from union pointer

	//Update frame length
	uint8_t frameLength = 8;
	uint8_t i = 0;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL || ( parser->base.function != 3 && parser->base.function != 4 ) ) //That's actually safe
	{
		status->finished = 1;
		return MODBUS_ERROR_OTHER;
	}

	//Don't do anything when frame is broadcasted
	//Base of the frame can be always safely checked, because main parser function takes care of that
	if ( parser->base.address == 0 )
	{
		status->finished = 1;
		return MODBUS_ERROR_OK;
	}

	//Check if frame length is valid
	if ( status->request.length != frameLength )
	{
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_VAL );
	}

	//Swap endianness of longer members (but not crc)
	parser->request0304.firstRegister = modbusSwapEndian( parser->request0304.firstRegister );
	parser->request0304.registerCount = modbusSwapEndian( parser->request0304.registerCount );

	//Check if reg is in valid range
	if ( parser->request0304.registerCount == 0 || parser->request0304.registerCount > 125 )
	{
		//Illegal data value error
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_VAL );
	}

	if ( parser->request0304.firstRegister >= ( parser->base.function == 3 ? status->registerCount : status->inputRegisterCount ) || \
		(uint32_t) parser->request0304.firstRegister + (uint32_t) parser->request0304.registerCount > \
		(uint32_t) ( parser->base.function == 3 ? status->registerCount : status->inputRegisterCount ) )
	{
		//Illegal data address exception
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_ADDR );
	}

	//Respond
	frameLength = 5 + ( parser->request0304.registerCount << 1 );

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//Set up basic response data
	builder->response0304.address = status->address;
	builder->response0304.function = parser->request0304.function;
	builder->response0304.byteCount = parser->request0304.registerCount << 1;

	//Copy registers to response frame
	for ( i = 0; i < parser->request0304.registerCount; i++ )
		builder->response0304.values[i] = modbusSwapEndian( ( parser->base.function == 3 ? status->registers : status->inputRegisters )[parser->request0304.firstRegister + i] );

	//Calculate crc
	builder->response0304.values[parser->request0304.registerCount] = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	status->finished = 1;

	return MODBUS_ERROR_OK;
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

	//Check if frame length is valid
	if ( status->request.length != frameLength )
	{
		if ( parser->base.address != 0 ) return modbusBuildException( status, 6, MODBUS_EXCEP_ILLEGAL_VAL );
		status->finished = 1;
		return MODBUS_ERROR_OK;
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
		if ( parser->base.address != 0 ) return modbusBuildException( status, 6, MODBUS_EXCEP_ILLEGAL_ADDR );
		status->finished = 1;
		return MODBUS_ERROR_OK;
	}

	//Check if reg is allowed to be written
	if ( modbusMaskRead( status->registerMask, status->registerMaskLength, parser->request06.reg ) == 1 )
	{
		//Slave failure exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 6, MODBUS_EXCEP_SLAVE_FAIL );
		status->finished = 1;
		return MODBUS_ERROR_OK;
	}

	//Respond
	frameLength = 8;

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//After all possible exceptions, write reg
	status->registers[parser->request06.reg] = parser->request06.value;

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 )
	{
		status->finished = 1;
		return MODBUS_ERROR_OK;
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

	return MODBUS_ERROR_OK;
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

	//Check if frame length is valid
	if ( status->request.length >= 7u )
	{
		frameLength = 9 + parser->request16.byteCount;
		if ( status->request.length != frameLength )
		{
			return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
			status->finished = 1;
			return MODBUS_ERROR_OK;
		}
	}
	else
	{
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
		status->finished = 1;
		return MODBUS_ERROR_OK;
	}

	//Check frame crc
	//Shifting is used instead of dividing for optimisation on smaller devices (AVR)
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->request16.values[parser->request16.byteCount >> 1] )
	{
		status->finished = 1;
		return MODBUS_ERROR_CRC;
	}

	//Swap endianness of longer members (but not crc)
	parser->request16.firstRegister = modbusSwapEndian( parser->request16.firstRegister );
	parser->request16.registerCount = modbusSwapEndian( parser->request16.registerCount );

	//Data checks
	if ( parser->request16.byteCount == 0 || \
		parser->request16.registerCount == 0 || \
		parser->request16.registerCount != ( parser->request16.byteCount >> 1 ) || \
		parser->request16.registerCount > 123 )
	{
		//Illegal data value error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
		status->finished = 1;
		return MODBUS_ERROR_OK;
	}

	if ( parser->request16.firstRegister >= status->registerCount || \
		(uint32_t) parser->request16.firstRegister + (uint32_t) parser->request16.registerCount > (uint32_t) status->registerCount )
	{
		//Illegal data address error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_ADDR );
		status->finished = 1;
		return MODBUS_ERROR_OK;
	}

	//Check for write protection
	for ( i = 0; i < parser->request16.registerCount; i++ )
		if ( modbusMaskRead( status->registerMask, status->registerMaskLength, parser->request16.firstRegister + i ) == 1 )
		{
			//Slave failure exception
			if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_SLAVE_FAIL );
			status->finished = 1;
			return MODBUS_ERROR_OK;
		}

	//Respond
	frameLength = 8;

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL )
	{
		status->finished = 1;
		return MODBUS_ERROR_ALLOC;
	}
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;


	//After all possible exceptions, write values to registers
	for ( i = 0; i < parser->request16.registerCount; i++ )
		status->registers[parser->request16.firstRegister + i] = modbusSwapEndian( parser->request16.values[i] );

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 )
	{
		status->finished = 1;
		return MODBUS_ERROR_OK;
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

	return MODBUS_ERROR_OK;
}
