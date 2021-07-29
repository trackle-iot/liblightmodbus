#include "slave.h"
#include "slave_func.h"

/**
	\brief Associates function IDs with pointers to functions responsible
	for parsing. Length of this array is stored in modbusSlaveDefaultFunctionCount

	\note Contents depend on defined `LIGHTMODBUS_FxxS` macros!
*/
ModbusSlaveFunctionHandler modbusSlaveDefaultFunctions[] =
{
#ifdef LIGHTMODBUS_F01S
	{1, modbusParseRequest01020304},
#endif

#ifdef LIGHTMODBUS_F02S
	{2, modbusParseRequest01020304},
#endif

#ifdef LIGHTMODBUS_F03S
	{3, modbusParseRequest01020304},
#endif

#ifdef LIGHTMODBUS_F04S
	{4, modbusParseRequest01020304},
#endif

#ifdef LIGHTMODBUS_F05S
	{5, modbusParseRequest0506},
#endif

#ifdef LIGHTMODBUS_F06S
	{6, modbusParseRequest0506},
#endif

#ifdef LIGHTMODBUS_F15S
	{15, modbusParseRequest1516},
#endif

#ifdef LIGHTMODBUS_F16S
	{16, modbusParseRequest1516},
#endif

#ifdef LIGHTMODBUS_F22S
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
LIGHTMODBUS_WARN_UNUSED ModbusError modbusSlaveDefaultAllocator(ModbusSlave *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose)
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
	\param allocator Memory allocator to be used (see \ref modbusSlaveDefaultAllocator) (required)
	\param registerCallback Callback function for handling all register operations (required)
	\param exceptionCallback Callback function for handling slave exceptions (optional)
	\param functions Pointer to array of supported function handlers (required).
		The lifetime of this array must not be shorter than the lifetime of the slave.
	\param functionCount Number of function handlers in the array (required)
	\returns MODBUS_NO_ERROR() on success

	\warning This function must not be called on an already initialized ModbusSlave struct.
	\see modbusSlaveDefaultAllocator()
	\see modbusSlaveDefaultFunctions
*/
LIGHTMODBUS_RET_ERROR modbusSlaveInit(
	ModbusSlave *status,
	uint8_t address,
	ModbusSlaveAllocator allocator,
	ModbusRegisterCallback registerCallback,
	ModbusSlaveExceptionCallback exceptionCallback,
	const ModbusSlaveFunctionHandler *functions,
	uint8_t functionCount)
{
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
	\brief Allows user to set the custom context pointer
*/
void modbusSlaveSetUserPointer(ModbusSlave *status, void *ptr)
{
	status->context = ptr;
}

/**
	\brief Retreieves the custom context pointer
*/
void *modbusSlaveGetUserPointer(ModbusSlave *status)
{
	return status->context;
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
	// Call the exception callback
	if (status->exceptionCallback)
		status->exceptionCallback(status, address, function, code);

	// Do not respond if the request was broadcasted
	if (address == 0)
	{
		if (modbusSlaveAllocateResponse(status, 0))
			return MODBUS_GENERAL_ERROR(ALLOC);
		else
			return MODBUS_NO_ERROR();
	}

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
	\param requestLength of the PDU
	\returns Any errors from parsing functions

	\warning This function expects ModbusSlave::response::pduOffset and
	ModbusSlave::response::padding to be set properly! If you're looking for a 
	function that operates strictly on the PDU, please use modbusParseRequestPDU() instead.
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
	\param requestLength length of the PDU
	\returns MODBUS_REQUEST_ERROR(LENGTH) if length of the frame is invalid
	\returns Any errors from parsing functions
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestPDU(ModbusSlave *status, uint8_t address, const uint8_t *request, uint16_t requestLength)
{
	// Check length
	if (!requestLength || requestLength > 255)
		return MODBUS_REQUEST_ERROR(LENGTH);

	status->response.pduOffset = 0;
	status->response.padding = 0;
	return modbusParseRequest(status, address, request, requestLength);
}

/**
	\brief Parses provided Modbus RTU request frame and generates a Modbus RTU response
	\param request pointer to a Modbus RTU frame
	\param requestLength length of the frame
	\returns MODBUS_REQUEST_ERROR(LENGTH) if length of the frame is invalid
	\returns MODBUS_REQUEST_ERROR(CRC) if CRC is invalid
	\returns Any errors from parsing functions
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestRTU(ModbusSlave *status, const uint8_t *request, uint16_t requestLength)
{
	// Check length
	if (requestLength < 4 || requestLength > 256)
		return MODBUS_REQUEST_ERROR(LENGTH);

	// Check if the message is meant for us
	uint8_t address = request[0];
	if (address != status->address || address == 0)
		return MODBUS_NO_ERROR();

	// Check CRC
	if (modbusCRC(request, requestLength - 2) != modbusRLE(request + requestLength - 2))
		return MODBUS_REQUEST_ERROR(CRC);

	// Parse the request
	ModbusErrorInfo err;
	status->response.pduOffset = 1;
	status->response.padding = 3;
	if (!modbusIsOk(err = modbusParseRequest(status, address, request + 1, requestLength - 3)))
		return err;
	
	// Write address and CRC to the reponse
	if (status->response.length)
	{
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
	\param requestLength length of the frame
	\returns MODBUS_REQUEST_ERROR(LENGTH) if length of the frame is invalid or different from the declared one
	\returns MODBUS_REQUEST_ERROR(CRC) if CRC is invalid
	\returns Any errors from parsing functions
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestTCP(ModbusSlave *status, const uint8_t *request, uint16_t requestLength)
{
	// Check length
	if (requestLength < 8 || requestLength > 260)
		return MODBUS_REQUEST_ERROR(LENGTH);

	// Read MBAP header
	uint16_t transactionID = modbusRBE(&request[0]);
	uint16_t protocolID    = modbusRBE(&request[2]);
	uint16_t messageLength = modbusRBE(&request[4]);

	// Discard non-Modbus messages
	if (protocolID != 0)
		return MODBUS_NO_ERROR();

	// Length mismatch
	if (messageLength != requestLength - 6)
		return MODBUS_REQUEST_ERROR(LENGTH);
	
	// Parse the request
	ModbusErrorInfo err;
	status->response.pduOffset = 7;
	status->response.padding = 7;
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