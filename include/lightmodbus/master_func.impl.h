#include "master_func.h"
#include "master.h"

/**
	\brief Parses response to requests 01, 02, 03 and 04
	\param function Response function code
	\param requestPDU pointer to the PDU section of the request frame
	\param requestLength request PDU section length
	\param responsePDU pointer to the PDU section of the response frame
	\param responseLength response PDU section length
	\param responseError Pointer to a variable where error regarding response should be returned (optional)
	\return \ref MODBUS_ERROR_LENGTH if either of the frames has invalid length
	\return \ref MODBUS_ERROR_FUNCTION if `function` is not one of: 01, 02, 03, 04
	\return \ref MODBUS_ERROR_COUNT if the declared register count is invalid
	\return \ref MODBUS_ERROR_RANGE if the declared register range wraps around address space
	\return \ref MODBUS_ERROR_LENGTH if the response length is different from expected one

	\todo Should errors be returned because of an invalid frame built by a slave?
*/
LIGHTMODBUS_RET_ERROR modbusParseResponse01020304(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength,
	ModbusError *responseError)
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

	// Prepare callback args
	ModbusDataCallbackArgs cargs = {
		.function = function,
		.type = datatype
	};

	// And finally read the data from the response
	for (uint16_t i = 0; i < count; i++)
	{
		cargs.id = index + i;
		if (bits == 1)
			cargs.value = modbusMaskRead(&responsePDU[2], i);
		else
			cargs.value = modbusRBE(&responsePDU[2 + (i << 1)]);

		status->dataCallback(status, &cargs);
	}

	return MODBUS_OK;
}

/**
	\brief Parses response to requests 05 and 06
	\param function Response function code
	\param requestPDU pointer to the PDU section of the request frame
	\param requestLength request PDU section length
	\param responsePDU pointer to the PDU section of the response frame
	\param responseLength response PDU section length
	\param responseError Pointer to a variable where error regarding response should be returned (optional)
	\return \ref MODBUS_ERROR_LENGTH if either of the frames has invalid length
	\return \ref MODBUS_ERROR_OTHER if the request is different from the response
*/
LIGHTMODBUS_RET_ERROR modbusParseResponse0506(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength,
	ModbusError *responseError)
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

/**
	\brief Parses response to requests 15 and 16
	\param function Response function code
	\param requestPDU pointer to the PDU section of the request frame
	\param requestLength request PDU section length
	\param responsePDU pointer to the PDU section of the response frame
	\param responseLength response PDU section length
	\param responseError Pointer to a variable where error regarding response should be returned (optional)
	\return \ref MODBUS_ERROR_LENGTH if either of the frames has invalid length
	\return \ref MODBUS_ERROR_INDEX if the declared register index differs between the request and the response
	\return \ref MODBUS_ERROR_COUNT if the declared register count differs between the request and the response
*/
LIGHTMODBUS_RET_ERROR modbusParseResponse1516(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength,
	ModbusError *responseError)
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

/**
	\brief Parses response to request 22
	\param function Response function code
	\param requestPDU pointer to the PDU section of the request frame
	\param requestLength request PDU section length
	\param responsePDU pointer to the PDU section of the response frame
	\param responseLength response PDU section length
	\param responseError Pointer to a variable where error regarding response should be returned (optional)
	\return \ref MODBUS_ERROR_LENGTH if either of the frames has invalid length
	\return \ref MODBUS_ERROR_OTHER if the request is different from the response
*/
LIGHTMODBUS_RET_ERROR modbusParseResponse22(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength,
	ModbusError *responseError)
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

/**
	\brief Read mutiple coils/discrete inputs/holding registers/input registers
	\param function 1 to read coils, 2 to read discrete inputs, 3 to read holding registers, 4 to read input registers
	\param index Index of the register to be read
	\param count Number of registers to be read
	\param value New value for the register/coil
	\returns \ref MODBUS_ERROR_FUNCTION if `function` is not one of: 01, 02, 03, 04
	\returns \ref MODBUS_ERROR_COUNT if the register count is invalid
	\returns \ref MODBUS_ERROR_RANGE if `count` + `index` wraps around the address range
	\returns \ref MODBUS_ERROR_ALLOC on memory allocation error
*/
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

/**
	\brief Read multiple coils
	\note A wrapper for \ref modbusBuildRequest01020304()
	\see modbusBuildRequest01020304()
*/
LIGHTMODBUS_RET_ERROR modbusBuildRequest01(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count)
{
	return modbusBuildRequest01020304(status, 1, index, count);
}

/**
	\brief Read multiple discrete inputs
	\note A wrapper for \ref modbusBuildRequest01020304()
	\see modbusBuildRequest01020304()
*/
LIGHTMODBUS_RET_ERROR modbusBuildRequest02(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count)
{
	return modbusBuildRequest01020304(status, 2, index, count);
}

/**
	\brief Read multiple holding registers
	\note A wrapper for \ref modbusBuildRequest01020304()
	\see modbusBuildRequest01020304()
*/
LIGHTMODBUS_RET_ERROR modbusBuildRequest03(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count)
{
	return modbusBuildRequest01020304(status, 3, index, count);
}

/**
	\brief Read multiple input registers
	\note A wrapper for \ref modbusBuildRequest01020304()
	\see modbusBuildRequest01020304()
*/
LIGHTMODBUS_RET_ERROR modbusBuildRequest04(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count)
{
	return modbusBuildRequest01020304(status, 4, index, count);
}

/**
	\brief Write single coil/holding register
	\param function 5 to write a coil, 6 to write a holding register
	\param index Index of the register/coil to be written
	\param value New value for the register/coil
	\returns \ref MODBUS_ERROR_FUNCTION if `function` is not not 5 nor 6
	\returns \ref MODBUS_ERROR_ALLOC on memory allocation error
*/
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

/**
	\brief Write single coil
	\note A wrapper for \ref modbusBuildRequest0506()
	\see modbusBuildRequest0506()
*/
LIGHTMODBUS_RET_ERROR modbusBuildRequest05(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count)
{
	return modbusBuildRequest0506(status, 5, index, count);
}

/**
	\brief Write single holding register
	\note A wrapper for \ref modbusBuildRequest0506()
	\see modbusBuildRequest0506()
*/
LIGHTMODBUS_RET_ERROR modbusBuildRequest06(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count)
{
	return modbusBuildRequest0506(status, 6, index, count);
}

/**
	\brief Write multiple coils
	\param function Ignored
	\param index Index of the first coil to be written
	\param count Number of coils to be written
	\param values Pointer to array containing `count` coil values (each bit corresponds to one coil value)
	\returns \ref MODBUS_ERROR_COUNT if the coil count is invalid (zero or greater than 1968)
	\returns \ref MODBUS_ERROR_RANGE if `count` + `index` wraps around the address range
	\returns \ref MODBUS_ERROR_ALLOC on memory allocation error
*/
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

/**
	\brief Write multiple holding registers
	\param function Ignored
	\param index Index of the first register to be written
	\param count Number of registers to be written
	\param values Pointer to array containing `count` register values
	\returns \ref MODBUS_ERROR_COUNT if the register count is invalid (zero or greater than 123)
	\returns \ref MODBUS_ERROR_RANGE if `count` + `index` wraps around the address range
	\returns \ref MODBUS_ERROR_ALLOC on memory allocation error
*/
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

/**
	\brief Mask write register request
	\param function Ignored
	\param index Register ID
	\param andmax AND mask
	\param ormask OR mask
	\returns \ref MODBUS_ERROR_ALLOC on memory allocation error
	\note Must be called between modbusBeginXXXRequest() and modbusEndXXXRequest()
*/
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