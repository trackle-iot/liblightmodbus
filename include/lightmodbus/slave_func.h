#ifndef LIGHTMODBUS_SLAVE_FUNC_H
#define LIGHTMODBUS_SLAVE_FUNC_H

#include "base.h"

typedef struct modbusSlave ModbusSlave;

LIGHTMODBUS_RET_ERROR modbusParseRequest0102(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

LIGHTMODBUS_RET_ERROR modbusParseRequest0304(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

LIGHTMODBUS_RET_ERROR modbusParseRequest05(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

LIGHTMODBUS_RET_ERROR modbusParseRequest06(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

LIGHTMODBUS_RET_ERROR modbusParseRequest15(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

LIGHTMODBUS_RET_ERROR modbusParseRequest16(
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