#ifndef LIGHTMODBUS_MASTER_FUNC_H
#define LIGHTMODBUS_MASTER_FUNC_H

#include "base.h"

typedef struct modbusMaster ModbusMaster;

LIGHTMODBUS_RET_ERROR modbusParseResponse01020304(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength);

LIGHTMODBUS_RET_ERROR modbusParseResponse0506(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength);

LIGHTMODBUS_RET_ERROR modbusParseResponse1516(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength);

LIGHTMODBUS_RET_ERROR modbusParseResponse22(
	ModbusMaster *status,
	uint8_t function,
	const uint8_t *requestPDU,
	uint8_t requestLength,
	const uint8_t *responsePDU,
	uint8_t responseLength);

LIGHTMODBUS_RET_ERROR modbusBuildRequest01020304(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count);

LIGHTMODBUS_RET_ERROR modbusBuildRequest01(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count);

LIGHTMODBUS_RET_ERROR modbusBuildRequest02(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count);

LIGHTMODBUS_RET_ERROR modbusBuildRequest03(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count);

LIGHTMODBUS_RET_ERROR modbusBuildRequest04(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count);

LIGHTMODBUS_RET_ERROR modbusBuildRequest0506(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t value);

LIGHTMODBUS_RET_ERROR modbusBuildRequest05(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count);

LIGHTMODBUS_RET_ERROR modbusBuildRequest06(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count);

LIGHTMODBUS_RET_ERROR modbusBuildRequest15(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count,
	const uint8_t *values);

LIGHTMODBUS_RET_ERROR modbusBuildRequest16(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t count,
	const uint16_t *values);

LIGHTMODBUS_RET_ERROR modbusBuildRequest22(
	ModbusMaster *status,
	uint8_t function,
	uint16_t index,
	uint16_t andmask,
	uint16_t ormask);


#endif