#ifndef LIGHTMODBUS_MASTER_H
#define LIGHTMODBUS_MASTER_H

#include <stdint.h>
#include <stddef.h>
#include "base.h"

typedef struct modbusMaster ModbusMaster;

/**
	\brief A pointer to response parsing function
*/
typedef ModbusError (*ModbusMasterParsingFunction)(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength,
	ModbusError *responseError);

/**
	\brief Associates Modbus function ID with a pointer to a response parsing function
*/
typedef struct
{
	uint8_t id;
	ModbusMasterParsingFunction ptr;
} ModbusMasterFunctionHandler;

typedef struct ModbusDataCallbackArgs
{
	ModbusDataType type;
	uint16_t id;
	uint16_t value;
	uint8_t function;
};

typedef ModbusError (*ModbusDataCallback)(
	ModbusMaster *status,
	const ModbusDataCallbackArgs *args);

/**
	\brief A pointer to a callback called when a Modbus exception is generated (for master)
*/
typedef void (*ModbusMasterExceptionCallback)(
	ModbusMaster *status,
	uint8_t address,
	uint8_t function,
	ModbusExceptionCode code);

/**
	\brief A pointer to allocator function called to allocate frame buffers (for master)
*/
typedef ModbusError (*ModbusMasterAllocator)(
	ModbusMaster *status,
	uint8_t **ptr,
	uint16_t size,
	ModbusBufferPurpose purpose);

/**
	\brief Master device status
*/
typedef struct modbusMaster
{
	ModbusMasterAllocator allocator;                  //!< A pointer to an allocator function (required)
	ModbusDataCallback dataCallback;                  //!< A pointer to data callback (required)
	ModbusMasterExceptionCallback exceptionCallback;  //!< A pointer to an exception callback (optional)

	const ModbusMasterFunctionHandler *functions; //!< A non-owning pointer to array of function handlers
	uint8_t functionCount; //!< Size of \ref functions array

	void *context; //!< User's context pointer

	//! Stores master's request for slave
	struct
	{
		uint8_t *data;      //!< Pointer to the request frame buffer
		uint8_t *pdu;       //!< Pointer to the PDU section of the request
		uint16_t length;    //!< Length of the request buffer

		uint16_t padding;   //!< Number of extra bytes surrounding the PDU
		uint16_t pduOffset; //!< PDU offset relative to the beginning of the frame
	} request;

} ModbusMaster;

LIGHTMODBUS_RET_ERROR modbusMasterInit(
	ModbusMaster *status,
	ModbusMasterAllocator allocator,
	ModbusDataCallback dataCallback,
	ModbusMasterExceptionCallback exceptionCallback);

void modbusMasterDestroy(ModbusMaster *status);

ModbusMaster *modbusBeginRequestRTU(ModbusMaster *status);
LIGHTMODBUS_RET_ERROR modbusEndRequestRTU(ModbusMaster *status, uint8_t address, ModbusError err);

ModbusMaster *modbusBeginRequestTCP(ModbusMaster *status);
LIGHTMODBUS_RET_ERROR modbusEndRequestTCP(ModbusMaster *status, uint16_t transaction, uint8_t unit, ModbusError err);

#define modbusBuildRequestPDU(s, function, ...) \
	modbusEndRequestPDU((s), modbusBuildRequest##function(modbusBeginRequestPDU((s)), (function), __VA_ARGS__))

#define modbusBuildRequestRTU(s, address, function, ...) \
	modbusEndRequestRTU((s), (address), modbusBuildRequest##function(modbusBeginRequestRTU((s)), (function), __VA_ARGS__))

#define modbusBuildRequestTCP(s, transaction, unit, function, ...) \
	modbusEndRequestTCP((s), (transaction), (unit), modbusBuildRequest##function(modbusBeginRequestTCP((s)), (function), __VA_ARGS__))

LIGHTMODBUS_RET_ERROR modbusMasterDefaultAllocator(ModbusMaster *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose);
LIGHTMODBUS_RET_ERROR modbusMasterAllocateRequest(ModbusMaster *status, uint16_t size);

LIGHTMODBUS_RET_ERROR modbusParseResponsePDU(
	ModbusMaster *status,
	uint8_t address,
	const uint8_t *request,
	uint8_t requestLength,
	const uint8_t *response,
	uint8_t responseLength,
	ModbusError *responseError);

LIGHTMODBUS_RET_ERROR modbusParseResponseRTU(
	ModbusMaster *status,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength,
	ModbusError *responseError);

LIGHTMODBUS_RET_ERROR modbusParseResponseTCP(
	ModbusMaster *status,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength,
	ModbusError *responseError);

extern ModbusMasterFunctionHandler modbusMasterDefaultFunctions[];
#define MODBUS_MASTER_DEFAULT_FUNCTION_COUNT (sizeof(modbusMasterDefaultFunctions) / sizeof(modbusMasterDefaultFunctions[0]))

#endif