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
#include <lightmodbus/slave.h>
#include <lightmodbus/slave/sregs.h>

#if defined(LIGHTMODBUS_F03S) || defined(LIGHTMODBUS_F04S)
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
	uint16_t index = modbusSwapEndian( parser->request0304.index );
	uint16_t count = modbusSwapEndian( parser->request0304.count );

	//Check if reg is in valid range
	if ( count == 0 || count > 125 )
	{
		//Illegal data value error
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_VAL );
	}

	if ( index >= ( parser->base.function == 3 ? status->registerCount : status->inputRegisterCount ) || \
		(uint32_t) index + (uint32_t) count > \
		(uint32_t) ( parser->base.function == 3 ? status->registerCount : status->inputRegisterCount ) || \
	 	( parser->base.function == 3 ? status->registers : status->inputRegisters ) == NULL )
	{
		//Illegal data address exception
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_ADDR );
	}

	//Respond
	frameLength = 5 + ( count << 1 );

	#ifndef LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE
		status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
		if ( status->response.frame == NULL )return MODBUS_ERROR_ALLOC;
	#else
		if ( frameLength > LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE ) return MODBUS_ERROR_ALLOC;
	#endif

	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//Set up basic response data
	builder->response0304.address = status->address;
	builder->response0304.function = parser->request0304.function;
	builder->response0304.length = count << 1;

	//Copy registers to response frame
	for ( i = 0; i < count; i++ )
		builder->response0304.values[i] = modbusSwapEndian( ( parser->base.function == 3 ? status->registers : status->inputRegisters )[index + i] );

	//Calculate crc
	builder->response0304.values[count] = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}
#endif

#ifdef LIGHTMODBUS_F06S
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

	//Swap endianness of longer members (but not crc)
	uint16_t index = modbusSwapEndian( parser->request06.index );
	uint16_t value = modbusSwapEndian( parser->request06.value );

	//Check if reg is in valid range
	if ( index >= status->registerCount || status->registers == NULL )
	{
		//Illegal data address exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 6, MODBUS_EXCEP_ILLEGAL_ADDR );
		return MODBUS_ERROR_OK;
	}

	//Check if reg is allowed to be written
	if ( modbusMaskRead( status->registerMask, status->registerMaskLength, index ) == 1 )
	{
		//Slave failure exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 6, MODBUS_EXCEP_SLAVE_FAIL );
		return MODBUS_ERROR_OK;
	}

	//Respond
	frameLength = 8;

	#ifndef LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE
		status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
		if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	#else
		if ( frameLength > LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE ) return MODBUS_ERROR_ALLOC;
	#endif

	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//After all possible exceptions, write reg
	status->registers[index] = value;

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Set up basic response data
	builder->response06.address = status->address;
	builder->response06.function = parser->request06.function;
	builder->response06.index = parser->request06.index;
	builder->response06.value = modbusSwapEndian( status->registers[index] );

	//Calculate crc
	builder->response06.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}
#endif

#ifdef LIGHTMODBUS_F16S
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
			if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
			return MODBUS_ERROR_OK;
		}
	}
	else
	{
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	//Swap endianness of longer members (but not crc)
	uint16_t index = modbusSwapEndian( parser->request16.index );
	uint16_t count = modbusSwapEndian( parser->request16.count );

	//Data checks
	if ( parser->request16.length == 0 || \
		count == 0 || \
		count != ( parser->request16.length >> 1 ) || \
		count > 123 )
	{
		//Illegal data value error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	if ( index >= status->registerCount || \
		status->registers == NULL || \
		(uint32_t) index + (uint32_t) count > (uint32_t) status->registerCount )
	{
		//Illegal data address error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_ILLEGAL_ADDR );
		return MODBUS_ERROR_OK;
	}

	//Check for write protection
	for ( i = 0; i < count; i++ )
		if ( modbusMaskRead( status->registerMask, status->registerMaskLength, index + i ) == 1 )
		{
			//Slave failure exception
			if ( parser->base.address != 0 ) return modbusBuildException( status, 16, MODBUS_EXCEP_SLAVE_FAIL );
			return MODBUS_ERROR_OK;
		}

	//Respond
	frameLength = 8;

	#ifndef LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE
		status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
		if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	#else
		if ( frameLength > LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE ) return MODBUS_ERROR_ALLOC;
	#endif

	union ModbusParser *builder = (union ModbusParser *) status->response.frame;


	//After all possible exceptions, write values to registers
	for ( i = 0; i < count; i++ )
		status->registers[index + i] = modbusSwapEndian( parser->request16.values[i] );

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Set up basic response data
	builder->response16.address = status->address;
	builder->response16.function = parser->request16.function;
	builder->response16.index = parser->request16.index;
	builder->response16.count = parser->request16.count;

	//Calculate crc
	builder->response16.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}
#endif

#ifdef LIGHTMODBUS_F22S
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
	uint16_t index = modbusSwapEndian( parser->request22.index );
	uint16_t andmask = modbusSwapEndian( parser->request22.andmask );
	uint16_t ormask = modbusSwapEndian( parser->request22.ormask );

	//Check if reg is in valid range
	if ( index >= status->registerCount || status->registers == NULL )
	{
		//Illegal data address exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 22, MODBUS_EXCEP_ILLEGAL_ADDR );
		return MODBUS_ERROR_OK;
	}

	//Check if reg is allowed to be written
	if ( modbusMaskRead( status->registerMask, status->registerMaskLength, index ) == 1 )
	{
		//Slave failure exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 22, MODBUS_EXCEP_SLAVE_FAIL );
		return MODBUS_ERROR_OK;
	}

	//Respond
	frameLength = 10;

	#ifndef LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE
		status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
		if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	#else
		if ( frameLength > LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE ) return MODBUS_ERROR_ALLOC;
	#endif

	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//After all possible exceptions, write reg
	status->registers[index] = ( status->registers[index] & andmask ) | ( ormask & ~andmask );

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Set up basic response data
	builder->response22.address = status->address;
	builder->response22.function = parser->request22.function;
	builder->response22.index = parser->request22.index;
	builder->response22.andmask = parser->request22.andmask;
	builder->response22.ormask = parser->request22.ormask;

	//Calculate crc
	builder->response22.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}
#endif
