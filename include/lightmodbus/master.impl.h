#include "master.h"
#include "master_func.h"

ModbusMasterFunctionHandler modbusMasterDefaultFunctions[] =
{
#ifdef LIGHTMODBUS_F01M
	{1, modbusParseResponse01020304},
#endif

#ifdef LIGHTMODBUS_F02M
	{2, modbusParseResponse01020304},
#endif

#ifdef LIGHTMODBUS_F03M
	{3, modbusParseResponse01020304},
#endif

#ifdef LIGHTMODBUS_F04M
	{4, modbusParseResponse01020304},
#endif

#ifdef LIGHTMODBUS_F05M
	{5, modbusParseResponse0506},
#endif

#ifdef LIGHTMODBUS_F06M
	{6, modbusParseResponse0506},
#endif

#ifdef LIGHTMODBUS_F15M
	{15, modbusParseResponse1516},
#endif

#ifdef LIGHTMODBUS_F16M
	{16, modbusParseResponse1516},
#endif

#ifdef LIGHTMODBUS_F22M
	{22, modbusParseResponse22},
#endif
};

/**
	\brief Default allocator for the slave based on modbusDefaultAllocator().
*/
LIGHTMODBUS_RET_ERROR modbusMasterDefaultAllocator(ModbusMaster *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose)
{
	return modbusDefaultAllocator(ptr, size, purpose);
}

LIGHTMODBUS_RET_ERROR modbusMasterAllocateRequest(
	ModbusMaster *status,
	uint16_t pdusize)
{
	uint16_t size = pdusize;
	if (pdusize) size += status->request.padding;

	ModbusError err = status->allocator(
		status,
		&status->request.data,
		size,
		MODBUS_MASTER_REQUEST_BUFFER);

	if (err == MODBUS_ERROR_ALLOC || size == 0)
	{
		status->request.data = NULL;
		status->request.pdu = NULL;
		status->request.length = 0;
	}
	else
	{
		status->request.pdu = status->request.data + status->request.pduOffset;
		status->request.length = size;
	}
	
	return err;
}

LIGHTMODBUS_RET_ERROR modbusMasterInit(
	ModbusMaster *status,
	ModbusMasterAllocator allocator,
	ModbusDataCallback dataCallback,
	ModbusMasterExceptionCallback exceptionCallback)
{
	status->allocator = allocator;
	status->dataCallback = dataCallback;
	status->exceptionCallback = exceptionCallback;

	status->functions = modbusMasterDefaultFunctions;
	status->functionCount = MODBUS_MASTER_DEFAULT_FUNCTION_COUNT;

	status->context = NULL;
	status->request.data = NULL;
	status->request.pdu = NULL;
	status->request.length = 0;
	status->request.padding = 0;
	status->request.pduOffset = 0;

	return MODBUS_OK;
}

void modbusMasterDestroy(ModbusMaster *status)
{
	(void) modbusMasterAllocateRequest(status, 0);
}

ModbusMaster *modbusBeginRequestPDU(ModbusMaster *status)
{
	status->request.pduOffset = 0;
	status->request.padding = 0;
	return status;
}

LIGHTMODBUS_RET_ERROR modbusEndRequestPDU(ModbusMaster *status, ModbusError err)
{
	return err;
}

ModbusMaster *modbusBeginRequestRTU(ModbusMaster *status)
{
	status->request.pduOffset = 1;
	status->request.padding = 3;
	return status;
}

/**
	\param err Used for error propagation from modbusBuildRequestxx
	\returns MODBUS_ERROR_LENGTH if the allocated frame is too short 
*/
LIGHTMODBUS_RET_ERROR modbusEndRequestRTU(ModbusMaster *status, uint8_t address, ModbusError err)
{
	if (err) return err;
	if (status->request.length < 4) return MODBUS_ERROR_LENGTH;

	// Put in slave address
	status->request.data[0] = address;

	// Compute and put in CRC
	uint16_t crc = modbusCRC(&status->request.data[0], status->request.length - 2);
	modbusWLE(&status->request.data[status->request.length - 2], crc);

	return MODBUS_OK;
}

