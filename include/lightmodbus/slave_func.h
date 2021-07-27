#ifndef LIGHTMODBUS_SLAVE_FUNC_H
#define LIGHTMODBUS_SLAVE_FUNC_H

#include "base.h"

typedef struct ModbusSlave ModbusSlave;

LIGHTMODBUS_RET_ERROR modbusParseRequest01020304(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

LIGHTMODBUS_RET_ERROR modbusParseRequest0506(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

LIGHTMODBUS_RET_ERROR modbusParseRequest1516(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

LIGHTMODBUS_RET_ERROR modbusParseRequest22(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

#endif