#include "slave_func.h"
#include "slave.h"

/**
	\brief Handles requests 01, 02, 03 and 04 (Read Multiple XX)
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest01020304(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	// Check frame length
	if (size != 5)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	ModbusDataType datatype;
	uint16_t maxCount;
	uint8_t bits;
	switch (function)
	{
		case 1:
			datatype = MODBUS_COIL;
			maxCount = 2000;
			bits = 1;
			break;

		case 2:
			datatype = MODBUS_DISCRETE_INPUT;
			maxCount = 2000;
			bits = 1;
			break;

		case 3:
			datatype = MODBUS_HOLDING_REGISTER;
			maxCount = 125;
			bits = 16;
			break;

		case 4:
			datatype = MODBUS_INPUT_REGISTER;
			maxCount = 125;
			bits = 16;
			break;
		
		default:
			return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_FUNCTION);
			break;
	}

	uint16_t index = modbusRBE(&data[1]);
	uint16_t count = modbusRBE(&data[3]);

	// Check count
	if (count == 0 || count > maxCount)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Addresss range check
	if (UINT16_MAX - count - 1 < index)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_ADDRESS);

	// Check if all registers can be read
	uint8_t fail = 0;
	ModbusExceptionCode ex = MODBUS_EXCEP_NONE;
	for (uint16_t i = 0; !fail && !ex && i < count; i++)
	{
		uint16_t res = MODBUS_OK;
		fail = status->registerCallback(status, datatype, MODBUS_REGQ_R_CHECK, function, index + i, 0, &res);
		if (res != MODBUS_OK)
			ex = (ModbusExceptionCode)res;
	}

	// ---- RESPONSE ----

	// Return exceptions (if any)
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (ex) return modbusBuildException(status, address, function, ex);

	uint8_t dataLength = (bits == 1 ? modbusBitsToBytes(count) : (count << 1));
	ModbusError err = modbusSlaveAllocateResponse(status, 2 + dataLength);
	if (err) return err;

	status->response.pdu[0] = function;
	status->response.pdu[1] = dataLength;
	
	// Clear with zeros, if we're writing bits
	for (uint8_t i = 0; i < dataLength; i++)
		status->response.pdu[2 + i] = 0;

	for (uint16_t i = 0; i < count; i++)
	{
		uint16_t value;
		(void) status->registerCallback(status, datatype, MODBUS_REGQ_R, function, index + i, 0, &value);
		
		if (bits == 1)
			modbusMaskWrite(&status->response.pdu[2], i, value != 0);
		else
			modbusWBE(&status->response.pdu[2 + (i << 1)], value);
	}

	return MODBUS_OK;
}

/**
	\brief Handles requests 05 and 06 (Write Single XX)
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest0506(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Check frame length
	if (size != 5)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get register ID and value
	ModbusDataType datatype = function == 5 ? MODBUS_COIL : MODBUS_HOLDING_REGISTER;
	uint16_t index = modbusRBE(&data[1]);
	uint16_t value = modbusRBE(&data[3]);

	// For coils - check if coil value is valid
	if (datatype == MODBUS_COIL && value != 0x0000 && value != 0xFF00)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Check if the register/coil can be written
	uint16_t res = 0;
	ModbusError fail = status->registerCallback(status, datatype, MODBUS_REGQ_W_CHECK, function, index, value, &res);
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (res) return modbusBuildException(status, address, function, (ModbusExceptionCode)res);

	// Write coil/register
	// Keep in mind that 0xff00 is 0 when cast to uint8_t
	status->registerCallback(
		status,
		datatype,
		MODBUS_REGQ_W,
		function,
		index,
		datatype == MODBUS_COIL ? (value != 0) : value, 
		&res);

	// ---- RESPONSE ----

	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	ModbusError err;
	if ((err = modbusSlaveAllocateResponse(status, 5)))
		return err;

	status->response.pdu[0] = function;
	modbusWBE(&status->response.pdu[1], index);
	modbusWBE(&status->response.pdu[3], value);
	return MODBUS_OK;
}

/**
	\brief Handles requests 15 and 16 (Write Multiple XX)
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest1516(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Check length
	if (size < 6)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get first index and register count
	ModbusDataType datatype = function == 15 ? MODBUS_COIL : MODBUS_HOLDING_REGISTER;
	uint16_t maxCount = datatype == MODBUS_COIL ? 1968 : 123;
	uint16_t index = modbusRBE(&data[1]);
	uint16_t count = modbusRBE(&data[3]);
	uint8_t declaredLength = data[5];

	// Check if the declared length is correct
	if (declaredLength == 0 || declaredLength != size - 6)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Check count
	if (count == 0
		|| count > maxCount
		|| declaredLength != (datatype == MODBUS_COIL ? modbusBitsToBytes(count) : (count << 1)))
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Addresss range check
	if (UINT16_MAX - count - 1 < index)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_ADDRESS);

	// Check write access
	uint8_t fail = 0;
	ModbusExceptionCode ex = MODBUS_EXCEP_NONE;
	for (uint16_t i = 0; !fail && !ex && i < count; i++)
	{
		uint16_t res = MODBUS_OK;
		uint16_t value = datatype == MODBUS_COIL ? modbusMaskRead(&data[6], i) : modbusRBE(&data[6 + (i << 1)]);
		fail = status->registerCallback(status, datatype, MODBUS_REGQ_W_CHECK, function, index + i, value, &res);
		if (res != MODBUS_OK)
			ex = (ModbusExceptionCode)res;
	}
	
	// Return exceptions (if any)
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (ex) return modbusBuildException(status, address, function, ex);

	// Write coils
	for (uint16_t i = 0; i < count; i++)
	{
		uint16_t dummy;
		uint16_t value = datatype == MODBUS_COIL ? modbusMaskRead(&data[6], i) : modbusRBE(&data[6 + (i << 1)]);
		(void) status->registerCallback(status, datatype, MODBUS_REGQ_W, function, index + i, value, &dummy);
	}

	// ---- RESPONSE ----

	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	ModbusError err;
	if ((err = modbusSlaveAllocateResponse(status, 5)))
		return err;

	status->response.pdu[0] = function;
	modbusWBE(&status->response.pdu[1], index);
	modbusWBE(&status->response.pdu[3], count);
	return MODBUS_OK;
}

/**
	\brief Handles request 22 (Mask Write Register)
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest22(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Check length	
	if (size != 7)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get index and masks
	uint16_t index   = modbusRBE(&data[1]);
	uint16_t andmask = modbusRBE(&data[3]);
	uint16_t ormask  = modbusRBE(&data[5]);

	// Check read access
	uint16_t res = 0;
	ModbusError fail = MODBUS_OK;
	fail = status->registerCallback(status, MODBUS_HOLDING_REGISTER, MODBUS_REGQ_R_CHECK, function, index, 0, &res);
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (res) return modbusBuildException(status, address, function, (ModbusExceptionCode)res);

	// Read the register
	uint16_t value;
	(void) status->registerCallback(status, MODBUS_HOLDING_REGISTER, MODBUS_REGQ_R, function, index, 0, &value);

	// Compute new value for the register
	value = (value & andmask) | (ormask & ~andmask);

	// Check write access
	fail = status->registerCallback(status, MODBUS_HOLDING_REGISTER, MODBUS_REGQ_W_CHECK, function, index, value, &res);
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (res) return modbusBuildException(status, address, function, (ModbusExceptionCode)res);

	// Write the register
	(void) status->registerCallback(status, MODBUS_HOLDING_REGISTER, MODBUS_REGQ_W, function, index, value, &res);
	
	// ---- RESPONSE ----

	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	ModbusError err;
	if ((err = modbusSlaveAllocateResponse(status, 7)))
		return err;

	status->response.pdu[0] = function;
	modbusWBE(&status->response.pdu[1], index);
	modbusWBE(&status->response.pdu[3], andmask);
	modbusWBE(&status->response.pdu[5], ormask);
	return MODBUS_OK;
}