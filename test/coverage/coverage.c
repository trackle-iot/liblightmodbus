#include <lightmodbus/lightmodbus.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define REG_COUNT 8
static uint16_t registers[REG_COUNT] = {0};
// static uint16_t inputs[REG_COUNT] = {0};
static uint8_t coils[REG_COUNT] = {0};
// static uint8_t discrete[REG_COUNT] = {0};

ModbusError regCallback(
	ModbusSlave *status,
	const ModbusRegisterCallbackArgs *args,
	uint16_t *result)
{
	if (args->query == MODBUS_REGQ_R_CHECK || args->query == MODBUS_REGQ_W_CHECK)
	{
		*result = args->id < REG_COUNT ? MODBUS_EXCEP_NONE : MODBUS_EXCEP_ILLEGAL_ADDRESS;
	}
	else if (args->query == MODBUS_REGQ_R)
	{
		switch (args->type)
		{
			case MODBUS_HOLDING_REGISTER: *result = registers[args->id]; break;
			case MODBUS_INPUT_REGISTER:   *result = registers[args->id]; break;
			case MODBUS_COIL:             *result = modbusMaskRead(coils, args->id); break;
			case MODBUS_DISCRETE_INPUT:   *result = modbusMaskRead(coils, args->id); break;
		}
	}
	else if (args->query == MODBUS_REGQ_W)
	{
		switch (args->type)
		{
			case MODBUS_HOLDING_REGISTER: registers[args->id] = args->value; break;
			// case MODBUS_INPUT_REGISTER:   inputs[id] = value; break;
			case MODBUS_COIL:             modbusMaskWrite(coils, args->id, args->value); break;
			// case MODBUS_DISCRETE_INPUT:   modbusMaskWrite(discrete, id, value); break;
			default: break;
		}
	}
	else
		return MODBUS_ERROR_OTHER;

	return MODBUS_OK;
}

void parse(ModbusSlave *s, const uint8_t *data, int n)
{
	ModbusErrorInfo err = modbusParseRequestRTU(s, data, n);
	if (modbusIsOk(err))
	{
		for (int i = 0; i < s->response.length; i++)
			printf("%02x ", s->response.data[i]);
	}
	else
	{
		printf("Error: {source: %s, error: %s}", modbusErrorSourceStr(err.source), modbusErrorStr(err.error));
	}
	// dumpregs();
	putchar('\n');
}

/*
	Accepts multiple requests - one per line in hex format
*/
void hex(ModbusSlave *s)
{
	char *line = NULL;
	size_t linesize = 0;
	while (getline(&line, &linesize, stdin) > 0)
	{
		uint8_t data[1024];

		int offset = 0;
		unsigned int len = 1;
		while (len < sizeof(data) - 3)
		{
			int c = 0, n = 0;
			if (sscanf(&line[offset], "%02x%n", &c, &n) != 1)
				break;
			offset += n;
			data[len++] = c;
		}
		
		data[0] = 1;
		modbusWLE(&data[len], modbusCRC(data, len));
		len += 2;

		parse(s, data, len);
	}
	free(line);
}

int main()
{
	ModbusSlave slave;
	ModbusErrorInfo err;
	err = modbusSlaveInit(&slave, 1, modbusSlaveDefaultAllocator, regCallback, NULL, modbusSlaveDefaultFunctions, modbusSlaveDefaultFunctionCount);
	assert(modbusIsOk(err) && "Init failed!");

	#ifdef FUZZ
	raw(&slave);
	#else
	hex(&slave);
	#endif

	modbusSlaveDestroy(&slave);
}