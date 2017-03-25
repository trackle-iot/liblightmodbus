/*
	liblightmodbus - a lightweight, multiplatform Modbus library
	Copyright (C) 2016 Jacek Wieczorek <mrjjot@gmail.com>

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
#include <lightmodbus/slave.h>
#include <lightmodbus/parser.h>
#include <lightmodbus/slave/sregs.h>
#include <lightmodbus/slave/scoils.h>

uint8_t modbusBuildException( ModbusSlave *status, uint8_t function, uint8_t code )
{
	//Generates modbus exception frame in allocated memory frame
	//Returns generated frame length

	//Check if given pointer is valid
	if ( status == NULL || code == 0 ) return MODBUS_ERROR_OTHER;

	//Reallocate frame memory
	status->response.frame = (uint8_t *) calloc( 5, sizeof( uint8_t ) );
	if ( status->response.frame == NULL ) return MODBUS_ERROR_ALLOC;
	union ModbusParser *exception = (union ModbusParser *) status->response.frame;

	//Setup exception frame
	exception->exception.address = status->address;
	exception->exception.function = ( 1 << 7 ) | function;
	exception->exception.code = code;
	exception->exception.crc = modbusCRC( exception->frame, 3 );

	//Set frame length - frame is ready
	status->response.length = 5;

	//So, user should rather know, that master slave had to throw exception, right?
	//That's the reason exception should be thrown - just like that, an information
	return MODBUS_ERROR_EXCEPTION;
}

uint8_t modbusParseRequest( ModbusSlave *status )
{
	//Parse and interpret given modbus frame on slave-side
	uint8_t err = 0;

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Reset response frame status
	status->response.length = 0;

	//If there is memory allocated for response frame - free it
	free( status->response.frame );
	status->response.frame = NULL;

	//If user tries to parse an empty frame return error
	//That enables us to ommit the check in each parsing function
	if ( status->request.length < 4u || status->request.frame == NULL ) return MODBUS_ERROR_OTHER;

	//Check CRC
	if ( *( (uint16_t*)( status->request.frame + status->request.length - 2 ) )\
		!= modbusCRC( status->request.frame, status->request.length - 2 ) )
			return MODBUS_ERROR_CRC;


	union ModbusParser *parser = (union ModbusParser *) status->request.frame;

	//If frame is not broadcasted and address doesn't match skip parsing
	if ( parser->base.address != status->address && parser->base.address != 0 )
		return MODBUS_ERROR_OK;

	switch ( parser->base.function )
	{
		case 1: //Read multiple coils
		case 2: //Read multiple discrete inputs
			if ( LIGHTMODBUS_SLAVE_COILS ) err = modbusParseRequest0102( status, parser );
			else err = MODBUS_ERROR_PARSE;
			break;

		case 3: //Read multiple holding registers
		case 4: //Read multiple input registers
			if ( LIGHTMODBUS_SLAVE_REGISTERS ) err = modbusParseRequest0304( status, parser );
			else err = MODBUS_ERROR_PARSE;
			break;

		case 5: //Write single coil
			if ( LIGHTMODBUS_SLAVE_COILS ) err = modbusParseRequest05( status, parser );
			else err = MODBUS_ERROR_PARSE;
			break;

		case 6: //Write single holding reg
			if ( LIGHTMODBUS_SLAVE_REGISTERS ) err = modbusParseRequest06( status, parser );
			else err = MODBUS_ERROR_PARSE;
			break;

		case 15: //Write multiple coils
			if ( LIGHTMODBUS_SLAVE_COILS ) err = modbusParseRequest15( status, parser );
			else err = MODBUS_ERROR_PARSE;
			break;

		case 16: //Write multiple holding registers
			if ( LIGHTMODBUS_SLAVE_REGISTERS ) err = modbusParseRequest16( status, parser );
			else err = MODBUS_ERROR_PARSE;
			break;

		case 22: //Mask write single register
			if ( LIGHTMODBUS_SLAVE_REGISTERS ) err = modbusParseRequest22( status, parser );
			else err = MODBUS_ERROR_PARSE;
			break;

		default:
			err = MODBUS_ERROR_PARSE;
			break;
	}

	if ( err == MODBUS_ERROR_PARSE )
		if ( parser->base.address != 0 ) err = modbusBuildException( status, parser->base.function, MODBUS_EXCEP_ILLEGAL_FUNC );

	return err;
}

uint8_t modbusSlaveInit( ModbusSlave *status )
{
	//Very basic init of slave side
	//User has to modify pointers etc. himself

	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Reset response frame status
	status->request.length = 0;
	status->request.frame = NULL;
	status->response.length = 0;
	status->response.frame = NULL;

	if ( status->address == 0 )
	{
		status->address = 1;
		return MODBUS_ERROR_OTHER;
	}

	//Some safety checks
	if ( status->registerCount == 0 || status->registers == NULL )
	{
		status->registerCount = 0;
		status->registers = NULL;
	}

	if ( status->coilCount == 0 || status->coils == NULL )
	{
		status->coilCount = 0;
		status->coils = NULL;
	}

	if ( status->discreteInputCount == 0 || status->discreteInputs == NULL )
	{
		status->discreteInputCount = 0;
		status->discreteInputs = NULL;
	}

	if ( status->inputRegisterCount == 0 || status->inputRegisters == NULL )
	{
		status->inputRegisterCount = 0;
		status->inputRegisters = NULL;
	}

	return MODBUS_ERROR_OK;
}

uint8_t modbusSlaveEnd( ModbusSlave *status )
{
	//Check if given pointer is valid
	if ( status == NULL ) return MODBUS_ERROR_OTHER;

	//Free memory
	free( status->response.frame );

	return MODBUS_ERROR_OK;
}
