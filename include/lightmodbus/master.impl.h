#ifndef LIGHTMODBUS_MASTER_IMPL_H
#define LIGHTMODBUS_MASTER_IMPL_H

#include "master.h"
#include "master_func.h"

/**
	\brief Default array of supported functions. Length is stored in
	modbusMasterDefaultFunctionCount.

	Contents are controlled by defining `LIGHTMODBUS_FxxM` macros.
*/
ModbusMasterFunctionHandler modbusMasterDefaultFunctions[] =
{
#if defined(LIGHTMODBUS_F01M) || defined(LIGHTMODBUS_MASTER_FULL)
	{1, modbusParseResponse01020304},
#endif

#if defined(LIGHTMODBUS_F02M) || defined(LIGHTMODBUS_MASTER_FULL)
	{2, modbusParseResponse01020304},
#endif

#if defined(LIGHTMODBUS_F03M) || defined(LIGHTMODBUS_MASTER_FULL)
	{3, modbusParseResponse01020304},
#endif

#if defined(LIGHTMODBUS_F04M) || defined(LIGHTMODBUS_MASTER_FULL)
	{4, modbusParseResponse01020304},
#endif

#if defined(LIGHTMODBUS_F05M) || defined(LIGHTMODBUS_MASTER_FULL)
	{5, modbusParseResponse0506},
#endif

#if defined(LIGHTMODBUS_F06M) || defined(LIGHTMODBUS_MASTER_FULL)
	{6, modbusParseResponse0506},
#endif

#if defined(LIGHTMODBUS_F15M) || defined(LIGHTMODBUS_MASTER_FULL)
	{15, modbusParseResponse1516},
#endif

#if defined(LIGHTMODBUS_F16M) || defined(LIGHTMODBUS_MASTER_FULL)
	{16, modbusParseResponse1516},
#endif

#if defined(LIGHTMODBUS_F22M) || defined(LIGHTMODBUS_MASTER_FULL)
	{22, modbusParseResponse22},
#endif
};

/**
	\brief Stores length of modbusMasterDefaultFunctions array
*/
const uint8_t modbusMasterDefaultFunctionCount = sizeof(modbusMasterDefaultFunctions) / sizeof(modbusMasterDefaultFunctions[0]);

/**
	\brief Default allocator for master device. Based on modbusDefaultAllocator().
	\param ptr pointer to the pointer to the buffer
	\param size requested size of the buffer
	\param purpose purpose of the buffer
	\returns MODBUS_ERROR_ALLOC on allocation failure
	\returns MODBUS_OK on success
*/
LIGHTMODBUS_WARN_UNUSED ModbusError modbusMasterDefaultAllocator(ModbusMaster *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose)
{
	return modbusDefaultAllocator(ptr, size, purpose);
}

