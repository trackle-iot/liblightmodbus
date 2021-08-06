#ifndef LIGHTMODBUS_SLAVE_IMPL_H
#define LIGHTMODBUS_SLAVE_IMPL_H

#include "slave.h"
#include "slave_func.h"

/**
	\brief Associates function IDs with pointers to functions responsible
	for parsing. Length of this array is stored in modbusSlaveDefaultFunctionCount

	\note Contents depend on defined `LIGHTMODBUS_FxxS` macros!
*/
ModbusSlaveFunctionHandler modbusSlaveDefaultFunctions[] =
{
#if defined(LIGHTMODBUS_F01S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{1, modbusParseRequest01020304},
#endif

#if defined(LIGHTMODBUS_F02S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{2, modbusParseRequest01020304},
#endif

#if defined(LIGHTMODBUS_F03S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{3, modbusParseRequest01020304},
#endif

#if defined(LIGHTMODBUS_F04S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{4, modbusParseRequest01020304},
#endif

#if defined(LIGHTMODBUS_F05S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{5, modbusParseRequest0506},
#endif

#if defined(LIGHTMODBUS_F06S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{6, modbusParseRequest0506},
#endif

#if defined(LIGHTMODBUS_F15S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{15, modbusParseRequest1516},
#endif

#if defined(LIGHTMODBUS_F16S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{16, modbusParseRequest1516},
#endif

#if defined(LIGHTMODBUS_F22S) || defined(LIGHTMODBUS_SLAVE_FULL)
	{22, modbusParseRequest22},
#endif
};

/**
	\brief Stores length of modbusSlaveDefaultFunctions
*/
const uint8_t modbusSlaveDefaultFunctionCount = sizeof(modbusSlaveDefaultFunctions) / sizeof(modbusSlaveDefaultFunctions[0]);

/**
	\brief Default allocator for the slave based on modbusDefaultAllocator().
	\param ptr Pointer to the pointer to point to the allocated memory.
	\param size Requested size of the memory block
	\param purpose Purpose of the buffer
	\returns MODBUS_ERROR_ALLOC on memory allocation failure
	\returns MODBUS_OK on success
*/
LIGHTMODBUS_WARN_UNUSED ModbusError modbusSlaveDefaultAllocator(const ModbusSlave *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose)
{
	return modbusDefaultAllocator(ptr, size, purpose);
}

/**
	\brief Allocates memory for slave's response frame
	\param pduSize size of the PDU section. 0 if the slave doesn't want to respond.
	\returns \ref MODBUS_ERROR_ALLOC on allocation failure

	If called with size == 0, the response buffer is freed. Otherwise a buffer
	for `(pduSize + status->response.padding)` bytes is allocated. This guarantees
	that if a response is made, the buffer is big enough to hold the entire ADU.

	This function is responsible for managing `data`, `pdu` and `length` fields
	in the response struct. The `pdu` pointer is set up to point `pduOffset` bytes
	after the `data` pointer unless `data` is a null pointer.
*/
LIGHTMODBUS_WARN_UNUSED ModbusError modbusSlaveAllocateResponse(ModbusSlave *status, uint16_t pduSize)
{
	uint16_t size = pduSize;
	if (pduSize) size += status->response.padding;

	ModbusError err = status->allocator(status, &status->response.data, size, MODBUS_SLAVE_RESPONSE_BUFFER);

	if (err == MODBUS_ERROR_ALLOC || size == 0)
	{
		status->response.data = NULL;
		status->response.pdu  = NULL;
		status->response.length = 0;
	}
	else
	{
		status->response.pdu = status->response.data + status->response.pduOffset;
		status->response.length = size;
	}

	return err;
}

/**
	\brief Frees memory allocated for slave's response frame
	
	Calls modbusSlaveAllocateResponse() with size == 0.
*/
void modbusSlaveFreeResponse(ModbusSlave *status)
{
	ModbusError err = modbusSlaveAllocateResponse(status, 0);
	(void) err;
}

