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
#include <lightmodbus/slave/scoils.h>

uint8_t modbusParseRequest0102( ModbusSlave *status, union ModbusParser *parser )
{
	//Read multiple coils or discrete inputs
	//Using data from union pointer

	//Update frame length
	uint8_t frameLength = 8;
	uint8_t coil = 0;
	uint16_t i = 0;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL || ( parser->base.function != 1 && parser->base.function != 2 ) ) return MODBUS_ERROR_OTHER;

	//Don't do anything when frame is broadcasted
	//Base of the frame can be always safely checked, because main parser function takes care of that
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Check if frame length is valid
	if ( status->request.length != frameLength )
	{
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_VAL );
	}

	//Swap endianness of longer members (but not crc)
	parser->request0102.firstCoil = modbusSwapEndian( parser->request0102.firstCoil );
	parser->request0102.coilCount = modbusSwapEndian( parser->request0102.coilCount );

	//Check if coil is in valid range
	if ( parser->request0102.coilCount == 0 || parser->request0102.coilCount > 2000 )
	{
		//Illegal data value error
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_VAL );
	}

	if ( parser->request0102.firstCoil >= ( parser->base.function == 1 ? status->coilCount : status->discreteInputCount ) || \
		(uint32_t) parser->request0102.firstCoil + (uint32_t) parser->request0102.coilCount > \
		(uint32_t) ( parser->base.function == 1 ? status->coilCount : status->discreteInputCount ) )
	{
		//Illegal data address exception
		return modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_ADDR );
	}

	//Respond
	frameLength = 5 + BITSTOBYTES( parser->request0102.coilCount );

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//Set up basic response data
	builder->base.address = status->address;
	builder->base.function = parser->base.function;
	builder->response0102.byteCount = BITSTOBYTES( parser->request0102.coilCount );

	//Copy registers to response frame
	for ( i = 0; i < parser->request0102.coilCount; i++ )
	{
		if ( ( coil = modbusMaskRead( parser->base.function == 1 ? status->coils : status->discreteInputs, \
			BITSTOBYTES( status->coilCount ), i + parser->request0102.firstCoil ) ) == 255 )
				return MODBUS_ERROR_OTHER;
		if ( modbusMaskWrite( builder->response0102.values, 32, i, coil ) == 255 )
			return MODBUS_ERROR_OTHER;
	}

	//Calculate crc
	*( (uint16_t*)( builder->frame + frameLength - 2 ) ) = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}