/**
	\brief Allocates memory for the request frame
	\param pduSize size of the PDU section of the frame. 0 implies no request at all.
	\returns MODBUS_ERROR_ALLOC on allocation failure

	If called with size == 0, the request buffer is freed. Otherwise a buffer
	for `(pduSize + status->request.padding)` bytes is allocated. This guarantees
	that if a response is made, the buffer is big enough to hold the entire ADU.

	This function is responsible for managing `data`, `pdu` and `length` fields
	in the request struct. The `pdu` pointer is set up to point `pduOffset` bytes
	after the `data` pointer unless `data` is a null pointer.
*/
LIGHTMODBUS_WARN_UNUSED ModbusError modbusMasterAllocateRequest(
	ModbusMaster *status,
	uint16_t pduSize)
{
	uint16_t size = pduSize;
	if (pduSize) size += status->request.padding;

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

/**
	\brief Frees memory allocated for master's request frame
	
	Calls modbusMasterAllocateRequest() with size == 0.
*/
void modbusMasterFreeRequest(ModbusMaster *status)
{
	ModbusError err = modbusMasterAllocateRequest(status, 0);
	(void) err;
}

/**
	\brief Initializes a ModbusMaster struct
	\param status ModbusMaster struct to be initialized
	\param dataCallback Callback function for handling incoming data (required)
	\param exceptionCallback Callback function for handling slave exceptions (optional)
	\param allocator Memory allocator to be used (see \ref modbusMasterDefaultAllocator()) (required)
	\param functions Pointer to an array of supported function handlers (required). 
		The lifetime of this array must not be shorter than the lifetime of the master.
	\param functionCount Number of elements in the `functions` array (required)
	\returns MODBUS_NO_ERROR() on success

	\see modbusSlaveDefaultAllocator()
	\see modbusMasterDefaultFunctions
*/
LIGHTMODBUS_RET_ERROR modbusMasterInit(
	ModbusMaster *status,
	ModbusDataCallback dataCallback,
	ModbusMasterExceptionCallback exceptionCallback,
	ModbusMasterAllocator allocator,
	const ModbusMasterFunctionHandler *functions,
	uint8_t functionCount)
{
	status->allocator = allocator;
	status->dataCallback = dataCallback;
	status->exceptionCallback = exceptionCallback;

	status->functions = functions;
	status->functionCount = functionCount;

	status->context = NULL;
	status->request.data = NULL;
	status->request.pdu = NULL;
	status->request.length = 0;
	status->request.padding = 0;
	status->request.pduOffset = 0;

	return MODBUS_NO_ERROR();
}

/**
	\brief Deinitializes a ModbusMaster struct
	\param status ModbusMaster struct to be destroyed
	\note This does not free the memory pointed to by the `status` pointer and
	only cleans up the interals ofthe ModbusMaster struct.
*/
void modbusMasterDestroy(ModbusMaster *status)
{
	ModbusError err = modbusMasterAllocateRequest(status, 0);
	(void) err;
}

/**
	\brief Begins a PDU-only request
	\returns MODBUS_NO_ERROR()
*/
LIGHTMODBUS_RET_ERROR modbusBeginRequestPDU(ModbusMaster *status)
{
	status->request.pduOffset = 0;
	status->request.padding = 0;
	return MODBUS_NO_ERROR();
}

/**
	\brief Finalizes a PDU-only request
	\returns MODBUS_NO_ERROR()
*/
LIGHTMODBUS_RET_ERROR modbusEndRequestPDU(ModbusMaster *status)
{
	return MODBUS_NO_ERROR();
}

/**
	\brief Begins a RTU request
	\returns MODBUS_NO_ERROR()
*/
LIGHTMODBUS_RET_ERROR modbusBeginRequestRTU(ModbusMaster *status)
{
	status->request.pduOffset = MODBUS_RTU_PDU_OFFSET;
	status->request.padding = MODBUS_RTU_ADU_PADDING;
	return MODBUS_NO_ERROR();
}

/**
	\brief Finalizes a Modbus RTU request
	\returns MODBUS_REQUEST_ERROR(LENGTH) if the allocated frame has invalid length
	\returns MODBUS_NO_ERROR() on success
*/
LIGHTMODBUS_RET_ERROR modbusEndRequestRTU(ModbusMaster *status, uint8_t address)
{
	if (status->request.length < MODBUS_RTU_ADU_MIN || status->request.length > MODBUS_RTU_ADU_MAX) 
		return MODBUS_REQUEST_ERROR(LENGTH);

	// Put in slave address
	status->request.data[0] = address;

	// Compute and put in CRC
	uint16_t crc = modbusCRC(&status->request.data[0], status->request.length - 2);
	modbusWLE(&status->request.data[status->request.length - 2], crc);

	return MODBUS_NO_ERROR();
}

/**
	\brief Begins a TCP request
	\returns MODBUS_NO_ERROR()
*/
LIGHTMODBUS_RET_ERROR modbusBeginRequestTCP(ModbusMaster *status)
{
	status->request.pduOffset = MODBUS_TCP_PDU_OFFSET;
	status->request.padding = MODBUS_TCP_ADU_PADDING;
	return MODBUS_NO_ERROR();
}

/**
	\brief Finalizes a Modbus TCP request
	\returns MODBUS_REQUEST_ERROR(LENGTH) if the allocated frame has invalid length 
	\returns MODBUS_NO_ERROR() on success
*/
LIGHTMODBUS_RET_ERROR modbusEndRequestTCP(ModbusMaster *status, uint16_t transaction, uint8_t unit)
{
	if (status->request.length < MODBUS_TCP_ADU_MIN || status->request.length > MODBUS_TCP_ADU_MAX)
		return MODBUS_REQUEST_ERROR(LENGTH);

	uint16_t length = status->request.length - 6;
	modbusWBE(&status->request.data[0], transaction); // Transaction ID
	modbusWBE(&status->request.data[2], 0);           // Protocol ID
	modbusWBE(&status->request.data[4], length);      // Data length
	status->request.data[6] = unit;                   // Unit ID

	return MODBUS_NO_ERROR();
}

/**
	\brief Parses a PDU section of a slave response
	\param address Address of the slave that sent in the data
	\param request Pointer to the PDU section of the request frame
	\param requestLength Length of the request PDU (valid range: 1 - 253)
	\param response Pointer to the PDU section of the response
	\param responseLength Length of the response PDU (valid range: 1 - 253)
	\returns MODBUS_REQUEST_ERROR(LENGTH) if the request has invalid length
	\returns MODBUS_RESPONSE_ERROR(LENGTH) if the response has invalid length
	\returns MODBUS_RESPONSE_ERROR(ADDRESS) on attempt to parse a response to a broadcast request
	\returns MODBUS_RESPONSE_ERROR(FUNCTION) if the function code in request doesn't match the one in response
	\returns MODBUS_GENERAL_ERROR(FUNCTION) if the function code is not supported
	\returns Result from the parsing function on success (modbusParseRequest*() functions)
*/
LIGHTMODBUS_RET_ERROR modbusParseResponsePDU(
	ModbusMaster *status,
	uint8_t address,
	const uint8_t *request,
	uint8_t requestLength,
	const uint8_t *response,
	uint8_t responseLength)
{
	// Check if lengths are ok
	if (!requestLength || requestLength > MODBUS_PDU_MAX)
		return MODBUS_REQUEST_ERROR(LENGTH);
	if (!responseLength || responseLength > MODBUS_PDU_MAX)
		return MODBUS_RESPONSE_ERROR(LENGTH);
	
	// Discard responses if the address is broadcast
	if (!address) return MODBUS_RESPONSE_ERROR(ADDRESS);

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

		return MODBUS_NO_ERROR();
	}

	// Check if function code matches the one in request frame
	if (function != request[0])
		return MODBUS_RESPONSE_ERROR(FUNCTION);

	// Find a parsing function
	for (uint16_t i = 0; i < status->functionCount; i++)
		if (function == status->functions[i].id)
			return status->functions[i].ptr(
				status,
				address,
				function,
				request,
				requestLength,
				response,
				responseLength);

	// No matching function handler
	return MODBUS_GENERAL_ERROR(FUNCTION);
}