/**
	\brief Initializes slave device
	\param address ID of the slave
	\param registerCallback Callback function for handling all register operations (required)
	\param exceptionCallback Callback function for handling slave exceptions (optional)
	\param allocator Memory allocator to be used (see \ref modbusSlaveDefaultAllocator) (required)
	\param functions Pointer to array of supported function handlers (required).
		The lifetime of this array must not be shorter than the lifetime of the slave.
	\param functionCount Number of function handlers in the array (required)
	\returns return MODBUS_GENERAL_ERROR(ADDRESS) if address is 0
	\returns MODBUS_NO_ERROR() on success

	\warning This function must not be called on an already initialized ModbusSlave struct.
	\see modbusSlaveDefaultAllocator()
	\see modbusSlaveDefaultFunctions
*/
LIGHTMODBUS_RET_ERROR modbusSlaveInit(
	ModbusSlave *status,
	uint8_t address,
	ModbusRegisterCallback registerCallback,
	ModbusSlaveExceptionCallback exceptionCallback,
	ModbusSlaveAllocator allocator,
	const ModbusSlaveFunctionHandler *functions,
	uint8_t functionCount)
{
	if (address == 0)
		return MODBUS_GENERAL_ERROR(ADDRESS);

	status->address = address;
	status->response.data = NULL;
	status->response.pdu = NULL;
	status->response.length = 0;
	status->response.padding = 0;
	status->response.pduOffset = 0;
	status->functions = functions;
	status->functionCount = functionCount;
	status->allocator = allocator;
	status->registerCallback = registerCallback;
	status->exceptionCallback = exceptionCallback;
	status->context = NULL;

	return MODBUS_NO_ERROR();
}

/**
	\brief Frees memory allocated in the ModbusSlave struct
*/
void modbusSlaveDestroy(ModbusSlave *status)
{
	ModbusError err = modbusSlaveAllocateResponse(status, 0);
	(void) err;
}

/**
	\brief Builds an exception response frame
	\param address address of the slave
	\param function function that reported the exception
	\param code Modbus exception code
	\returns MODBUS_GENERAL_ERROR(ALLOC) on memory allocation failure
	\returns MODBUS_NO_ERROR() on success
	\note If set, `exceptionCallback` from ModbusSlave is called.
*/
LIGHTMODBUS_RET_ERROR modbusBuildException(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	ModbusExceptionCode code)
{
	// Do not respond if the request was broadcasted
	if (address == 0)
	{
		if (modbusSlaveAllocateResponse(status, 0))
			return MODBUS_GENERAL_ERROR(ALLOC);
		else
			return MODBUS_NO_ERROR();
	}

	// Call the exception callback
	if (status->exceptionCallback)
		status->exceptionCallback(status, address, function, code);

	if (modbusSlaveAllocateResponse(status, 2))
		return MODBUS_GENERAL_ERROR(ALLOC);

	status->response.pdu[0] = function | 0x80;
	status->response.pdu[1] = code;

	return MODBUS_NO_ERROR();
}

/**
	\brief Parses provided PDU and generates response honorinng `pduOffset` and `padding`
		set in ModbusSlave during response generation.
	\param address address of the slave
	\param request pointer to the PDU data
	\param requestLength length of the PDU (valid range: 1 - 253)
	\returns Any errors from parsing functions

	\warning This function expects ModbusSlave::response::pduOffset and
	ModbusSlave::response::padding to be set properly! If you're looking for a 
	function to parse PDU and generate a PDU response, please use modbusParseRequestPDU() instead.

	\warning The response frame can only be accessed if modbusIsOk() called 
		on the return value of this function evaluates to true.
*/
LIGHTMODBUS_RET_ERROR modbusParseRequest(ModbusSlave *status, uint8_t address, const uint8_t *request, uint8_t requestLength)
{
	uint8_t function = request[0];

	// Look for matching function
	for (uint16_t i = 0; i < status->functionCount; i++)
		if (function == status->functions[i].id)
			return status->functions[i].ptr(status, address, function, &request[0], requestLength);

	// No match found
	return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_FUNCTION);
}

