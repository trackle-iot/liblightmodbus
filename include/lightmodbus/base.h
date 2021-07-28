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

#define LIGHTMODBUS_RET_ERROR LIGHTMODBUS_WARN_UNUSED ModbusErrorInfo

#define MODBUS_ERROR_SOURCE_GENERAL  0u
#define MODBUS_ERROR_SOURCE_REQUEST  1u
#define MODBUS_ERROR_SOURCE_RESPONSE 2u

/**
	\def MODBUS_ERROR_SOURCE_RESERVED
	\brief Reserved for future use.
*/
#define MODBUS_ERROR_SOURCE_RESERVED 3u

#define MODBUS_MAKE_ERROR(s, e) ((ModbusErrorInfo){.source = (s), .error = (e)})
#define MODBUS_NO_ERROR() MODBUS_MAKE_ERROR(MODBUS_ERROR_SOURCE_GENERAL, MODBUS_OK)
#define MODBUS_GENERAL_ERROR(e) MODBUS_MAKE_ERROR(MODBUS_ERROR_SOURCE_GENERAL, (MODBUS_ERROR_##e))
#define MODBUS_REQUEST_ERROR(e) MODBUS_MAKE_ERROR(MODBUS_ERROR_SOURCE_REQUEST, (MODBUS_ERROR_##e))
#define MODBUS_RESPONSE_ERROR(e) MODBUS_MAKE_ERROR(MODBUS_ERROR_SOURCE_RESPONSE, (MODBUS_ERROR_##e))

/**
	\brief Error represenation - source and type of error
*/
typedef struct ModbusErrorInfo
{
	unsigned int source : 2; //!< General/Request/Response/Callback
	unsigned int error  : 6; //!< Contains ModbusError
} ModbusErrorInfo;

/**
	\brief Represtents different kinds of errors.
*/
typedef enum ModbusError
{
	/**
		\brief No error

		Returned when everything is fine.
	*/
	MODBUS_OK = 0,

	/**
		\brief Same as MODBUS_OK
	*/
	MODBUS_ERROR_OK = 0,
	
	/**
		\brief Invalid frame length

		 - The provided frame is either too long or too short
		 - The data length declared in frame does not match the actual frame length
	*/
	MODBUS_ERROR_LENGTH,
	
	/**
		\brief Memory allocation error

		The allocator has failed to allocate requested amount of memory.
	*/
	MODBUS_ERROR_ALLOC,

	/**
		\brief Invalid function

		 - The function codes in the request and response do not match
		 - There is no function handler for this function code (Master)
	*/
	MODBUS_ERROR_FUNCTION,

	/**
		\brief Invalid register count

		Provided register count is 0 or exceeds maximum allowed value.
	*/
	MODBUS_ERROR_COUNT,

	/**
		\todo Do we need this?
	*/
	MODBUS_ERROR_INDEX,

	/**
		\brief Invalid register range

		Returned when accessing `count` registers starting at `index` would cause
		16-bit unsigned int overflow.
	*/
	MODBUS_ERROR_RANGE,

	/**
		\brief CRC invalid

		CRC is invalid in either the request or the response.

		\note Only in Modbus RTU
	*/
	MODBUS_ERROR_CRC,

	/**
		\brief Invalid protocol ID

		The protocol ID in either the request or response frame is not 0.

		\note Only in Modbus TCP
	*/
	MODBUS_ERROR_BAD_PROTOCOL,

	/**
		\brief Mismatched transaction ID
		
		The transaction identifier in the response does not match the one in the request frame.

		\note Only in Modbus TCP
	*/
	MODBUS_ERROR_BAD_TRANSACTION,

	/**
		\brief Invalid slave address

		 - Received response for a broadcasted request
		 - Address byte is different in the request and the response
	*/
	MODBUS_ERROR_ADDRESS,

	/**
		\brief Other error
	*/
	MODBUS_ERROR_OTHER,
} ModbusError;

/**
	\brief Represents a Modbus exception code
*/
typedef enum ModbusExceptionCode
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
typedef enum ModbusDataType
{
	MODBUS_HOLDING_REGISTER = 1, //!< Holding register
	MODBUS_INPUT_REGISTER = 2,   //!< Input register
	MODBUS_COIL = 4,             //!< Coil
	MODBUS_DISCRETE_INPUT = 8    //!< Discrete input
} ModbusDataType;

/**
	\brief Describes what content will be held in the buffer
*/
typedef enum ModbusBufferPurpose
{
	MODBUS_SLAVE_RESPONSE_BUFFER,
	MODBUS_MASTER_REQUEST_BUFFER,
} ModbusBufferPurpose;

/**
	\brief Memory allocator
*/
typedef ModbusError (*ModbusAllocator)(uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose, void *ctx);


LIGHTMODBUS_WARN_UNUSED ModbusError modbusDefaultAllocator(
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

LIGHTMODBUS_WARN_UNUSED uint8_t modbusGetErrorSource(ModbusErrorInfo err);
LIGHTMODBUS_WARN_UNUSED ModbusError modbusGetErrorCode(ModbusErrorInfo err);
LIGHTMODBUS_WARN_UNUSED uint8_t modbusIsOk(ModbusErrorInfo err);
LIGHTMODBUS_WARN_UNUSED ModbusError modbusGetGeneralError(ModbusErrorInfo err);
LIGHTMODBUS_WARN_UNUSED ModbusError modbusGetRequestError(ModbusErrorInfo err);
LIGHTMODBUS_WARN_UNUSED ModbusError modbusGetResponseError(ModbusErrorInfo err);
LIGHTMODBUS_WARN_UNUSED ModbusError modbusGetCallbackError(ModbusErrorInfo err);

#endif