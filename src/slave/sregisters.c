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
	if ( status == NULL || parser == NULL || ( parser->base.function != 3 && parser->base.function != 4 ) ) return MODBUS_ERROR_OTHER;

	//Don't do anything when frame is broadcasted
	//Base of the frame can be always safely checked, because main parser function takes care of that
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Check if frame length is valid
	if ( status->request.length != frameLength )
	{
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_VAL );
	}

	//Swap endianness of longer members (but not crc)
	parser->request0304.index = modbusSwapEndian( parser->request0304.index );
	parser->request0304.count = modbusSwapEndian( parser->request0304.count );

	//Check if reg is in valid range
	if ( parser->request0304.count == 0 || parser->request0304.count > 125 )
	{
		//Illegal data value error
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_VAL );
	}

	if ( parser->request0304.index >= ( parser->base.function == 3 ? status->registerCount : status->inputRegisterCount ) || \
		(uint32_t) parser->request0304.index + (uint32_t) parser->request0304.count > \
		(uint32_t) ( parser->base.function == 3 ? status->registerCount : status->inputRegisterCount ) )
	{
		//Illegal data address exception
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_ADDR );
	}

	//Respond
	frameLength = 5 + ( parser->request0304.count << 1 );

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL )return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//Set up basic response data
	builder->response0304.address = status->address;
	builder->response0304.function = parser->request0304.function;
	builder->response0304.length = parser->request0304.count << 1;

	//Copy registers to response frame
	for ( i = 0; i < parser->request0304.count; i++ )
		builder->response0304.values[i] = modbusSwapEndian( ( parser->base.function == 3 ? status->registers : status->inputRegisters )[parser->request0304.index + i] );

	//Calculate crc
	builder->response0304.values[parser->request0304.count] = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}

uint8_t modbusParseRequest06( ModbusSlave *status, union ModbusParser *parser )
{
	//Write single holding reg
	//Using data from union pointer

	//Update frame length
	uint8_t frameLength = 8;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL ) return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	if ( status->request.length != frameLength )
	{
		if ( parser->base.address != 0 ) return modbusBuildException( status, 6, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	//Check frame crc
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->request06.crc ) return MODBUS_ERROR_CRC;

	//Swap endianness of longer members (but not crc)
	parser->request06.index = modbusSwapEndian( parser->request06.index );
	parser->request06.value = modbusSwapEndian( parser->request06.value );

	//Check if reg is in valid range
	if ( parser->request06.index >= status->registerCount )
	{
		//Illegal data address exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 6, MODBUS_EXCEP_ILLEGAL_ADDR );
		return MODBUS_ERROR_OK;
	}

	//Check if reg is allowed to be written
	if ( modbusMaskRead( status->registerMask, status->registerMaskLength, parser->request06.index ) == 1 )
	{
		//Slave failure exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 6, MODBUS_EXCEP_SLAVE_FAIL );
		return MODBUS_ERROR_OK;
	}

	//Respond
	frameLength = 8;

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//After all possible exceptions, write reg
	status->registers[parser->request06.index] = parser->request06.value;

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Set up basic response data
	builder->response06.address = status->address;
	builder->response06.function = parser->request06.function;
	builder->response06.index = modbusSwapEndian( parser->request06.index );
	builder->response06.value = modbusSwapEndian( status->registers[parser->request06.index] );

	//Calculate crc
	builder->response06.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
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
	if ( status == NULL || parser == NULL ) return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	if ( status->request.length >= 7u )
	{
		frameLength = 9 + parser->request16.length;
		if ( status->request.length != frameLength )
		{
			return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
			return MODBUS_ERROR_OK;
		}
	}
	else
	{
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	//Check frame crc
	//Shifting is used instead of dividing for optimisation on smaller devices (AVR)
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->request16.values[parser->request16.length >> 1] ) return MODBUS_ERROR_CRC;

	//Swap endianness of longer members (but not crc)
	parser->request16.index = modbusSwapEndian( parser->request16.index );
	parser->request16.count = modbusSwapEndian( parser->request16.count );

	//Data checks
	if ( parser->request16.length == 0 || \
		parser->request16.count == 0 || \
		parser->request16.count != ( parser->request16.length >> 1 ) || \
		parser->request16.count > 123 )
	{
		//Illegal data value error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	if ( parser->request16.index >= status->registerCount || \
		(uint32_t) parser->request16.index + (uint32_t) parser->request16.count > (uint32_t) status->registerCount )
	{
		//Illegal data address error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_ADDR );
		return MODBUS_ERROR_OK;
	}

	//Check for write protection
	for ( i = 0; i < parser->request16.count; i++ )
		if ( modbusMaskRead( status->registerMask, status->registerMaskLength, parser->request16.index + i ) == 1 )
		{
			//Slave failure exception
			if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_SLAVE_FAIL );
			return MODBUS_ERROR_OK;
		}

	//Respond
	frameLength = 8;

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;


	//After all possible exceptions, write values to registers
	for ( i = 0; i < parser->request16.count; i++ )
		status->registers[parser->request16.index + i] = modbusSwapEndian( parser->request16.values[i] );

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Set up basic response data
	builder->response16.address = status->address;
	builder->response16.function = parser->request16.function;
	builder->response16.index = modbusSwapEndian( parser->request16.index );
	builder->response16.count = modbusSwapEndian( parser->request16.count );

	//Calculate crc
	builder->response16.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}

uint8_t modbusParseRequest22( ModbusSlave *status, union ModbusParser *parser )
{
	//Mask write single holding reg
	//Using data from union pointer

	//Update frame length
	uint8_t frameLength = 10;

	//Check if given pointers are valid
	if ( status == NULL || parser == NULL ) return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	if ( status->request.length != frameLength )
	{
		if ( parser->base.address != 0 ) return modbusBuildException( status, 22, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	//Check frame crc
	if ( modbusCRC( parser->frame, frameLength - 2 ) != parser->request22.crc ) return MODBUS_ERROR_CRC;

	//Swap endianness of longer members (but not crc)
	parser->request22.index = modbusSwapEndian( parser->request22.index );
	parser->request22.andmask = modbusSwapEndian( parser->request22.andmask );
	parser->request22.ormask = modbusSwapEndian( parser->request22.ormask );

	//Check if reg is in valid range
	if ( parser->request22.index >= status->registerCount )
	{
		//Illegal data address exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 22, MODBUS_EXCEP_ILLEGAL_ADDR );
		return MODBUS_ERROR_OK;
	}

	//Check if reg is allowed to be written
	if ( modbusMaskRead( status->registerMask, status->registerMaskLength, parser->request22.index ) == 1 )
	{
		//Slave failure exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 22, MODBUS_EXCEP_SLAVE_FAIL );
		return MODBUS_ERROR_OK;
	}

	//Respond
	frameLength = 10;

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//After all possible exceptions, write reg
	status->registers[parser->request06.index] = ( status->registers[parser->request22.index] & parser->request22.andmask ) | \
		( parser->request22.ormask & ~parser->request22.andmask );

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Set up basic response data
	builder->response22.address = status->address;
	builder->response22.function = parser->request22.function;
	builder->response22.index = modbusSwapEndian( parser->request22.index );
	builder->response22.andmask = modbusSwapEndian( parser->request22.andmask );
	builder->response22.ormask = modbusSwapEndian( parser->request22.ormask );

	//Calculate crc
	builder->response22.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}
