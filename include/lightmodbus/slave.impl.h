#include "slave.h"

ModbusFunctionHandler modbusSlaveDefaultFunctions[MODBUS_SLAVE_DEFAULT_FUNCTION_COUNT] =
{
	{3, modbusParseRequest0304},
	{4, modbusParseRequest0304},
};

ModbusError modbusSlaveDefaultAllocator(ModbusSlave *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose, void *ctx)
{
	return modbusDefaultAllocator(ptr, size, purpose, ctx);
}


ModbusError modbusSlaveInit(
	ModbusSlave *status,
	uint8_t address,
	ModbusSlaveAllocator allocator,
	ModbusRegisterCallback registerCallback)
{
	status->address = address;
	status->response = NULL;
	status->responseLength = 0;
	status->functions = modbusSlaveDefaultFunctions;
	status->functionCount = MODBUS_SLAVE_DEFAULT_FUNCTION_COUNT;
	status->lastException = MODBUS_EXCEP_NONE;
	status->allocator = allocator;
	status->registerCallback = registerCallback;
	status->exceptionCallback = NULL;
	status->context = NULL;

	return MODBUS_OK;
}

void modbusSlaveDestroy(ModbusSlave *status)
{
	status->allocator(status, &status->response, 0, MODBUS_RESPONSE_BUFFER, status->context);
}

ModbusError modbusSlaveAllocateResponse(ModbusSlave *status, uint16_t size)
{
	ModbusError err = status->allocator(status, &status->response, size, MODBUS_RESPONSE_BUFFER, status->context);
	if (err == MODBUS_ERROR_ALLOC)
		status->responseLength = 0;
	else
		status->responseLength = size;
	return err;
}

void modbusSlaveWriteResponseCRC(ModbusSlave *status)
{
	if (status->responseLength > 2)
		modbusWLE(
			&status->response[status->responseLength - 2],
			modbusCRC(status->response, status->responseLength - 2)
			);
}

ModbusError modbusParseRequest0304(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	ModbusDataType datatype = function == 3 ? MODBUS_HOLDING_REGISTER : MODBUS_INPUT_REGISTER;
	uint16_t index = modbusRBE(&data[0]);
	uint16_t count = modbusRBE(&data[2]);

	if (count == 0 || count > 125)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	uint8_t ok = 1;
	for (uint16_t i = index; ok && i < index + count; i++)
		ok = ok && status->registerCallback(status, datatype, MODBUS_REGQ_R_CHECK, function, i, 0);

	if (!ok)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);

	// RESPONSE
	ModbusError err = modbusSlaveAllocateResponse(status, 5 + (count << 1));
	if (err)
		return err;

	status->response[0] = status->address;
	status->response[1] = function;
	status->response[2] = count << 1;
	
	for (uint16_t i = 0; i < count; i++)
		modbusWBE(
			&status->response[3 + (i << 1)],
			status->registerCallback(status, datatype, MODBUS_REGQ_R, function, index + i, 0)
		);

	modbusSlaveWriteResponseCRC(status);
	return MODBUS_OK;
}

ModbusError modbusBuildException(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	ModbusExceptionCode code)
{
	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	ModbusError err;
	if ((err = modbusSlaveAllocateResponse(status, 5)))
		return err;

	status->response[0] = status->address;
	status->response[1] = function | 0x80;
	status->response[2] = code;
	modbusSlaveWriteResponseCRC(status);
	status->lastException = code;

	return MODBUS_OK;
}

ModbusError modbusParseRequestPDU(ModbusSlave *status, uint8_t address, const uint8_t *data, uint8_t length)
{
	uint8_t function = data[0];

	// Look for matching function
	for (uint16_t i = 0; i < status->functionCount; i++)
		if (function == status->functions[i].id)
			return status->functions[i].ptr(status, address, function, &data[1], length - 1);

	// No match found
	return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_FUNCTION);
}

ModbusError modbusParseRequestRTU(ModbusSlave *status, const uint8_t *data, uint16_t length)
{
	// Check length
	if (length < 4 || length > 256)
		return MODBUS_ERROR_LENGTH;

	// Check if the message is meant for us
	uint8_t address = data[0];
	if (address != status->address || address == 0)
		return MODBUS_OK;

	// Check CRC
	if (modbusCRC(data, length - 2) != modbusRLE(data + length - 2))
		return MODBUS_OK;

	
	return modbusParseRequestPDU(status, address, data + 1, length - 3);
}