/**
	\brief Parses provided PDU and generates PDU for the response frame
	\param address address of the slave
	\param request pointer to the PDU data
	\param requestLength length of the PDU (valid range: 1 - 253)
	\returns MODBUS_REQUEST_ERROR(LENGTH) if length of the frame is invalid
	\returns Any errors from parsing functions

	\warning The response frame can only be accessed if modbusIsOk() called 
		on the return value from this function evaluates to true.

	\warning The `requestLength` argument is  of type `uint8_t` and not `uint16_t`
		as in case of Modbus RTU and TCP.
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestPDU(ModbusSlave *status, uint8_t address, const uint8_t *request, uint8_t requestLength)
{
	// Check length
	if (!requestLength || requestLength > MODBUS_PDU_MAX)
		return MODBUS_REQUEST_ERROR(LENGTH);

	status->response.pduOffset = 0;
	status->response.padding = 0;
	return modbusParseRequest(status, address, request, requestLength);
}

/**
	\brief Parses provided Modbus RTU request frame and generates a Modbus RTU response
	\param request pointer to a Modbus RTU frame
	\param requestLength length of the frame (valid range: 4 - 256)
	\returns MODBUS_REQUEST_ERROR(LENGTH) if length of the frame is invalid
	\returns MODBUS_REQUEST_ERROR(CRC) if CRC is invalid
	\returns MODBUS_GENERAL_ERROR(ADDRESS) if a response was generated to a broadcast request
	\returns Any errors from parsing functions

	\warning The response frame can only be accessed if modbusIsOk() called 
		on the return value of this function evaluates to true.
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestRTU(ModbusSlave *status, const uint8_t *request, uint16_t requestLength)
{
	// Check length
	if (requestLength < MODBUS_RTU_ADU_MIN || requestLength > MODBUS_RTU_ADU_MAX)
		return MODBUS_REQUEST_ERROR(LENGTH);

	// Check if the message is meant for us
	uint8_t address = request[0];
	if (address && address != status->address)
	{
		if (modbusSlaveAllocateResponse(status, 0))
			return MODBUS_GENERAL_ERROR(ALLOC);
		return MODBUS_NO_ERROR();
	}

	// Check CRC
	if (modbusCRC(request, requestLength - 2) != modbusRLE(request + requestLength - 2))
		return MODBUS_REQUEST_ERROR(CRC);

	// Parse the request
	ModbusErrorInfo err;
	status->response.pduOffset = MODBUS_RTU_PDU_OFFSET;
	status->response.padding = MODBUS_RTU_ADU_PADDING;
	if (!modbusIsOk(err = modbusParseRequest(status, address, request + 1, requestLength - 3)))
		return err;
	
	// Write address and CRC to the reponse
	if (status->response.length)
	{
		if (address == 0)
			return MODBUS_GENERAL_ERROR(ADDRESS);

		status->response.data[0] = address;
		modbusWLE(
			&status->response.data[status->response.length - 2],
			modbusCRC(status->response.data, status->response.length - 2)
		);
	}

	return MODBUS_NO_ERROR();
}

/**
	\brief Parses provided Modbus TCP request frame and generates a Modbus TCP response
	\param request pointer to a Modbus TCP frame
	\param requestLength length of the frame (valid range: 8 - 260)
	\returns MODBUS_REQUEST_ERROR(LENGTH) if length of the frame is invalid or different from the declared one
	\returns MODBUS_REQUEST_ERROR(BAD_PROTOCOL) if the frame is not a Modbus TCP message
	\returns Any errors from parsing functions

	\warning The response frame can only be accessed if modbusIsOk() called 
		on the return value of this function evaluates to true.
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestTCP(ModbusSlave *status, const uint8_t *request, uint16_t requestLength)
{
	// Check length
	if (requestLength < MODBUS_TCP_ADU_MIN || requestLength > MODBUS_TCP_ADU_MAX)
		return MODBUS_REQUEST_ERROR(LENGTH);

	// Read MBAP header
	uint16_t transactionID = modbusRBE(&request[0]);
	uint16_t protocolID    = modbusRBE(&request[2]);
	uint16_t messageLength = modbusRBE(&request[4]);

	// Discard non-Modbus messages
	if (protocolID != 0)
		return MODBUS_REQUEST_ERROR(BAD_PROTOCOL);

	// Length mismatch
	if (messageLength != requestLength - 6)
		return MODBUS_REQUEST_ERROR(LENGTH);
	
	// Parse the request
	ModbusErrorInfo err;
	status->response.pduOffset = MODBUS_TCP_PDU_OFFSET;
	status->response.padding = MODBUS_TCP_ADU_PADDING;
	if (!modbusIsOk(err = modbusParseRequest(status, status->address, request + 7, messageLength - 1)))
		return err;

	// Write MBAP header
	if (status->response.length)
	{
		modbusWBE(&status->response.data[0], transactionID);
		modbusWBE(&status->response.data[2], protocolID);
		modbusWBE(&status->response.data[4], status->response.length - 6);
		status->response.data[6] = request[6]; // Copy Unit ID
	}

	return MODBUS_NO_ERROR();
}

#endif