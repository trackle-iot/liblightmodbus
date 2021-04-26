#include "slave_func.h"
#include "slave.h"

LIGHTMODBUS_RET_ERROR modbusParseRequest0304(
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
	if (size != 4)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	ModbusDataType datatype = function == 3 ? MODBUS_HOLDING_REGISTER : MODBUS_INPUT_REGISTER;
	uint16_t index = modbusRBE(&data[0]);
	uint16_t count = modbusRBE(&data[2]);

	if (count == 0 || count > 125 || UINT16_MAX - count - 1 < index)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

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

	ModbusError err = modbusSlaveAllocateResponse(status, 2 + (count << 1));
	if (err) return err;

	status->response.pdu[0] = function;
	status->response.pdu[1] = count << 1;
	
	for (uint16_t i = 0; i < count; i++)
	{
		uint16_t value;
		(void) status->registerCallback(status, datatype, MODBUS_REGQ_R, function, index + i, 0, &value);
		modbusWBE(&status->response.pdu[2 + (i << 1)], value);
	}

	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusParseRequest06(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Check length	
	if (size != 4)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get register ID and value
	uint16_t index = modbusRBE(&data[0]);
	uint16_t value = modbusRBE(&data[2]);

	// Check write access
	uint16_t res = 0;
	ModbusError fail = status->registerCallback(status, MODBUS_HOLDING_REGISTER, MODBUS_REGQ_W_CHECK, function, index, value, &res);
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (res) return modbusBuildException(status, address, function, (ModbusExceptionCode)res);

	// Write register
	status->registerCallback(status, MODBUS_HOLDING_REGISTER, MODBUS_REGQ_W, function, index, value, &res);

	// ---- RESPONSE ----

	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	ModbusError err;
	if ((err = modbusSlaveAllocateResponse(status, 4)))
		return err;

	modbusWBE(&status->response.pdu[0], index);
	modbusWBE(&status->response.pdu[2], value);
	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusParseRequest16(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Check length
	if (size < 7)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get first index and register count
	uint8_t declaredLength = data[0];
	uint16_t index = modbusRBE(&data[1]);
	uint16_t count = modbusRBE(&data[3]);

	// Check if declared length is correct
	if (declaredLength == 0 || declaredLength != size - 5)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Check if index and count are valid
	if (count == 0
		|| count != (declaredLength >> 1)
		|| count > 123
		|| UINT16_MAX - count - 1 < index)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Check write access
	uint8_t fail = 0;
	ModbusExceptionCode ex = MODBUS_EXCEP_NONE;
	for (uint16_t i = 0; !fail && !ex && i < count; i++)
	{
		uint16_t res = MODBUS_OK;
		uint16_t value = modbusRBE(&data[5 + (i << 2)]);
		fail = status->registerCallback(status, MODBUS_HOLDING_REGISTER, MODBUS_REGQ_W_CHECK, function, index + i, value, &res);
		if (res != MODBUS_OK)
			ex = (ModbusExceptionCode)res;
	}

	// Return exceptions (if any)
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (ex) return modbusBuildException(status, address, function, ex);

	// Write registers
	for (uint16_t i = 0; i < count; i++)
	{
		uint16_t dummy;
		uint16_t value = modbusRBE(&data[5 + (i << 2)]);
		(void) status->registerCallback(status, MODBUS_HOLDING_REGISTER, MODBUS_REGQ_W, function, index + i, value, &dummy);
	}

	// ---- RESPONSE ----

	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	ModbusError err;
	if ((err = modbusSlaveAllocateResponse(status, 4)))
		return err;

	modbusWBE(&status->response.pdu[0], index);
	modbusWBE(&status->response.pdu[2], count);
	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusParseRequest22(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Check length	
	if (size != 6)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	// Get index and masks
	uint16_t index   = modbusRBE(&data[0]);
	uint16_t andmask = modbusRBE(&data[2]);
	uint16_t ormask  = modbusRBE(&data[4]);

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
	if ((err = modbusSlaveAllocateResponse(status, 6)))
		return err;

	modbusWBE(&status->response.pdu[0], index);
	modbusWBE(&status->response.pdu[2], andmask);
	modbusWBE(&status->response.pdu[2], ormask);
	return MODBUS_OK;
}