/**
	\brief Parses a Modbus RTU slave response
	\param request Pointer to the request frame
	\param requestLength Length of the request (valid range: 4 - 256)
	\param response Pointer to the response frame
	\param responseLength Length of the response (valid range: 4 - 256)
	\returns MODBUS_REQUEST_ERROR(LENGTH) if the request has invalid length
	\returns MODBUS_RESPONSE_ERROR(LENGTH) if the response has invalid length
	\returns MODBUS_REQUEST_ERROR(CRC) if the request CRC is invalid
	\returns MODBUS_RESPONSE_ERROR(CRC) if the response CRC is invalid
	\returns MODBUS_RESPONSE_ERROR(ADDRESS) if the address is 0 or if request/response addressess don't match
	\returns Result of modbusParseResponsePDU() otherwise
*/
LIGHTMODBUS_RET_ERROR modbusParseResponseRTU(
	ModbusMaster *status,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength)
{
	// Check lengths
	if (requestLength < MODBUS_RTU_ADU_MIN || requestLength > MODBUS_RTU_ADU_MAX)
		return MODBUS_REQUEST_ERROR(LENGTH);
	if (responseLength < MODBUS_RTU_ADU_MIN || responseLength > MODBUS_RTU_ADU_MAX)
		return MODBUS_RESPONSE_ERROR(LENGTH);
	
	// Request CRC check can be omitted for better performance
	#ifndef LIGHTMODBUS_MASTER_OMIT_REQUEST_CRC
	uint16_t requestCRC = modbusCRC(request, requestLength - 2);
	if (requestCRC != modbusRLE(&request[requestLength - 2]))
		return MODBUS_REQUEST_ERROR(CRC);
	#endif

	// Check response CRC
	uint16_t responseCRC = modbusCRC(response, responseLength - 2);
	if (responseCRC != modbusRLE(&response[responseLength - 2]))
		return MODBUS_RESPONSE_ERROR(CRC);

	// Check addresses
	uint8_t address = request[0];
	if (address == 0 || request[0] != response[0])
		return MODBUS_RESPONSE_ERROR(ADDRESS);

	return modbusParseResponsePDU(
		status,
		address,
		request + MODBUS_RTU_PDU_OFFSET,
		requestLength - MODBUS_RTU_ADU_PADDING,
		response + MODBUS_RTU_PDU_OFFSET,
		responseLength - MODBUS_RTU_ADU_PADDING);
}