uint8_t modbusParseRequest05( ModbusSlave *status, union ModbusParser *parser )
{
	//Write single coil
	//Using data from union pointer

	//Update frame length
	uint8_t frameLength = 8;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL ) return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	if ( status->request.length != frameLength )
	{
		if ( parser->base.address != 0 ) return modbusBuildException( status, 5, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	//Swap endianness of longer members (but not crc)
	parser->request05.coil = modbusSwapEndian( parser->request05.coil );
	parser->request05.value = modbusSwapEndian( parser->request05.value );

	//Check if coil value is valid
	if ( parser->request05.value != 0x0000 && parser->request05.value != 0xFF00 )
	{
		//Illegal data address error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 5, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	//Check if coil is in valid range
	if ( parser->request05.coil >= status->coilCount )
	{
		//Illegal data address error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 5, MODBUS_EXCEP_ILLEGAL_ADDR );
		return MODBUS_ERROR_OK;
	}

	//Check if reg is allowed to be written
	if ( modbusMaskRead( status->coilMask, status->coilMaskLength, parser->request05.coil ) == 1 )
	{
		//Slave failure exception
		if ( parser->base.address != 0 ) return modbusBuildException( status, 5, MODBUS_EXCEP_SLAVE_FAIL );
		return MODBUS_ERROR_OK;
	}

	//Respond
	frameLength = 8;


	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->response.frame;

	//After all possible exceptions, write coils
	if ( modbusMaskWrite( status->coils, BITSTOBYTES( status->coilCount ), parser->request05.coil, parser->request05.value == 0xFF00 ) == 255 )
		return MODBUS_ERROR_OTHER;

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Set up basic response data
	builder->base.address = status->address;
	builder->base.function = parser->base.function;
	builder->response05.coil = modbusSwapEndian( parser->request05.coil );
	builder->response05.value = modbusSwapEndian( parser->request05.value );

	//Calculate crc
	builder->response05.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return MODBUS_ERROR_OK;
}

uint8_t modbusParseRequest15( ModbusSlave *status, union ModbusParser *parser )
{
	//Write multiple coils
	//Using data from union pointer

	//Update frame length
	uint16_t i = 0;
	uint8_t frameLength;
	uint8_t coil = 0;

	//Check if given pointers are valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;
	if ( parser == NULL ) return MODBUS_ERROR_OTHER;

	//Check if frame length is valid
	if ( status->request.length >= 7u )
	{
		frameLength = 9 + parser->request15.byteCount;
		if ( status->request.length != frameLength )
		{
			if ( parser->base.address != 0 ) return modbusBuildException( status, 15, MODBUS_EXCEP_ILLEGAL_VAL );
			return MODBUS_ERROR_OK;
		}
	}
	else
	{
		if ( parser->base.address != 0 ) return modbusBuildException( status, 15, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	//Swap endianness of longer members (but not crc)
	parser->request15.firstCoil = modbusSwapEndian( parser->request15.firstCoil );
	parser->request15.coilCount = modbusSwapEndian( parser->request15.coilCount );

	//Data checks
	if ( parser->request15.byteCount == 0 || \
		parser->request15.coilCount == 0 || \
		BITSTOBYTES( parser->request15.coilCount ) != parser->request15.byteCount || \
		parser->request15.coilCount > 1968 )
	{
		//Illegal data value error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 15, MODBUS_EXCEP_ILLEGAL_VAL );
		return MODBUS_ERROR_OK;
	}

	if ( parser->request15.firstCoil >= status->coilCount || \
		(uint32_t) parser->request15.firstCoil + (uint32_t) parser->request15.coilCount > (uint32_t) status->coilCount )
	{
		//Illegal data address error
		if ( parser->base.address != 0 ) return modbusBuildException( status, 15, MODBUS_EXCEP_ILLEGAL_ADDR );
		return MODBUS_ERROR_OK;
	}

	//Check for write protection
	for ( i = 0; i < parser->request15.coilCount; i++ )
		if ( modbusMaskRead( status->coilMask, status->coilMaskLength, parser->request15.firstCoil + i ) == 1 )
		{
			//Slave failure exception
			if ( parser->base.address != 0 ) return modbusBuildException( status, 15, MODBUS_EXCEP_SLAVE_FAIL );
			return MODBUS_ERROR_OK;
		}

	//Respond
	frameLength = 8;

	status->response.frame = (uint8_t *) calloc( frameLength, sizeof( uint8_t ) ); //Reallocate response frame memory to needed memory
	if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *builder = (union ModbusParser *) status->response.frame; //Allocate memory for builder union

	//After all possible exceptions write values to registers
	for ( i = 0; i < parser->request15.coilCount; i++ )
	{
		if ( ( coil = modbusMaskRead( parser->request15.values, parser->request15.byteCount, i ) ) == 255 ) return MODBUS_ERROR_OTHER;
		if ( modbusMaskWrite( status->coils, status->coilCount, parser->request15.firstCoil + i, coil ) == 255 ) return MODBUS_ERROR_OTHER;
	}

	//Do not respond when frame is broadcasted
	if ( parser->base.address == 0 ) return MODBUS_ERROR_OK;

	//Set up basic response data
	builder->base.address = status->address;
	builder->base.function = parser->base.function;
	builder->response15.firstCoil = modbusSwapEndian( parser->request15.firstCoil );
	builder->response15.coilCount = modbusSwapEndian( parser->request15.coilCount );

	//Calculate crc
	builder->response15.crc = modbusCRC( builder->frame, frameLength - 2 );

	//Set frame length - frame is ready
	status->response.length = frameLength;
	return 0;
}