ModbusMaster *modbusBeginRequestTCP(ModbusMaster *status)
{
	status->request.pduOffset = 0;
	status->request.padding = 7;
	return status;
}

/**
	\param err Used for error propagation from modbusBuildRequestxx
	\returns MODBUS_ERROR_LENGTH if the allocated frame is too short 
*/
LIGHTMODBUS_RET_ERROR modbusEndRequestTCP(ModbusMaster *status, uint16_t transaction, uint8_t unit, ModbusError err)
{
	if (err) return err;
	if (status->request.length < 7) return MODBUS_ERROR_LENGTH;

	uint16_t length = status->request.length - 6;
	modbusWBE(&status->request.data[0], transaction); // Transaction ID
	modbusWBE(&status->request.data[2], 0);           // Protocol ID
	modbusWBE(&status->request.data[4], length);      // Data length
	status->request.data[6] = unit;                   // Unit ID

	return MODBUS_OK;
}

LIGHTMODBUS_RET_ERROR modbusParseResponsePDU(
	ModbusMaster *status,
	uint8_t address,
	const uint8_t *request,
	uint8_t requestLength,
	const uint8_t *response,
	uint8_t responseLength)
{
	// Check if lengths are ok
	if (!requestLength || !responseLength)
		return MODBUS_ERROR_LENGTH;

	uint8_t function = response[0];

	// Handle exception frames
	if (function & 0x80 && responseLength == 2)
	{
		if (status->exceptionCallback)
			status->exceptionCallback(
				status,
				address,
				function & 0x7f,
				(ModbusExceptionCode) response[1]);

		return MODBUS_OK;
	}

	// Check if function code matches the one in request frame
	if (function != request[0])
		return MODBUS_ERROR_FUNCTION;

	// Find a parsing function
	for (uint16_t i = 0; i < status->functionCount; i++)
		if (function == status->functions[i].id)
			return status->functions[i].ptr(
				status,
				function,
				request,
				requestLength,
				response,
				responseLength);

	// No matching function handler
	return MODBUS_ERROR_FUNCTION;
}

LIGHTMODBUS_RET_ERROR modbusParseResponseRTU(
	ModbusMaster *status,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength)
{
	// Check lengths
	if (requestLength < 4 || responseLength < 4)
		return MODBUS_ERROR_LENGTH;

	// Check CRC
	uint16_t requestCRC = modbusCRC(request, requestLength - 2);
	if (requestCRC != modbusRLE(&request[requestLength - 2]))
		return MODBUS_ERROR_CRC;
	uint16_t responseCRC = modbusCRC(response, responseLength - 2);
	if (responseCRC != modbusRLE(&response[responseLength - 2]))
		return MODBUS_ERROR_CRC;

	// Check addresses
	uint8_t address = request[0];
	if (address == 0 || request[0] != response[0])
		return MODBUS_ERROR_ADDRESS;

	return modbusParseResponsePDU(
		status,
		address,
		request + 1,
		requestLength - 3,
		response + 1,
		responseLength - 3);
}

LIGHTMODBUS_RET_ERROR modbusParseResponseTCP(
	ModbusMaster *status,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength)
{
	// Check lengths
	if (requestLength < 8 || responseLength < 8)
		return MODBUS_ERROR_LENGTH;

	// Check if protocol IDs are correct
	if (modbusRBE(&request[2]) != 0 || modbusRBE(&response[0]) != 0)
		return MODBUS_ERROR_BAD_PROTOCOL;

	// Check if transaction IDs match
	if (modbusRBE(&request[0]) != modbusRBE(&response[0]))
		return MODBUS_ERROR_BAD_TRANSACTION;

	// Check if lengths are ok
	if (modbusRBE(&request[4]) != requestLength - 6
		|| modbusRBE(&response[4]) != responseLength - 6)
		return MODBUS_ERROR_LENGTH;

	//! \todo Check addresses (but how?)
	uint8_t address = response[6];

	return modbusParseResponsePDU(
		status,
		address,
		request + 7,
		requestLength - 7,
		response + 7,
		responseLength - 7);
}