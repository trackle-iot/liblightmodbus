#ifndef LIGHTMODBUS_BASE_H
#define LIGHTMODBUS_BASE_H

#include <stdint.h>

/**
	This macro can be used to check if all ModbusErrors coming from library
	functions are handled properly
*/
#ifndef LIGHTMOBBUS_WARN_UNUSED
#define LIGHTMODBUS_WARN_UNUSED __attribute__((warn_unused_result))
#endif

#define LIGHTMODBUS_RET_ERROR LIGHTMODBUS_WARN_UNUSED ModbusError

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
	MODBUS_SLAVE_RESPONSE_BUFFER,
	MODBUS_MASTER_REQUEST_BUFFER,
} ModbusBufferPurpose;

/**
	\brief Memory allocator
*/
typedef ModbusError (*ModbusAllocator)(uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose, void *ctx);


LIGHTMODBUS_RET_ERROR modbusDefaultAllocator(
	uint8_t **ptr,
	uint16_t size,
	ModbusBufferPurpose purpose);

uint16_t modbusCRC(const uint8_t *data, uint16_t length);
uint8_t modbusMaskRead(const uint8_t *mask, uint16_t n);
void modbusMaskWrite(uint8_t *mask, uint16_t n, uint8_t value);

uint16_t modbusBitsToBytes(uint16_t n);
uint16_t modbusRLE(const uint8_t *p);
uint16_t modbusWLE(uint8_t *p, uint16_t val);
uint16_t modbusRBE(const uint8_t *p);
uint16_t modbusWBE(uint8_t *p, uint16_t val);

#endif