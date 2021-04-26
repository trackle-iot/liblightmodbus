#ifndef LIGHTMODBUS_SLAVE_H
#define LIGHTMODBUS_SLAVE_H

#include <stdint.h>
#include <stddef.h>
#include "base.h"

typedef struct modbusSlave ModbusSlave;

/**
	\brief Associates Modbus function ID with a pointer to a parsing function
*/
typedef struct
{
	uint8_t id;
	ModbusError (*ptr)(
		ModbusSlave *status,
		uint8_t address,
		uint8_t function,
		const uint8_t *data,
		uint8_t size);
} ModbusFunctionHandler;

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
	\brief A pointer to callback for performing all register operations

	Returning any error from this function results in a 'slave failure' exception
	being returned to the master device.

	\todo requirements description
*/
typedef ModbusError (*ModbusRegisterCallback)(
	ModbusSlave *status,
	ModbusDataType type,
	ModbusRegisterQuery query,
	uint8_t function,
	uint16_t id,
	uint16_t value,
	uint16_t *result);

/**
	\brief A pointer to callback called when Modbus exception is generated
*/
typedef void (*ModbusExceptionCallback)(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	ModbusExceptionCode code);

/**
	\brief A pointer to allocator function called to allocate frame buffers.

	\todo list requirements
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
	uint8_t address;

	struct 
	{
		uint8_t *data;
		uint8_t *pdu;
		uint16_t length;

		uint16_t padding;
		uint16_t pduOffset;
	} response;

	ModbusFunctionHandler *functions;
	uint8_t functionCount;

	ModbusSlaveAllocator allocator;
	ModbusRegisterCallback registerCallback;
	ModbusExceptionCallback exceptionCallback;
	
	void *context;

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

LIGHTMODBUS_RET_ERROR modbusSlaveDefaultAllocator(ModbusSlave *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose);
LIGHTMODBUS_RET_ERROR modbusSlaveAllocateResponse(ModbusSlave *status, uint16_t size);
LIGHTMODBUS_RET_ERROR modbusParseRequestPDU(ModbusSlave *status, uint8_t address, const uint8_t *data, uint8_t length);
LIGHTMODBUS_RET_ERROR modbusParseRequestRTU(ModbusSlave *status, const uint8_t *data, uint16_t length);
LIGHTMODBUS_RET_ERROR modbusParseRequestTCP(ModbusSlave *status, const uint8_t *data, uint16_t length);

extern ModbusFunctionHandler modbusSlaveDefaultFunctions[];
#define MODBUS_SLAVE_DEFAULT_FUNCTION_COUNT (sizeof(modbusSlaveDefaultFunctions) / sizeof(modbusSlaveDefaultFunctions[0]))

#endif