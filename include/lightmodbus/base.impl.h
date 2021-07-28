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
	\brief Reads n-th bit from an array
	\param mask A pointer to the array
	\param n Number of the bit to be read
	\returns The bit value
*/
LIGHTMODBUS_WARN_UNUSED uint8_t modbusMaskRead(const uint8_t *mask, uint16_t n)
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

/**
	\brief Returns number of bytes necessary to hold given number of bits
	\param n Number of bits
	\returns Number of bytes requred to hold n bits
*/
LIGHTMODBUS_WARN_UNUSED inline uint16_t modbusBitsToBytes(uint16_t n)
{
	return (n + 7) >> 3;
}

/**
	\brief Safely reads a little-endian 16-bit word from provided pointer
*/
LIGHTMODBUS_WARN_UNUSED inline uint16_t modbusRLE(const uint8_t *p)
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
LIGHTMODBUS_WARN_UNUSED inline uint16_t modbusRBE(const uint8_t *p)
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

/**
	\brief Returns uint8_t describing error source of ModbusErrorInfo
	\returns error source

	\see MODBUS_ERROR_SOURCE_GENERAL
	\see MODBUS_ERROR_SOURCE_REQUEST
	\see MODBUS_ERROR_SOURCE_REQUEST
*/
LIGHTMODBUS_WARN_UNUSED inline uint8_t modbusGetErrorSource(ModbusErrorInfo err)
{
	return err.source;
}

/**
	\brief Returns ModbusError contained in ModbusErrorInfo
	\returns MdobusError contained in ModbusErrorInfo
*/
LIGHTMODBUS_WARN_UNUSED inline ModbusError modbusGetErrorCode(ModbusErrorInfo err)
{
	return (ModbusError) err.error;
}

/**
	\brief Checks if ModbusErrorInfo contains an error
	\returns true if ModbusErrorInfo contains an error

	\see MODBUS_NO_ERROR()
*/
LIGHTMODBUS_WARN_UNUSED inline uint8_t modbusIsOk(ModbusErrorInfo err)
{
	return modbusGetErrorSource(err) == MODBUS_ERROR_SOURCE_GENERAL && modbusGetErrorCode(err) == MODBUS_OK;
}

/**
	\brief Returns general error from ModbusErrorInfo
	\returns ModbusError if ModbusErrorInfo contains an error from MODBUS_ERROR_SOURCE_GENERAL
	\returns MODBUS_OK otherwise
*/
LIGHTMODBUS_WARN_UNUSED inline ModbusError modbusGetGeneralError(ModbusErrorInfo err)
{
	return err.source == MODBUS_ERROR_SOURCE_GENERAL ? modbusGetErrorCode(err) : MODBUS_OK;
}

/**
	\brief Returns request error from ModbusErrorInfo
	\returns ModbusError if ModbusErrorInfo contains an error from MODBUS_ERROR_SOURCE_REQUEST
	\returns MODBUS_OK otherwise
*/
LIGHTMODBUS_WARN_UNUSED inline ModbusError modbusGetRequestError(ModbusErrorInfo err)
{
	return err.source == MODBUS_ERROR_SOURCE_REQUEST ? modbusGetErrorCode(err) : MODBUS_OK;
}

/**
	\brief Returns response error from ModbusErrorInfo
	\returns ModbusError if ModbusErrorInfo contains an error from MODBUS_ERROR_SOURCE_RESPONSE
	\returns MODBUS_OK otherwise
*/
LIGHTMODBUS_WARN_UNUSED inline ModbusError modbusGetResponseError(ModbusErrorInfo err)
{
	return err.source == MODBUS_ERROR_SOURCE_RESPONSE ? modbusGetErrorCode(err) : MODBUS_OK;
}
