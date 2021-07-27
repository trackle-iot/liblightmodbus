#include <stdio.h>

#define LIGHTMODBUS_FULL
#define LIGHTMODBUS_DEBUG
#define LIGHTMODBUS_IMPL
#include <lightmodbus/lightmodbus.h>

ModbusError regCallback(
	ModbusSlave *status,
	const ModbusRegisterCallbackArgs *args,
	uint16_t *result)
{
	static const char *querynames[4] = {
		[MODBUS_REGQ_R] = "MODBUS_REGQ_R",
		[MODBUS_REGQ_R_CHECK] = "MODBUS_REGQ_R_CHECK",
		[MODBUS_REGQ_W] = "MODBUS_REGQ_W",
		[MODBUS_REGQ_W_CHECK] = "MODBUS_REGQ_W_CHECK",
	};

	printf("%s:\n\tfun: %d\n\tid:  %d\n\tval: %d\n", querynames[args->query], args->function, args->id, args->value);
	
	if (args->query == MODBUS_REGQ_R)
		*result = 1;
	else
		*result = 0;
	return MODBUS_OK;
}

int main(void)
{
	ModbusSlave slave;
	ModbusErrorInfo err;
	err = modbusSlaveInit(&slave, 1, modbusSlaveDefaultAllocator, regCallback);

	uint8_t data[] = {0x01, 0x03, 0x05, 0x00, 0x02, 0x00, 0xff, 0xff};
	modbusWLE(data + sizeof(data) - 2, modbusCRC(data, sizeof(data) - 2));

	err = modbusParseRequestRTU(&slave, data, sizeof(data));
	if (!modbusIsOk(err))
		printf("Parse error: {source: %s, err: %s}\n", modbusErrorSourceStr(err.source), modbusErrorStr(err.code));

	printf("Response: ");
	for (int i = 0; i < slave.response.length; i++)
		printf("0x%02x ", slave.response.data[i]);
	printf("\n");

	modbusSlaveDestroy(&slave);
	return 0;
}
