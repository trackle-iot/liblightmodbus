#include "slave_func.h"
#include "slave.h"

LIGHTMODBUS_RET_ERROR modbusParseRequest0304(
	ModbusSlave *status,
	uint8_t address,
	uint8_t function,
	const uint8_t *data,
	uint8_t size)
{
	// Do not respond if the request was broadcasted
	if (address == 0)
		return modbusSlaveAllocateResponse(status, 0);

	ModbusDataType datatype = function == 3 ? MODBUS_HOLDING_REGISTER : MODBUS_INPUT_REGISTER;
	uint16_t index = modbusRBE(&data[0]);
	uint16_t count = modbusRBE(&data[2]);

	if (count == 0 || count > 125)
		return modbusBuildException(status, address, function, MODBUS_EXCEP_ILLEGAL_VALUE);

	uint8_t fail = 0;
	ModbusExceptionCode ex = MODBUS_EXCEP_NONE;

	// Check if all registers can be read
	for (uint16_t i = index; !fail && !ex && i < index + count; i++)
	{
		uint16_t res = MODBUS_OK;
		fail = status->registerCallback(status, datatype, MODBUS_REGQ_R_CHECK, function, i, 0, &res);
		if (res != MODBUS_OK)
			ex = res;
	}

	// ---- RESPONSE ----

	// Return exceptions (if any)
	if (fail) return modbusBuildException(status, address, function, MODBUS_EXCEP_SLAVE_FAILURE);
	if (ex) return modbusBuildException(status, address, function, ex);

	ModbusError err = modbusSlaveAllocateResponse(status, 2 + (count << 1));
	if (err) return err;

	status->response.pdu[0] = function;
	status->response.pdu[1] = count << 1;
	
	for (uint16_t i = 0; i < count; i++)
	{
		uint16_t value;
		(void) status->registerCallback(status, datatype, MODBUS_REGQ_R, function, index + i, 0, &value);
		modbusWBE(&status->response.pdu[2 + (i << 1)], value);
	}

	return MODBUS_OK;
}