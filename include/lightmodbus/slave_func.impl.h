#include "slave_func.h"
#include "slave.h"

/**
	\brief Handles requests 01, 02, 03 and 04 (Read Multiple XX) and generates response.
	\param address address of the slave
	\param function function code
	\param requestPDU pointer to the PDU section of the request
	\param requestLength length of the PDU section in bytes
	\returns MODBUS_GENERAL_ERROR(ALLOC) on memory allocation error
	\returns MODBUS_NO_ERROR() on success
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest01020304(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength)
{
	// Do not respond if the request was broadcasted
	if (address == 0)
	{
		if (modbusSlaveAllocateResponse(status, 0))
			return MODBUS_GENERAL_ERROR(ALLOC);
	}

	// Check frame length
	if (requestLength != 5)
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

	uint16_t index = modbusRBE(&requestPDU[1]);
	uint16_t count = modbusRBE(&requestPDU[3]);

	// Check count
	if (count == 0 || count > maxCount)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Addresss range check
	if (UINT16_MAX - count - 1 < index)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_ADDRESS);

	// Prepare callback args
	ModbusRegisterCallbackArgs cargs = {
		.type = datatype,
		.query = MODBUS_REGQ_R_CHECK,
		.value = 0,
		.function = function,
	};

	// Check if all registers can be read
	ModbusError fail = MODBUS_OK;
	ModbusExceptionCode ex = MODBUS_EXCEP_NONE;
	for (uint16_t i = 0; !fail && !ex && i < count; i++)
	{
		uint16_t res = MODBUS_OK;
		cargs.index = index + i;
		fail = status->registerCallback(status, &cargs, &res);
		if (res != MODBUS_OK)
			ex = (ModbusExceptionCode)res;
	}

	// ---- RESPONSE ----

	// Return exceptions (if any)
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (ex) return modbusBuildException(status, address, function, ex);

	uint8_t dataLength = (bits == 1 ? modbusBitsToBytes(count) : (count << 1));
	if (modbusSlaveAllocateResponse(status, 2 + dataLength))
		return MODBUS_GENERAL_ERROR(ALLOC);

	status->response.pdu[0] = function;
	status->response.pdu[1] = dataLength;
	
	// Clear with zeros, if we're writing bits
	for (uint8_t i = 0; i < dataLength; i++)
		status->response.pdu[2 + i] = 0;

	cargs.query = MODBUS_REGQ_R;
	for (uint16_t i = 0; i < count; i++)
	{
		uint16_t value;
		cargs.index = index + i;
		(void) status->registerCallback(status, &cargs, &value);
		
		if (bits == 1)
			modbusMaskWrite(&status->response.pdu[2], i, value != 0);
		else
			modbusWBE(&status->response.pdu[2 + (i << 1)], value);
	}

	return MODBUS_NO_ERROR();
}

/**
	\brief Handles requests 05 and 06 (Write Single XX) and generates response.
	\param address address of the slave
	\param function function code
	\param requestPDU pointer to the PDU section of the request
	\param requestLength length of the PDU section in bytes
	\returns MODBUS_GENERAL_ERROR(ALLOC) on memory allocation error
	\returns MODBUS_NO_ERROR() on success
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest0506(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength)
{
	// Check frame length
	if (requestLength != 5)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get register index and value
	ModbusDataType datatype = function == 5 ? MODBUS_COIL : MODBUS_HOLDING_REGISTER;
	uint16_t index = modbusRBE(&requestPDU[1]);
	uint16_t value = modbusRBE(&requestPDU[3]);

	// For coils - check if coil value is valid
	if (datatype == MODBUS_COIL && value != 0x0000 && value != 0xFF00)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Prepare callback args
	ModbusRegisterCallbackArgs cargs = {
		.type = datatype,
		.query = MODBUS_REGQ_W_CHECK,
		.index = index,
		.value = (uint16_t)((datatype == MODBUS_COIL) ? (value != 0) : value),
		.function = function,
	};

	// Check if the register/coil can be written
	uint16_t res = 0;
	ModbusError fail = status->registerCallback(status, &cargs, &res);
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (res) return modbusBuildException(status, address, function, (ModbusExceptionCode)res);

	// Write coil/register
	// Keep in mind that 0xff00 is 0 when cast to uint8_t
	cargs.query = MODBUS_REGQ_W;
	status->registerCallback(status, &cargs, &res);

	// ---- RESPONSE ----

	// Do not respond if the request was broadcasted
	if (address == 0)
	{
		if (modbusSlaveAllocateResponse(status, 0))
			return MODBUS_GENERAL_ERROR(ALLOC);
	}

	if ( modbusSlaveAllocateResponse(status, 5))
		return MODBUS_GENERAL_ERROR(ALLOC);

	status->response.pdu[0] = function;
	modbusWBE(&status->response.pdu[1], index);
	modbusWBE(&status->response.pdu[3], value);

	return MODBUS_NO_ERROR();
}

/**
	\brief Handles requests 15 and 16 (Write Multiple XX) and generates response.
	\param address address of the slave
	\param function function code
	\param requestPDU pointer to the PDU section of the request
	\param requestLength length of the PDU section in bytes
	\returns MODBUS_GENERAL_ERROR(ALLOC) on memory allocation error
	\returns MODBUS_NO_ERROR() on success
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest1516(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength)
{
	// Check length
	if (requestLength < 6)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get first index and register count
	ModbusDataType datatype = function == 15 ? MODBUS_COIL : MODBUS_HOLDING_REGISTER;
	uint16_t maxCount = datatype == MODBUS_COIL ? 1968 : 123;
	uint16_t index = modbusRBE(&requestPDU[1]);
	uint16_t count = modbusRBE(&requestPDU[3]);
	uint8_t declaredLength = requestPDU[5];

	// Check if the declared length is correct
	if (declaredLength == 0 || declaredLength != requestLength - 6)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Check count
	if (count == 0
		|| count > maxCount
		|| declaredLength != (datatype == MODBUS_COIL ? modbusBitsToBytes(count) : (count << 1)))
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Addresss range check
	if (UINT16_MAX - count - 1 < index)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_ADDRESS);

	// Prepare callback args
	ModbusRegisterCallbackArgs cargs = {
		.type = datatype,
		.query = MODBUS_REGQ_W_CHECK,
		.function = function,
	};

	// Check write access
	ModbusError fail = MODBUS_OK;
	ModbusExceptionCode ex = MODBUS_EXCEP_NONE;
	for (uint16_t i = 0; !fail && !ex && i < count; i++)
	{
		uint16_t res = MODBUS_OK;
		uint16_t value = datatype == MODBUS_COIL ? modbusMaskRead(&requestPDU[6], i) : modbusRBE(&requestPDU[6 + (i << 1)]);
		cargs.index = index + i;
		cargs.value = value;
		fail = status->registerCallback(status, &cargs, &res);
		if (res != MODBUS_OK)
			ex = (ModbusExceptionCode)res;
	}
	
	// Return exceptions (if any)
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (ex) return modbusBuildException(status, address, function, ex);

	// Write coils
	cargs.query = MODBUS_REGQ_W;
	for (uint16_t i = 0; i < count; i++)
	{
		uint16_t dummy;
		uint16_t value = datatype == MODBUS_COIL ? modbusMaskRead(&requestPDU[6], i) : modbusRBE(&requestPDU[6 + (i << 1)]);
		cargs.index = index + i;
		cargs.value = value;
		(void) status->registerCallback(status, &cargs, &dummy);
	}

	// ---- RESPONSE ----

	// Do not respond if the request was broadcasted
	if (address == 0)
	{
		if (modbusSlaveAllocateResponse(status, 0))
			return MODBUS_GENERAL_ERROR(ALLOC);
	}

	if (modbusSlaveAllocateResponse(status, 5))
		return MODBUS_GENERAL_ERROR(ALLOC);

	status->response.pdu[0] = function;
	modbusWBE(&status->response.pdu[1], index);
	modbusWBE(&status->response.pdu[3], count);
	
	return MODBUS_NO_ERROR();
}

/**
	\brief Handles request 22 (Mask Write Register) and generates response.
	\param address address of the slave
	\param function function code
	\param requestPDU pointer to the PDU section of the request
	\param requestLength length of the PDU section in bytes
	\returns MODBUS_GENERAL_ERROR(ALLOC) on memory allocation error
	\returns MODBUS_NO_ERROR() on success
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest22(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength)
{
	// Check length	
	if (requestLength != 7)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get index and masks
	uint16_t index   = modbusRBE(&requestPDU[1]);
	uint16_t andmask = modbusRBE(&requestPDU[3]);
	uint16_t ormask  = modbusRBE(&requestPDU[5]);

	// Prepare callback args
	ModbusRegisterCallbackArgs cargs = {
		.type = MODBUS_HOLDING_REGISTER,
		.query = MODBUS_REGQ_R_CHECK,
		.index = index,
		.value = 0,
		.function = function,
	};

	// Check read access
	uint16_t res = 0;
	ModbusError fail = MODBUS_OK;
	cargs.query = MODBUS_REGQ_R_CHECK;
	fail = status->registerCallback(status, &cargs, &res);
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (res) return modbusBuildException(status, address, function, (ModbusExceptionCode)res);

	// Read the register
	uint16_t value;
	cargs.query = MODBUS_REGQ_R;
	(void) status->registerCallback(status, &cargs, &value);

	// Compute new value for the register
	value = (value & andmask) | (ormask & ~andmask);

	// Check write access
	cargs.query = MODBUS_REGQ_W_CHECK;
	cargs.value = value;
	fail = status->registerCallback(status, &cargs, &res);
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (res) return modbusBuildException(status, address, function, (ModbusExceptionCode)res);

	// Write the register
	cargs.query = MODBUS_REGQ_W;
	(void) status->registerCallback(status, &cargs, &res);
	
	// ---- RESPONSE ----

	// Do not respond if the request was broadcasted
	if (address == 0)
	{
		if (modbusSlaveAllocateResponse(status, 0))
			return MODBUS_GENERAL_ERROR(ALLOC);
	}

	if (modbusSlaveAllocateResponse(status, 7))
		return MODBUS_GENERAL_ERROR(ALLOC);

	status->response.pdu[0] = function;
	modbusWBE(&status->response.pdu[1], index);
	modbusWBE(&status->response.pdu[3], andmask);
	modbusWBE(&status->response.pdu[5], ormask);
	
	return MODBUS_NO_ERROR();
}