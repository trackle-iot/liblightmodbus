#include "base.h"
#include <stdlib.h>

/**
	\brief The default memory allocator based on realloc()

	## Allocator requirements

	The allocator function must meet following requirements:
	 - When `size` is 0, the memory pointed to by `*ptr` must be freed. `*ptr`
	   shall then be set to NULL.
	 - When `size` is not 0, `*ptr` should be set to point to a memory block of
	   `size` bytes. If such block cannot be allocated, the allocator must
	   return MODBUS_ERROR_ALLOC and free memory block pointed to by current
	   `*ptr` value.

	Certain guarantees are made by the library:
	 - All allocation requests originating from a single ModbusMaster or
	   ModbusSlave instance refer to the same buffer as long as `purpose`
	   argument has the same value. **This allows for static memory allocation
	   if the allocator can distinguish between different ModbusMaster and
	   ModbusSlave instances that can issue the allocation request.**
*/
LIGHTMODBUS_RET_ERROR modbusDefaultAllocator(uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose)
{
	(void) purpose;

	// Make sure to handle the case when *ptr = NULL and size = 0
	// We don't want to allocate any memory then, but realloc would
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
	\brief Reads n-th bit from an array

	\param mask A pointer to the array
	\param n Number of the bit to be read
	\returns The bit value
*/
uint8_t modbusMaskRead(const uint8_t *mask, uint16_t n)
{
	return (mask[n >> 3] & (1 << (n & 7))) != 0;
}

/**
	\brief Writes n-th bit in an array

	\param mask A pointer to the array
	\param n Number of the bit to write
	\param value Bit value to be written
*/
void modbusMaskWrite(uint8_t *mask, uint16_t n, uint8_t value)
{
	if (value)
		mask[n >> 3] |= (1 << (n & 7));
	else
		mask[n >> 3] &= ~(1 << (n & 7));
}

/**
	\brief Calculates 16-bit Modbus CRC of provided data

	\param data A pointer to the data to be processed
	\param length Number of bytes, starting at the `data` pointer, to process
	\returns 16-bit Modbus CRC value
*/
uint16_t modbusCRC(const uint8_t *data, uint16_t length)
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

/**
	\brief Returns number of bytes necessary to hold given number of bits
	\param n Number of bits
	\returns Number of bytes
*/
inline uint16_t modbusBitsToBytes(uint16_t n)
{
	return (n + 7) >> 3;
}

/**
	\brief Safely reads a little-endian 16-bit word from provided pointer
*/
inline uint16_t modbusRLE(const uint8_t *p)
{
	uint8_t lo = *p;
	uint8_t hi = *(p + 1);
	return (uint16_t) lo | ((uint16_t) hi << 8);
}

/**
	\brief Safely writes a little-endian 16-bit word to provided pointer
*/
inline uint16_t modbusWLE(uint8_t *p, uint16_t val)
{
	*p = val;
	*(p + 1) = val >> 8;
	return val;
}

/**
	\brief Safely reads a big-endian 16-bit word from provided pointer
*/
inline uint16_t modbusRBE(const uint8_t *p)
{
	uint8_t lo = *(p + 1);
	uint8_t hi = *p;
	return (uint16_t) lo | ((uint16_t) hi << 8);
}

/**
	\brief Safely writes a big-endian 16-bit word to provided pointer
*/
inline uint16_t modbusWBE(uint8_t *p, uint16_t val)
{
	*p = val >> 8;
	*(p + 1) = val;
	return val;
}