/**
	\brief Parses a Modbus TCP slave response
	\param request Pointer to the request frame
	\param requestLength Length of the request (valid range: 8 - 260)
	\param response Pointer to the response frame
	\param responseLength Length of the response (valid range: 8 - 260)
	\returns MODBUS_REQUEST_ERROR(LENGTH) if the request has invalid length or if the request frame has different from declared one
	\returns MODBUS_RESPONSE_ERROR(LENGTH) if the response has invalid length or if the response frame has different from declared one
	\returns MODBUS_REQUEST_ERROR(BAD_PROTOCOL) if the protocol ID in request is not 0
	\returns MODBUS_RESPONSE_ERROR(BAD_PROTOCOL) if the protocol ID in response is not 0
	\returns MODBUS_RESPONSE_ERROR(BAD_TRANSACTION) if the transaction ID in request is not the same as in response
	\returns MODBUS_RESPONSE_ERROR(ADDRESS) if the address in response is not the same as in request
	\returns Result of modbusParseResponsePDU() otherwise
*/
LIGHTMODBUS_RET_ERROR modbusParseResponseTCP(
	ModbusMaster *status,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength)
{
	// Check lengths
	if (requestLength < MODBUS_TCP_ADU_MIN || requestLength > MODBUS_TCP_ADU_MAX)
		return MODBUS_REQUEST_ERROR(LENGTH);
	if (responseLength < MODBUS_TCP_ADU_MIN || responseLength > MODBUS_TCP_ADU_MAX)
		return MODBUS_RESPONSE_ERROR(LENGTH);

	// Check if protocol IDs are correct
	if (modbusRBE(&request[2]) != 0) return MODBUS_REQUEST_ERROR(BAD_PROTOCOL);
	if (modbusRBE(&response[2]) != 0) return MODBUS_RESPONSE_ERROR(BAD_PROTOCOL);

	// Check if transaction IDs match
	if (modbusRBE(&request[0]) != modbusRBE(&response[0]))
		return MODBUS_RESPONSE_ERROR(BAD_TRANSACTION);

	// Check if lengths are ok
	// The declared length includes the Unit ID byte (hence 6 and not 7 bytes are subtracted)
	if (modbusRBE(&request[4]) != requestLength - 6) return MODBUS_REQUEST_ERROR(LENGTH);
	if (modbusRBE(&response[4]) != responseLength - 6) return MODBUS_RESPONSE_ERROR(LENGTH);

	// Check if returned address is the same as in request
	uint8_t address = response[6];
	if (address != request[6])
		return MODBUS_RESPONSE_ERROR(ADDRESS);

	return modbusParseResponsePDU(
		status,
		address,
		request + MODBUS_TCP_PDU_OFFSET,
		requestLength - MODBUS_TCP_ADU_PADDING,
		response + MODBUS_TCP_PDU_OFFSET,
		responseLength - MODBUS_TCP_ADU_PADDING);
}

#endif