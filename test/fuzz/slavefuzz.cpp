#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#define LIGHTMODBUS_FULL
#define LIGHTMODBUS_DEBUG
#define LIGHTMODBUS_IMPL
#include <lightmodbus/lightmodbus.h>

std::array<uint16_t, 32768> regs;
std::array<uint8_t, 32768> coils;

ModbusError regCallback(
	const ModbusSlave *status,
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *result)
{
	switch (args->query)
	{
		case MODBUS_REGQ_R_CHECK:
		case MODBUS_REGQ_W_CHECK:
		{
			int max;
			switch (args->type)
			{
				case MODBUS_HOLDING_REGISTER: max = regs.size(); break;
				case MODBUS_INPUT_REGISTER:   max = regs.size(); break;
				case MODBUS_COIL:             max = coils.size(); break;
				case MODBUS_DISCRETE_INPUT:   max = coils.size(); break;
				default: throw std::runtime_error{"invalid type in query"}; break;
			}

			result->exceptionCode = args->index < max ? MODBUS_EXCEP_NONE : MODBUS_EXCEP_ILLEGAL_ADDRESS;
		}
		break;

		case MODBUS_REGQ_R:
		{
			switch (args->type)
			{
				case MODBUS_HOLDING_REGISTER: result->value = regs.at(args->index); break;
				case MODBUS_INPUT_REGISTER:   result->value = regs.at(args->index); break;
				case MODBUS_COIL:             result->value = coils.at(args->index); break;
				case MODBUS_DISCRETE_INPUT:   result->value = coils.at(args->index); break;
				default:throw std::runtime_error{"invalid type in read query"}; break;
			}
		}
		break;

		case MODBUS_REGQ_W:
			{
				switch (args->type)
				{
					case MODBUS_HOLDING_REGISTER: regs.at(args->index) = args->value; break;
					case MODBUS_COIL:             coils.at(args->index) = args->value; break;
					default: throw std::runtime_error{"invalid write query"}; break;
				}
			}
			break;

		default:
			throw std::runtime_error{"invalid query"};
			break;
	}

	return MODBUS_OK;
}

void parse(ModbusSlave *s, const uint8_t *data, int n)
{
	n = std::min(255, n);

	ModbusErrorInfo err = modbusParseRequestPDU(s, s->address, data, n);
	if (modbusGetGeneralError(err) != MODBUS_OK)
		throw std::runtime_error{"PDU parse general error"};

	if (modbusIsOk(err))
	{
		std::fwrite(
			modbusSlaveGetResponse(s),
			1,
			modbusSlaveGetResponseLength(s),
			stdout);
	}
}

/*
	Single binary request, delimited by EOF
*/
void raw(ModbusSlave *s)
{
	uint8_t data[1024];
	data[0] = 1;
	int n = std::fread(&data[0], 1, sizeof(data) - 3, stdin);
	// modbusWLE(&data[n + 1], modbusCRC(data, n + 1));
	// int len = n + 3;
	parse(s, data, n);
}

int main()
{
	ModbusSlave slave;
	ModbusErrorInfo err;
	err = modbusSlaveInit(
		&slave,
		1,
		regCallback,
		NULL,
		modbusSlaveDefaultAllocator,
		modbusSlaveDefaultFunctions,
		modbusSlaveDefaultFunctionCount);
	assert(modbusIsOk(err) && "Init failed!");
	raw(&slave);
	modbusSlaveDestroy(&slave);
}