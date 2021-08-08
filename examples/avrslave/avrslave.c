/*
	THIS EXAMPLE IS INCOMPLETE
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <assert.h>

#define LIGHTMODBUS_SLAVE_FULL
#define LIGHTMODBUS_IMPL
#include <lightmodbus/lightmodbus.h>

#ifndef REG_COUNT
#define REG_COUNT 16
#endif

#ifndef MAX_REQUEST
#define MAX_REQUEST 64
#endif

#ifndef MAX_RESPONSE
#define MAX_RESPONSE 64
#endif

#ifndef SLAVE_ADDRESS
#define SLAVE_ADDRESS 1
#endif

#ifndef RS485_DIR_RX
#define RS485_DIR_RX
#endif

#ifndef RS485_DIR_TX
#define RS485_DIR_TX
#endif

/*
	Request data. Written inside an interrupt
	hence the 'volatile'.
*/
static volatile uint8_t request[MAX_REQUEST];
static volatile uint16_t requestLength = 0;
static volatile uint8_t requestReady = 0;

// Arrays storing register and coil values
uint16_t regs[REG_COUNT];
uint8_t coils[REG_COUNT / 8];

ISR(TIMER0_COMPA_vect)
{

}

/*
	A custom allocator. Returns memory
	from a statically allocated array.
*/
ModbusError staticSlaveAllocator(
	const ModbusSlave *slave,
	uint8_t **ptr,
	uint16_t size,
	ModbusBufferPurpose purpose)
{
	// Array for holding the response frame
	static uint8_t response[MAX_RESPONSE];
	
	assert(purpose == MODBUS_SLAVE_RESPONSE_BUFFER);

	if (size != 0) // Allocation reqest
	{
		if (size <= MAX_RESPONSE)
		{
			*ptr = response;
			return MODBUS_OK;
		}
		else
		{
			*ptr = NULL;
			return MODBUS_ERROR_ALLOC;
		}
	}
	else // Free request
	{
		*ptr = NULL;
		return MODBUS_OK;
	}
}

/*
	A simple register callback
*/
ModbusError regCallback(
	const ModbusSlave *slave,
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *result)
{
	switch (args->query)
	{
		// All regs can be read
		case MODBUS_REGQ_R_CHECK:
			if (args->index < REG_COUNT)
				result->exceptionCode = MODBUS_EXCEP_NONE;
			else	
				result->exceptionCode = MODBUS_EXCEP_ILLEGAL_ADDRESS;
			break;
			
		// All but two last regs/coils can be written
		case MODBUS_REGQ_W_CHECK:
			if (args->index < REG_COUNT - 2)
				result->exceptionCode = MODBUS_EXCEP_NONE;
			else	
				result->exceptionCode = MODBUS_EXCEP_SLAVE_FAILURE;
			break;

		// Read registers
		case MODBUS_REGQ_R:
			switch (args->type)
			{
				case MODBUS_HOLDING_REGISTER: result->value = regs[args->index]; break;
				case MODBUS_INPUT_REGISTER: result->value = regs[args->index]; break;
				case MODBUS_COIL: result->value = modbusMaskRead(coils, args->index); break;
				case MODBUS_DISCRETE_INPUT: result->value = modbusMaskRead(coils, args->index); break;
			}
			break;

		// Write registers
		case MODBUS_REGQ_W:
			switch (args->type)
			{
				case MODBUS_HOLDING_REGISTER: regs[args->index] = args->value; break;
				case MODBUS_COIL: modbusMaskWrite(coils, args->index, args->value); break;
				default: abort(); break;
			}
			break;
	}

	return MODBUS_OK;
}

/*
	Transmits a single byte over USART
*/
void usartTXB(uint8_t b)
{

}

/*
	Transmits bytes over USART (RS485)
*/
void usartTX(const uint8_t *data, uint16_t length)
{
	// This is because we want to avoid
	// turning on the driver even if the for loop
	// does 0 iterations
	if (!length) return;

	RS485_DIR_TX;

	for (uint16_t i = 0; i < length; i++)
		usartTXB(data[i]);

	RS485_DIR_RX;
}

int main(void)
{
	// Init slave instance
	ModbusErrorInfo err;
	ModbusSlave slave;
	err = modbusSlaveInit(
		&slave,
		regCallback,
		NULL,
		staticSlaveAllocator,
		modbusSlaveDefaultFunctions,
		modbusSlaveDefaultFunctionCount);
	assert(modbusIsOk(err));

	// Enable interrupts and start receiving
	sei();

	while (1)
	{
		if (requestReady)
		{
			// Attempt to parse the received frame
			err = modbusParseRequestRTU(
				&slave,
				SLAVE_ADDRESS,
				(const uint8_t*) request,
				requestLength
			);

			// We ignore request/response errors 
			// and only care about the serious stuff
			switch (modbusGetGeneralError(err))
			{
				// Since we're only doing static memory allocation
				// we can nicely handle memory allocation errors
				// and respond with a slave failure exception
				case MODBUS_ERROR_ALLOC:
					err = modbusBuildExceptionRTU(
						&slave,
						SLAVE_ADDRESS,
						request[1],
						MODBUS_EXCEP_SLAVE_FAILURE);
					
					if (!modbusIsOk(err)) abort();
					break;

				// Oh no.
				default:
					abort();
					break;
			}

			// Respond only if the response can be accessed
			// and has non-zero length
			if (modbusIsOk(err) && modbusSlaveGetResponseLength(&slave))
				usartTX(
					modbusSlaveGetResponse(&slave),
					modbusSlaveGetResponseLength(&slave));

			// We're ready to accept a new request
			requestReady = 0;
		}
	}

	// Technically, we should destroy ModbusSlave here,
	// but... well...
	// modbusSlaveDestroy(&slave);
}