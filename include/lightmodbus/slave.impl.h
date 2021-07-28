#include "slave.h"
#include "slave_func.h"

/**
	\brief Associates function IDs with pointers to functions responsible
	for parsing.

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
	\brief Default allocator for the slave based on modbusDefaultAllocator().
*/
LIGHTMODBUS_WARN_UNUSED ModbusError modbusSlaveDefaultAllocator(ModbusSlave *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose)
{
	return modbusDefaultAllocator(ptr, size, purpose);
}

/**
	\brief Allocates memory for slave's response frame
	\param pdusize size of the PDU section. 0 if the slave doesn't want to respond.
	\returns \ref MODBUS_ERROR_ALLOC on allocation failure

	If called with size == 0, the response buffer is freed. Otherwise a buffer
	for `(pdusize + status->response.padding)` bytes is allocated. This guarantees
	that if a response is made, the buffer is big enough to hold the entire ADU.

	This function is responsible for managing `data`, `pdu` and `length` fields
	in the response struct. The `pdu` pointer is set up to point `pduOffset` bytes
	after the `data` pointer unless `data` is a null pointer.
*/
LIGHTMODBUS_WARN_UNUSED ModbusError modbusSlaveAllocateResponse(ModbusSlave *status, uint16_t pdusize)
{
	uint16_t size = pdusize;
	if (pdusize) size += status->response.padding;

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
	\param registerCallback Callback function handling all register operations (required)
	\warning This function must not be called on an already initialized ModbusSlave struct.
	\see modbusSlaveDefaultAllocator()
	\see modbusSlaveDefaultFunctions
*/
LIGHTMODBUS_RET_ERROR modbusSlaveInit(
	ModbusSlave *status,
	uint8_t address,
	ModbusSlaveAllocator allocator,
	ModbusRegisterCallback registerCallback)
{
	status->address = address;
	status->response.data = NULL;
	status->response.pdu = NULL;
	status->response.length = 0;
	status->response.padding = 0;
	status->response.pduOffset = 0;
	status->functions = modbusSlaveDefaultFunctions;
	status->functionCount = MODBUS_SLAVE_DEFAULT_FUNCTION_COUNT;
	status->allocator = allocator;
	status->registerCallback = registerCallback;
	status->exceptionCallback = NULL;
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

	ModbusSlave.exceptionCallback is called if set.
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
	\brief Parses provided PDU (Protocol Data Unit)
	\warning This function expects ModbusSlave::response::pduOffset and
	ModbusSlave::response::padding to be set properly!
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestPDU(ModbusSlave *status, uint8_t address, const uint8_t *data, uint8_t length)
{
	uint8_t function = data[0];

	// Look for matching function
	for (uint16_t i = 0; i < status->functionCount; i++)
		if (function == status->functions[i].id)
			return status->functions[i].ptr(status, address, function, &data[0], length);

	// No match found
	return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_FUNCTION);
}

/**
	\brief Parses provided Modbus RTU request frame
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestRTU(ModbusSlave *status, const uint8_t *data, uint16_t length)
{
	// Check length
	if (length < 4 || length > 256)
		return MODBUS_REQUEST_ERROR(LENGTH);

	// Check if the message is meant for us
	uint8_t address = data[0];
	if (address != status->address || address == 0)
		return MODBUS_NO_ERROR();

	//! \todo Should we check if address is less than 248?

	// Check CRC
	if (modbusCRC(data, length - 2) != modbusRLE(data + length - 2))
		return MODBUS_REQUEST_ERROR(CRC);

	// Parse the request
	ModbusErrorInfo err;
	status->response.pduOffset = 1;
	status->response.padding = 3;
	if (!modbusIsOk(err = modbusParseRequestPDU(status, address, data + 1, length - 3)))
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
	\brief Parses provided Modbus TCP request frame
*/
LIGHTMODBUS_RET_ERROR modbusParseRequestTCP(ModbusSlave *status, const uint8_t *data, uint16_t length)
{
	// Check length
	if (length < 8 || length > 260)
		return MODBUS_REQUEST_ERROR(LENGTH);

	// Read MBAP header
	uint16_t transactionID = modbusRBE(&data[0]);
	uint16_t protocolID    = modbusRBE(&data[2]);
	uint16_t messageLength = modbusRBE(&data[4]);
	uint8_t address = status->address;

	// Discard non-Modbus messages
	if (protocolID != 0)
		return MODBUS_NO_ERROR();

	// Length mismatch
	if (messageLength != length - 6)
		return MODBUS_REQUEST_ERROR(OTHER); // TODO?
	
	// Parse the request
	ModbusErrorInfo err;
	status->response.pduOffset = 7;
	status->response.padding = 7;
	if (!modbusIsOk(err = modbusParseRequestPDU(status, address, data + 7, messageLength - 1)))
		return err;

	// Write MBAP header
	if (status->response.length)
	{
		modbusWBE(&status->response.data[0], transactionID);
		modbusWBE(&status->response.data[2], protocolID);
		modbusWBE(&status->response.data[4], status->response.length - 6);
		status->response.data[6] = address;
	}

	return MODBUS_NO_ERROR();
}