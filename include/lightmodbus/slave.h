#ifndef LIGHTMODBUS_SLAVE_H
#define LIGHTMODBUS_SLAVE_H

#include <stdint.h>
#include <stddef.h>
#include "base.h"

typedef struct modbusSlave ModbusSlave;

/**
	\brief A pointer to request parsing function
*/
typedef ModbusErrorInfo (*ModbusSlaveParsingFunction)(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

/**
	\brief Associates Modbus function ID with a pointer to a parsing function
*/
typedef struct
{
	uint8_t id;
	ModbusSlaveParsingFunction ptr;
} ModbusSlaveFunctionHandler;

/**
	\brief Determines type of request made to the register callback function
*/
typedef enum
{
	MODBUS_REGQ_R_CHECK, //!< Request for read access
	MODBUS_REGQ_W_CHECK, //!< Request for write access
	MODBUS_REGQ_R,       //!< Read request
	MODBUS_REGQ_W        //!< Write request
} ModbusRegisterQuery;

/**
	\brief Contains arguments for the register callback function
*/
typedef struct
{
	ModbusDataType type;
	ModbusRegisterQuery query;
	uint16_t id;
	uint16_t value;
	uint8_t function;
} ModbusRegisterCallbackArgs;

/**
	\brief A pointer to callback for performing all register operations

	Returning any error from this function results in a 'slave failure' exception
	being returned to the master device.

	\todo requirements description
*/
typedef ModbusError (*ModbusRegisterCallback)(
	ModbusSlave *status,
	const ModbusRegisterCallbackArgs *args,
	uint16_t *result);

/**
	\brief A pointer to a callback called when a Modbus exception is generated (for slave)
*/
typedef void (*ModbusSlaveExceptionCallback)(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	ModbusExceptionCode code);

/**
	\brief A pointer to allocator function called to allocate frame buffers (for slave)
*/
typedef ModbusError (*ModbusSlaveAllocator)(
	ModbusSlave *status,
	uint8_t **ptr,
	uint16_t size,
	ModbusBufferPurpose purpose);

/**
	\brief Slave device status
*/
typedef struct modbusSlave
{
	ModbusSlaveAllocator allocator;                 //!< A pointer to allocator function (required)
	ModbusRegisterCallback registerCallback;        //!< A pointer to register callback (required)
	ModbusSlaveExceptionCallback exceptionCallback; //!< A pointer to exception callback (optional)
	ModbusSlaveFunctionHandler *functions;          //!< A pointer to an array of function handlers (required)
	uint8_t functionCount;                          //!< Number of function handlers in the array (`functions`)
	
	void *context; //!< User's context pointer

	//! Stores slave's response to master
	struct 
	{
		uint8_t *data;      //!< Pointer to the response frame buffer
		uint8_t *pdu;       //!< A pointer to the PDU section of the response frame
		uint16_t length;    //!< Length of the response frame

		uint16_t padding;   //!< Number of extra bytes surrounding the PDU
		uint16_t pduOffset; //!< PDU offset relative to the beginning of the frame
	} response;

	uint8_t address; //!< Slave's address/ID

} ModbusSlave;


LIGHTMODBUS_RET_ERROR modbusSlaveInit(
	ModbusSlave *status,
	uint8_t address,
	ModbusSlaveAllocator allocator,
	ModbusRegisterCallback registerCallback);

void modbusSlaveDestroy(ModbusSlave *status);

LIGHTMODBUS_RET_ERROR modbusBuildException(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	ModbusExceptionCode code);

LIGHTMODBUS_WARN_UNUSED ModbusError modbusSlaveDefaultAllocator(ModbusSlave *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose);
LIGHTMODBUS_WARN_UNUSED ModbusError modbusSlaveAllocateResponse(ModbusSlave *status, uint16_t size);
LIGHTMODBUS_RET_ERROR modbusParseRequestPDU(ModbusSlave *status, uint8_t address, const uint8_t *data, uint8_t length);
LIGHTMODBUS_RET_ERROR modbusParseRequestRTU(ModbusSlave *status, const uint8_t *data, uint16_t length);
LIGHTMODBUS_RET_ERROR modbusParseRequestTCP(ModbusSlave *status, const uint8_t *data, uint16_t length);

extern ModbusSlaveFunctionHandler modbusSlaveDefaultFunctions[];
#define MODBUS_SLAVE_DEFAULT_FUNCTION_COUNT (sizeof(modbusSlaveDefaultFunctions) / sizeof(modbusSlaveDefaultFunctions[0]))

#endif