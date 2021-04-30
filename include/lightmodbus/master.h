#ifndef LIGHTMODBUS_MASTER_H
#define LIGHTMODBUS_MASTER_H

#include <stdint.h>
#include "base.h"

typedef struct modbusMaster ModbusMaster;

typedef ModbusError (*ModbusDataCallback)(
	ModbusMaster *status,
	ModbusDataType type,
	uint8_t function,
	uint16_t id,
	uint16_t value);

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

typedef struct modbusMaster
{
	ModbusMasterAllocator allocator;
	ModbusDataCallback dataCallback;
	ModbusMasterExceptionCallback exceptionCallback;

	void *context;

	struct
	{
		uint8_t *data;
		uint8_t *pdu;
		uint16_t length;

		uint16_t padding;
		uint16_t pduOffset;
	} request;

} ModbusMaster;

ModbusMaster *modbusBeginRequestRTU(ModbusMaster *status);
LIGHTMODBUS_RET_ERROR modbusEndRequestRTU(ModbusMaster *status, uint8_t address, ModbusError err);

ModbusMaster *modbusBeginRequestTCP(ModbusMaster *status);
LIGHTMODBUS_RET_ERROR modbusEndRequestTCP(ModbusMaster *status, uint16_t transaction, uint8_t unit, ModbusError err);

#define modbusBuildRequestPDU(s, function, ...) \
	modbusEndRequestPDU((s), modbusBuildRequest##function(modbusBeginRequestPDU((s)), __VA_ARGS__))
#define modbusBuildRequestRTU(s, address, function, ...) \
	modbusEndRequestRTU((s), (address), modbusBuildRequest##function(modbusBeginRequestRTU((s)), __VA_ARGS__))
#define modbusBuildRequestTCP(s, transaction, unit, function, ...) \
	modbusEndRequestTCP((s), (transaction), (unit), modbusBuildRequest##function(modbusBeginRequestTCP((s)), __VA_ARGS__))

LIGHTMODBUS_RET_ERROR modbusMasterDefaultAllocator(ModbusMaster *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose);
LIGHTMODBUS_RET_ERROR modbusMasterAllocateRequest(ModbusMaster *status, uint16_t size);

LIGHTMODBUS_RET_ERROR modbusParseResponsePDU(
	ModbusMaster *status,
	uint8_t address,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength);
	
LIGHTMODBUS_RET_ERROR modbusParseResponseRTU(
	ModbusMaster *status,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength);

LIGHTMODBUS_RET_ERROR modbusParseResponseTCP(
	ModbusMaster *status,
	const uint8_t *request,
	uint16_t requestLength,
	const uint8_t *response,
	uint16_t responseLength);

#endif