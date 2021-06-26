#include "master_func.h"
#include "master.h"

LIGHTMODBUS_RET_ERROR modbusParseResponse01020304(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength)
{
	// Check if lengths are ok
	if (requestLength != 5 || responseLength < 3)
		return MODBUS_ERROR_LENGTH;

	// Determine data type
	uint8_t bits;
	uint16_t maxCount;
	ModbusDataType datatype;
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
			return MODBUS_ERROR_FUNCTION;
	}

	uint16_t index = modbusRBE(&requestPDU[1]);
	uint16_t count = modbusRBE(&requestPDU[3]);

	// Check count
	if (count == 0 || count > maxCount)
		return MODBUS_ERROR_COUNT;

	// Address range check
	if (UINT16_MAX - count + 1 <= index)
		return MODBUS_ERROR_RANGE;

	// Based on the request, calculate expected data size
	uint8_t expected = (bits == 16) ? (count << 1) : modbusBitsToBytes(count);

	// Check if declared data size matches
	// and if response length is valid
	if (responsePDU[1] != expected || responseLength != expected + 2)
		return MODBUS_ERROR_LENGTH;

	// And finally read the data from the response
	for (uint16_t i = 0; i < count; i++)
	{
		uint16_t value;

		if (bits == 1)
			value = modbusMaskRead(&responsePDU[2], i);
		else
			value = modbusRBE(&responsePDU[2 + (i << 1)]);

		status->dataCallback(status, datatype, function, index + i, value);
	}

	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusParseResponse0506(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength)
{
	// Check if lengths are ok
	if (requestLength != 5 || responseLength != 5)
		return MODBUS_ERROR_LENGTH;

	// The response should be identical to the request
	uint8_t ok = 1;
	for (uint8_t i = 0; ok && i < 5; i++)
		ok = ok && (responsePDU[i] == requestPDU[i]);

	if (!ok) return MODBUS_ERROR_OTHER; //!< \todo is this ok?
	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusParseResponse1516(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength)
{
	// Check if lengths are ok
	if (requestLength < 7 || responseLength != 5)
		return MODBUS_ERROR_LENGTH;

	//! \todo should we handle invalid count/index here?
	
	// Check if index is the same
	if (modbusRBE(&requestPDU[1]) != modbusRBE(&responsePDU[1]))
		return MODBUS_ERROR_INDEX;
		
	// Check if count is the same
	if (modbusRBE(&requestPDU[3]) != modbusRBE(&responsePDU[3]))
		return MODBUS_ERROR_COUNT;

	
	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusParseResponse22(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength)
{
	// Check lengths
	if (requestLength != 7 || responseLength != 7)
		return MODBUS_ERROR_LENGTH;
	
	// The response should be identical to the request
	uint8_t ok = 1;
	for (uint8_t i = 0; ok && i < 7; i++)
		ok = ok && (responsePDU[i] == requestPDU[i]);
	
	if (!ok) return MODBUS_ERROR_OTHER; //!< \todo is this ok?
	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusBuildRequest01020304(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count)
{
	uint16_t maxCount;
	switch (function)
	{
		case 1:
		case 2:
			maxCount = 2000;
			break;

		case 3:
		case 4:
			maxCount = 125;
			break;
		
		default:
			return MODBUS_ERROR_FUNCTION;
	}

	// Check count
	if (count == 0 || count > maxCount)
		return MODBUS_ERROR_COUNT;
	
	// Address range check
	if (UINT16_MAX - count + 1 <= index)
		return MODBUS_ERROR_RANGE;

	ModbusError err = modbusMasterAllocateRequest(status, 5);
	if (err) return err;
	
	status->request.pdu[0] = function;
	modbusWBE(&status->request.pdu[1], index);
	modbusWBE(&status->request.pdu[3], count);

	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusBuildRequest0506(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t value)
{
	if (function != 5 && function != 6)
		return MODBUS_ERROR_FUNCTION;

	// Write coils using proper value
	if (function == 5)
		value = value ? 0xff00 : 0;

	ModbusError err = modbusMasterAllocateRequest(status, 5);
	if (err) return err;
	
	status->request.pdu[0] = function;
	modbusWBE(&status->request.pdu[1], index);
	modbusWBE(&status->request.pdu[3], value);

	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusBuildRequest15(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count,
	const uint8_t *values)
{
	// Check count
	if (count == 0 || count > 1968)
		return MODBUS_ERROR_COUNT;

	// Address range check
	if (UINT16_MAX - count + 1 <= index)
		return MODBUS_ERROR_RANGE;

	uint8_t dataLength = modbusBitsToBytes(count);

	ModbusError err = modbusMasterAllocateRequest(status, 6 + dataLength);
	if (err) return err;

	// Number of full bytes and remaining bits
	uint8_t n = count >> 3;
	uint8_t r = count & 7;

	// Copy n full bytes
	for (uint8_t i = 0; i < n; i++)
		status->request.pdu[6 + i] = values[i];

	// Copy remaining bits
	if (r)
	{
		status->request.pdu[6 + n] = 0;
		for (uint8_t i = 0; i < r; i++)
			modbusMaskWrite(
				&status->request.pdu[6 + n], 
				i,
				modbusMaskRead(values + n, i));
	}

	status->request.pdu[0] = 15;
	modbusWBE(&status->request.pdu[1], index);
	modbusWBE(&status->request.pdu[3], count);
	status->request.pdu[5] = dataLength;
	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusBuildRequest16(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count,
	const uint16_t *values)
{
	// Check count
	if (count == 0 || count > 123)
		return MODBUS_ERROR_COUNT;

	// Addresss range check
	if (UINT16_MAX - count + 1 <= index)
		return MODBUS_ERROR_RANGE;

	uint8_t dataLength = count << 1;

	ModbusError err = modbusMasterAllocateRequest(status, 6 + dataLength);
	if (err) return err;

	// Copy register values
	for (uint8_t i = 0; i < (uint8_t)count; i++)
		modbusWBE(&status->request.pdu[6 + (i << 1)], values[i]);

	status->request.pdu[0] = 16;
	modbusWBE(&status->request.pdu[1], index);
	modbusWBE(&status->request.pdu[3], count);
	status->request.pdu[5] = dataLength;
	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusBuildRequest22(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t andmask,
	uint16_t ormask)
{
	ModbusError err = modbusMasterAllocateRequest(status, 7);
	if (err) return err;
	
	status->request.pdu[0] = 22;
	modbusWBE(&status->request.pdu[1], index);
	modbusWBE(&status->request.pdu[3], andmask);
	modbusWBE(&status->request.pdu[5], ormask);

	return MODBUS_OK;
}