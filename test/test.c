#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define LIGHTMODBUS_FULL
#include <lightmodbus/lightmodbus.h>

#define REG_COUNT 8
static uint16_t registers[REG_COUNT] = {0};
// static uint16_t inputs[REG_COUNT] = {0};
static uint8_t coils[REG_COUNT] = {0};
// static uint8_t discrete[REG_COUNT] = {0};

ModbusError regCallback(
	ModbusSlave *status,
	ModbusDataType type,
	ModbusRegisterQuery query,
	uint8_t function,
	uint16_t id,
	uint16_t value,
	uint16_t *result)
{
	if (query == MODBUS_REGQ_R_CHECK || query == MODBUS_REGQ_W_CHECK)
	{
		*result = id < REG_COUNT ? MODBUS_EXCEP_NONE : MODBUS_EXCEP_ILLEGAL_ADDRESS;
	}
	else if (query == MODBUS_REGQ_R)
	{
		switch (type)
		{
			case MODBUS_HOLDING_REGISTER: *result = registers[id]; break;
			case MODBUS_INPUT_REGISTER:   *result = registers[id]; break;
			case MODBUS_COIL:             *result = modbusMaskRead(coils, id); break;
			case MODBUS_DISCRETE_INPUT:   *result = modbusMaskRead(coils, id); break;
		}
	}
	else if (query == MODBUS_REGQ_W)
	{
		switch (type)
		{
			case MODBUS_HOLDING_REGISTER: registers[id] = value; break;
			// case MODBUS_INPUT_REGISTER:   inputs[id] = value; break;
			case MODBUS_COIL:             modbusMaskWrite(coils, id, value); break;
			// case MODBUS_DISCRETE_INPUT:   modbusMaskWrite(discrete, id, value); break;
			default: break;
		}
	}
	else
		return MODBUS_ERROR_OTHER;

	return MODBUS_OK;
}

void dumpregs(void)
{
	printf("|R ");
	for (int i = 0; i < REG_COUNT; i++)
		printf("%04x ", registers[i]);

	// printf("|I ");
	// for (int i = 0; i < REG_COUNT; i++)
		// printf("%04x ", inputs[i]);

	printf("|C ");
	for (int i = 0; i < REG_COUNT / 8; i++)
		printf("%02x ", coils[i]);

	// printf("|D ");
	// for (int i = 0; i < REG_COUNT; i++)
		// printf("%04x ", discrete[i]);
}

void parse(ModbusSlave *s, const uint8_t *data, int n)
{
	ModbusError err = modbusParseRequestRTU(s, data, n);
	if (!err)
	{
		for (int i = 0; i < s->response.length; i++)
			printf("%02x ", s->response.data[i]);
	}
	else
	{
		printf("ERR %d", err);
	}
	dumpregs();
	putchar('\n');
}

/*
	Single binary request, delimited by EOF
*/
void raw(ModbusSlave *s)
{
	uint8_t data[1024];
	data[0] = 1;
	int n = fread(&data[1], 1, sizeof(data) - 3, stdin);
	modbusWLE(&data[n + 1], modbusCRC(data, n + 1));
	int len = n + 3;
	parse(s, data, len);
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
		// printf("got line '%s'\n", line);

		uint8_t data[1024];

		int offset = 0;
		unsigned int len = 1;
		while (len < sizeof(data) - 2)
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

		// for (int i = 0; i < len; i++)
		// 	printf("%02x ", data[i]);
		// putchar('\n');

		
	}
	free(line);
}

int main()
{
	ModbusSlave slave;
	ModbusError err;
	err = modbusSlaveInit(&slave, 1, modbusSlaveDefaultAllocator, regCallback);
	assert(err == MODBUS_OK && "Init failed!");

	#ifdef FUZZ
	raw(&slave);
	#else
	hex(&slave);
	#endif

	modbusSlaveDestroy(&slave);
}