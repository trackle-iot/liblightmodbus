#ifndef LIGHTMODBUS_MASTER_FUNC_H
#define LIGHTMODBUS_MASTER_FUNC_H

#include "base.h"

typedef struct modbusMaster ModbusMaster;

LIGHTMODBUS_RET_ERROR modbusBuildRequest01020304(
	ModbusMaster *status,
	uint8_t address,
	uint8_t function,
	uint16_t index,
	uint16_t count);

LIGHTMODBUS_RET_ERROR modbusBuildRequest0506(
	ModbusMaster *status,
	uint8_t address,
	uint8_t function,
	uint16_t index,
	uint16_t value);

LIGHTMODBUS_RET_ERROR modbusBuildRequest1516(
	ModbusMaster *status,
	uint8_t address,
	uint8_t function,
	uint16_t index,
	uint16_t count,
	const uint8_t *data);

LIGHTMODBUS_RET_ERROR modbusBuildRequest22(
	ModbusMaster *status,
	uint8_t address,
	uint8_t function,
	uint16_t index,
	uint16_t andmask,
	uint16_t ormask);


#endif