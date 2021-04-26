#ifndef LIGHTMODBUS_SLAVE_FUNC_H
#define LIGHTMODBUS_SLAVE_FUNC_H

#include "base.h"

typedef struct modbusSlave ModbusSlave;

LIGHTMODBUS_RET_ERROR modbusParseRequest0304(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size);

#endif