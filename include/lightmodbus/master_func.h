#ifndef LIGHTMODBUS_MASTER_FUNC_H
#define LIGHTMODBUS_MASTER_FUNC_H

#include "base.h"
#include "master.h"

#define LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(f, f_suffix, ...) \
	LIGHTMODBUS_RET_ERROR modbusBuildRequest##f_suffix##PDU(ModbusMaster *status, __VA_ARGS__)

#define LIGHTMODBUS_DEFINE_BUILD_PDU_BODY(f, f_suffix, ...) \
	{ \
		ModbusErrorInfo err; \
		if (!modbusIsOk(err = modbusBeginRequestPDU(status))) return err; \
		if (!modbusIsOk(err = modbusBuildRequest##f_suffix(status, f, __VA_ARGS__))) return err; \
		if (!modbusIsOk(err = modbusEndRequestPDU(status)))  return err; \
		return MODBUS_NO_ERROR(); \
	}

#define LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(f, f_suffix, ...) \
	LIGHTMODBUS_RET_ERROR modbusBuildRequest##f_suffix##RTU(ModbusMaster *status, uint8_t address, __VA_ARGS__)

#define LIGHTMODBUS_DEFINE_BUILD_RTU_BODY(f, f_suffix, ...) \
	{ \
		ModbusErrorInfo err; \
		if (!modbusIsOk(err = modbusBeginRequestRTU(status))) return err; \
		if (!modbusIsOk(err = modbusBuildRequest##f_suffix(status, f, __VA_ARGS__))) return err; \
		if (!modbusIsOk(err = modbusEndRequestRTU(status, address)))  return err; \
		return MODBUS_NO_ERROR(); \
	}

#define LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(f, f_suffix, ...) \
	LIGHTMODBUS_RET_ERROR modbusBuildRequest##f_suffix##TCP(ModbusMaster *status, uint16_t transaction, uint8_t unit, __VA_ARGS__)

#define LIGHTMODBUS_DEFINE_BUILD_TCP_BODY(f, f_suffix, ...) \
	{ \
		ModbusErrorInfo err; \
		if (!modbusIsOk(err = modbusBeginRequestTCP(status))) return err; \
		if (!modbusIsOk(err = modbusBuildRequest##f_suffix(status, f, __VA_ARGS__))) return err; \
		if (!modbusIsOk(err = modbusEndRequestTCP(status, transaction, unit)))  return err; \
		return MODBUS_NO_ERROR(); \
	}

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


LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(1, 01, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(1, 01, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(1, 01, uint16_t index, uint16_t count);

LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(2, 02, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(2, 02, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(2, 02, uint16_t index, uint16_t count);

LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(3, 03, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(3, 03, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(3, 03, uint16_t index, uint16_t count);

LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(4, 04, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(4, 04, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(4, 04, uint16_t index, uint16_t count);

LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(5, 05, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(5, 05, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(5, 05, uint16_t index, uint16_t count);

LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(6, 06, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(6, 06, uint16_t index, uint16_t count);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(6, 06, uint16_t index, uint16_t count);

LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(15, 15, uint16_t index, uint16_t count, const uint8_t *values);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(15, 15, uint16_t index, uint16_t count, const uint8_t *values);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(15, 15, uint16_t index, uint16_t count, const uint8_t *values);

LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(16, 16, uint16_t index, uint16_t count, const uint16_t *values);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(16, 16, uint16_t index, uint16_t count, const uint16_t *values);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(16, 16, uint16_t index, uint16_t count, const uint16_t *values);

LIGHTMODBUS_DEFINE_BUILD_PDU_HEADER(22, 22, uint16_t index, uint16_t andmask, uint16_t ormask);
LIGHTMODBUS_DEFINE_BUILD_RTU_HEADER(22, 22, uint16_t index, uint16_t andmask, uint16_t ormask);
LIGHTMODBUS_DEFINE_BUILD_TCP_HEADER(22, 22, uint16_t index, uint16_t andmask, uint16_t ormask);

#endif