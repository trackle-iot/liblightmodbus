#ifndef LIGHTMODBUS_BASE_H
#define LIGHTMODBUS_BASE_H

#include <stdint.h>

/**
	\brief Represents a library runtime error code.
*/
typedef enum modbusError
{
	MODBUS_OK = 0, //!< No error
	MODBUS_ERROR_LENGTH, //!< Frame too long or too short
	MODBUS_ERROR_ALLOC,
	MODBUS_ERROR_OTHER,
} ModbusError;


/**
	\brief Provides more information on frame building/parsing error.

	These error code should serve as an additional source of information for the user.
*/
/*
typedef enum modbusFrameError
{
	MODBUS_FERROR_OK = 0,     //!< Modbus frame OK. No error.
	MODBUS_FERROR_CRC,        //!< Invalid CRC
	MODBUS_FERROR_LENGTH,     //!< Invalid frame length
	MODBUS_FERROR_COUNT,      //!< Invalid declared data item count
	MODBUS_FERROR_VALUE,      //!< Illegal data value (eg. when writing a single coil)
	MODBUS_FERROR_RANGE,      //!< Invalid register range
	MODBUS_FERROR_NOREAD,     //!< No read access to at least one of requested regsiters
	MODBUS_FERROR_NOWRITE,    //!< No write access to at least one of requested regsiters
	MODBUS_FERROR_NOFUN,      //!< Function not supported
	MODBUS_FERROR_BADFUN,     //!< Requested a parsing function to parse a frame with wrong function code
	MODBUS_FERROR_NULLFUN,    //!< Function overriden by user with NULL pointer.
	MODBUS_FERROR_MISM_FUN,   //!< Function request-response mismatch
	MODBUS_FERROR_MISM_ADDR,  //!< Slave address request-response mismatch
	MODBUS_FERROR_MISM_INDEX, //!< Index value request-response mismatch
	MODBUS_FERROR_MISM_COUNT, //!< Count value request-response mismatch
	MODBUS_FERROR_MISM_VALUE, //!< Data value request-response mismatch
	MODBUS_FERROR_MISM_MASK,  //!< Mask value request-response mismatch
	MODBUS_FERROR_BROADCAST   //!< Received response for broadcast message
} ModbusFrameError;
*/

/**
	\brief Represents a Modbus exception code
*/
typedef enum modbusExceptionCode
{
	MODBUS_EXCEP_NONE = 0,
	MODBUS_EXCEP_ILLEGAL_FUNCTION = 1, //!< Illegal function code
	MODBUS_EXCEP_ILLEGAL_ADDRESS = 2,  //!< Illegal data address
	MODBUS_EXCEP_ILLEGAL_VALUE = 3,    //!< Illegal data value
	MODBUS_EXCEP_SLAVE_FAILURE = 4,    //!< Slave could not process the request
	MODBUS_EXCEP_ACK = 5,              //!< Acknowledge
	MODBUS_EXCEP_NACK = 7              //!< Negative acknowledge
} ModbusExceptionCode;

/**
	\brief Represents different Modbus data types
*/
typedef enum modbusDataType
{
	MODBUS_HOLDING_REGISTER = 1, //!< Holding register
	MODBUS_INPUT_REGISTER = 2,   //!< Input register
	MODBUS_COIL = 4,             //!< Coil
	MODBUS_DISCRETE_INPUT = 8    //!< Discrete input
} ModbusDataType;

/**
	\brief Describes what content will be held in the buffer
*/
typedef enum modbusBufferPurpose
{
	MODBUS_RESPONSE_BUFFER,
	MODBUS_REQUEST_BUFFER,
} ModbusBufferPurpose;

/**
	\brief Memory allocator


*/
typedef ModbusError (*ModbusAllocator)(uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose, void *ctx);
extern ModbusError modbusDefaultAllocator(uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose, void *ctx);


/**
	\brief Returns number of bytes necessary to hold given number of bits
	\param n Number of bits
	\returns Number of bytes
*/
static inline uint16_t modbusBitsToBytes(uint16_t n)
{
	return (n + 1) >> 3;
}

/**
	\brief Safely reads a little-endian 16-bit word from provided pointer
*/
static inline uint16_t modbusRLE(const uint8_t *p)
{
#ifdef LIGHTMODBUS_LITTLE_ENDIAN
	uint8_t lo = *p;
	uint8_t hi = *(p + 1);
#else
	uint8_t lo = *(p + 1);
	uint8_t hi = *p;
#endif
	return (uint16_t) lo | ((uint16_t) hi << 8);
}

/**
	\brief Safely writes a little-endian 16-bit word to provided pointer
*/
static inline uint16_t modbusWLE(uint8_t *p, uint16_t val)
{
#ifdef LIGHTMODBUS_LITTLE_ENDIAN
	*p = val;
	*(p + 1) = val >> 8;
#else
	*p = val >> 8;
	*(p + 1) = val;
#endif
	return val;
}

/**
	\brief Safely reads a big-endian 16-bit word from provided pointer
*/
static inline uint16_t modbusRBE(const uint8_t *p)
{
#ifdef LIGHTMODBUS_LITTLE_ENDIAN
	uint8_t hi = *p;
	uint8_t lo = *(p + 1);
#else
	uint8_t hi = *(p + 1);
	uint8_t lo = *p;
#endif
	return (uint16_t) lo | ((uint16_t) hi << 8);
}

/**
	\brief Safely writes a big-endian 16-bit word to provided pointer
*/
static inline uint16_t modbusWBE(uint8_t *p, uint16_t val)
{
#ifdef LIGHTMODBUS_LITTLE_ENDIAN
	*(p + 1) = val;
	*p = val >> 8;
#else
	*p = val;
	*(p + 1) = val >> 8;
#endif
	return val;
}

extern uint16_t modbusCRC(const uint8_t *data, uint16_t length);
extern uint8_t modbusMaskRead(const uint8_t *mask, uint16_t n);
extern void modbusMaskWrite(uint8_t *mask, uint16_t n, uint8_t value);

#endif