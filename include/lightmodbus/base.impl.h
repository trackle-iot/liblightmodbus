#include "base.h"
#include <stdlib.h>

/**
	\brief The default memory allocator based on realloc()
	\param ptr a pointer to the pointer to the memory region to be reallocated/freed
	\param size new desired buffer size in bytes
	\param purpose buffer purpose
	\returns MODBUS_ERROR_ALLOC on allocation failure
	\returns MODBUS_OK on success
	\see allocators
*/
LIGHTMODBUS_WARN_UNUSED ModbusError modbusDefaultAllocator(uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose)
{
	(void) purpose;

	// Make sure to handle the case when *ptr = NULL and size = 0
	// We don't want to allocate any memory then, but realloc(NULL, 0) would
	// result in malloc(0)
	if (!size)
	{
		free(*ptr);
		*ptr = NULL;
	}
	else
	{
		uint8_t *old_ptr = *ptr;
		*ptr = (uint8_t*)realloc(*ptr, size);
		
		if (!*ptr)
		{
			free(old_ptr);
			return MODBUS_ERROR_ALLOC;
		}
	}

	return MODBUS_OK;
}

/**
	\brief Calculates 16-bit Modbus CRC of provided data
	\param data A pointer to the data to be processed
	\param length Number of bytes, starting at the `data` pointer, to process
	\returns 16-bit Modbus CRC value
*/
LIGHTMODBUS_WARN_UNUSED uint16_t modbusCRC(const uint8_t *data, uint16_t length)
{
	uint16_t crc = 0xFFFF;

	for (uint16_t i = 0; i < length; i++)
	{
		crc ^= (uint16_t) data[i];
		for (uint8_t j = 8; j != 0; j--)
		{
			if ((crc & 0x0001) != 0)
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
				crc >>= 1;
		}
	}

	return crc;
}
