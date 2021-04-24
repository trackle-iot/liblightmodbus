#ifndef LIGHTMODBUS_SLAVE_H
#define LIGHTMODBUS_SLAVE_H

#include <stdint.h>
#include <stddef.h>
#include "base.h"

typedef struct modbusSlave ModbusSlave;

/**
	\brief Associates Modbus function ID with a pointer to a parsing function
*/
typedef struct
{
	uint8_t id;
	ModbusError (*ptr)(ModbusSlave *status, uint8_t address, uint8_t function, const uint8_t *data, uint8_t size);
} ModbusFunctionHandler;

/**
	\brief 
*/
typedef enum
{
	MODBUS_REGQ_R_CHECK,
	MODBUS_REGQ_W_CHECK,
	MODBUS_REGQ_R,
	MODBUS_REGQ_W
} ModbusRegisterQuery;

typedef ModbusError (*ModbusExceptionCallback)(ModbusSlave *status);

typedef uint16_t (*ModbusRegisterCallback)(
	ModbusSlave *status,
	ModbusDataType type,
	ModbusRegisterQuery query,
	uint8_t function,
	uint16_t id,
	uint16_t value);


typedef ModbusError (*ModbusSlaveAllocator)(ModbusSlave *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose, void *ctx);

/**
	\brief Slave's status
*/
typedef struct modbusSlave
{
	uint8_t address;

	uint8_t *response;
	uint8_t responseLength;

	ModbusFunctionHandler *functions;
	uint8_t functionCount;

	ModbusExceptionCode lastException;

	ModbusSlaveAllocator allocator;
	ModbusRegisterCallback registerCallback;
	ModbusExceptionCallback exceptionCallback;
	
	void *context;

} ModbusSlave;




ModbusError modbusBuildException(ModbusSlave *status, uint8_t address, uint8_t function, ModbusExceptionCode code);

ModbusError modbusParseRequest0304(ModbusSlave *status, uint8_t address, uint8_t function, const uint8_t *data, uint8_t size);

ModbusError modbusParseRequestPDU(ModbusSlave *status, uint8_t address, const uint8_t *data, uint8_t length);

ModbusError modbusParseRequestRTU(ModbusSlave *status, const uint8_t *data, uint16_t length);
ModbusError modbusParseRequestTCP(ModbusSlave *status, const uint8_t *data, uint16_t length);



ModbusError modbusSlaveInit(
	ModbusSlave *status,
	uint8_t address,
	ModbusSlaveAllocator allocator,
	ModbusRegisterCallback registerCallback);

void modbusSlaveDestroy(ModbusSlave *status);

ModbusError modbusSlaveAllocateResponse(ModbusSlave *status, uint16_t size);

ModbusError modbusSlaveDefaultAllocator(ModbusSlave *status, uint8_t **ptr, uint16_t size, ModbusBufferPurpose purpose, void *ctx);



#define MODBUS_SLAVE_DEFAULT_FUNCTION_COUNT 2
extern ModbusFunctionHandler modbusSlaveDefaultFunctions[MODBUS_SLAVE_DEFAULT_FUNCTION_COUNT];






















#ifdef LIGHTMODBUS_SLAVE_BASE

#ifdef LIGHTMODBUS_SLAVE_USER_FUNCTIONS
	struct modbusSlave;
	/**
		\brief Associates user defined parser function with the function ID
		\note Requires `SLAVE_USER_FUNCTIONS` module (see \ref building)
		\see user-functions
	*/
	typedef struct modbusSlaveUserFunction
	{
		uint8_t function; //!< The function code
		/**
			\brief Pointer to the user defined function
			\param status Slave structure to work with
			\param parser The parser structure containing frame data
			\returns A \ref ModbusError error code
		*/
		ModbusError ( *handler )( struct modbusSlave *status, ModbusParser *parser );
	} ModbusSlaveUserFunction;
#endif

#if defined( LIGHTMODBUS_REGISTER_CALLBACK ) || defined( LIGHTMODBUS_COIL_CALLBACK )
	#ifndef LIGHTMODBUS_EXPERIMENTAL
		#error Register callback functions are an experimental feature that may cause problems. Please define LIGHTMODBUS_EXPERIMENTAL to dismiss this error message.
	#endif
	/**
		\brief Represents register callback function
		\note Requires `REGISTER_CALLBACK` or `COIL_CALLBACK` module (see \ref building)
		\see \ref register-callback
	*/
	typedef enum modbusRegisterQuery
	{
		MODBUS_REGQ_R, //!< Requests callback function to return register value
		MODBUS_REGQ_W, //!< Requests callback function to write the register
		MODBUS_REGQ_R_CHECK, //!< Asks callback function if register can be read
		MODBUS_REGQ_W_CHECK //!< Asks callback function if register can be written
	} ModbusRegisterQuery;

	/**
		\brief Type representing a pointer to the user-defined register callback function
		\see \ref register-callback
		\param query A register query (see \ref ModbusRegisterQuery)
		\param datatype Modbus data type
		\param index Address of the register
		\param value Value of the register (only used with write access queries)
		\param ctx User's context data (passed from \ref ModbusSlave::registerCallbackContext)
	*/
	typedef uint16_t ( *ModbusRegisterCallbackFunction )( ModbusRegisterQuery query, ModbusDataType datatype, uint16_t index, uint16_t value, void *ctx );
#endif

/**
	\brief Represents Modbus slave device's status and configuration
*/
typedef struct modbusSlave
{
	uint8_t address; //!< The slave's address

	//Universal register/coil callback function
	#if defined( LIGHTMODBUS_COIL_CALLBACK ) || defined( LIGHTMODBUS_REGISTER_CALLBACK )
		/**
			\brief The pointer to the user-defined register callback function

			\note Requires `LIGHTMODBUS_COIL_CALLBACK` or `LIGHTMODBUS_REGISTER_CALLBACK` to be defined
		*/
		ModbusRegisterCallbackFunction registerCallback;

		/**
			\brief The user data pointer passed to the callback function each time it's used
			\warning This pointer is not managed nor controlled by library.
			So, what you set is what you get.
		*/
		void *registerCallbackContext;
	#endif

	//Slave registers arrays
	#ifndef LIGHTMODBUS_REGISTER_CALLBACK
		uint16_t *registers; //!< Pointer to registers data
		uint16_t *inputRegisters; //!< Pointer to input registers data
		uint8_t *registerMask; //!< Mask for register write protection (each bit corresponds to one register)
		uint16_t registerMaskLength; //!< Write protection mask (\ref registerMask) length in bytes (each byte covers 8 registers)
	#endif
	uint16_t registerCount; //!< Slave's register count
	uint16_t inputRegisterCount; //!< Slave's input register count

	//Slave coils array
	#ifndef LIGHTMODBUS_COIL_CALLBACK
		uint8_t *coils; //!< Pointer to coils data
		uint8_t *discreteInputs; //!< Pointer to discrete inputs data
		uint8_t *coilMask; //!< Masks for coil write protection (each bit corresponds to one coil)
		uint16_t coilMaskLength; //!< Write protection mask (\ref coilMask) length in bytes (each byte covers 8 coils)
	#endif
	uint16_t coilCount; //!< Slave's coil count
	uint16_t discreteInputCount; //!< Slave's discrete input count

	/**
		\brief Exception code of the last exception generated by \ref modbusBuildException
		\see modbusBuildException
	*/


	/**
		\brief More specific error code of problem encountered during frame parsing

		This variable is set up by \ref modbusBuildExceptionErr function
		\see modbusBuildExceptionErr
	*/
	ModbusFrameError parseError;

	//Array of user defined functions - these can override default Modbus functions
	#ifdef LIGHTMODBUS_SLAVE_USER_FUNCTIONS
		/**
			\brief A pointer to user defined Modbus functions array

			\note Requires `SLAVE_USER_FUNCTIONS` module (see \ref building)
			\see user-functions
		*/
		ModbusSlaveUserFunction *userFunctions;
		uint16_t userFunctionCount; //!< Number of user-defined Modbus functions /see userFunctions
	#endif

	/**
		\brief Struct containing slave's response to the master's request

		\note Declaration of the `frame` member depends on the library configuration.
		It can be either a statically allocated array or a pointer to dynamically allocated memory.
		The behavior is dependant on definition of the `LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE` macro

		\see \ref static-mem
	*/
	struct
	{
		#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE
			//! Statically allocated memory for the response frame
			uint8_t frame[LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE];
		#else
			//! A pointer to dynamically allocated memory for the response frame
			uint8_t *frame;
		#endif

		//! Frame length in bytes
		uint8_t length;
	} response;

	/**
		\brief Struct containing master's request frame

		\note Declaration of the `frame` member depends on the library configuration.
		It can be either a statically allocated array or a pointer to dynamically allocated memory.
		The behavior is dependant on definition of the `LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST` macro

		\see \ref static-mem
	*/
	struct
	{
		#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST
			//! Statically allocated memory for the request frame
			uint8_t frame[LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST];
		#else
			//! A pointer to dynamically allocated memory for the request frame
			const uint8_t *frame;
		#endif

		//! Frame length in bytes
		uint8_t length;
	} request;

} ModbusSlave;
#endif

//Function prototypes
#ifdef LIGHTMODBUS_SLAVE_BASE

/**
	\brief Builds an exception frame and stores it in the \ref ModbusSlave structure

	The exception frame is only built if the request was not broadcasted.

	Updates \ref ModbusSlave::lastException member in provided \ref ModbusSlave structure.

	This function is used by more sophisticated \ref modbusBuildExceptionErr().

	\param status The slave structure to work on
	\param function Function code of function throwing the exception
	\param code A Modbus exception code
	\returns A \ref ModbusError error code. Unlike other library functions, this one
	returns \ref MODBUS_ERROR_EXCEPTION on success as a form of information that exception
	had been thrown. If the request was broadcasted, returns \ref MODBUS_ERROR_OK
*/
extern ModbusError modbusBuildException( ModbusSlave *status, uint8_t function, ModbusExceptionCode code );

/**
	\brief Interprets incoming Modbus request frame located in the slave structure

	Firstly, the function frees memory allocated for the previous
	response frame (\ref ModbusSlave::response) (if dynamic memory
	allocation is enabled). Then, integrity of the frame is verified
	with \ref modbusCRC function.

	If the frame CRC is correct, the destination slave address is checked.
	Frames meant for other slave devices are discarded at this point.

	Before attempting to parse the frame with one of the built-in
	parsing functions, the array of user-defined parsing functions
	is searched. It is important to mention that functions defined
	by user override the default ones. That is to say, a NULL pointer
	in the function array can disable support of specific function.

	If matching function is found, it is exected and modbusParseRequest()
	returns the same error code the parsing function code had returned.
	Otherwise, an exception informing the master device of illegal
	function code is constructed, and \ref MODBUS_ERROR_EXCEPTION is returned.

	\param status The slave structure to work on
	\returns A \ref ModbusError error code.
*/
extern ModbusError modbusParseRequest( ModbusSlave *status );

/**
	\brief Performs initialization of the \ref ModbusSlave structure
	\param status The slave structure to be initialized
	\returns A \ref ModbusError error code.
*/
extern ModbusError modbusSlaveInit( ModbusSlave *status );

/**
	\brief Frees memory used by slave structure, previously initialized with \ref modbusSlaveInit
	\param status The slave structure to work on
	\returns A \ref ModbusError error code
*/
extern ModbusError modbusSlaveEnd( ModbusSlave *status );

/**
	\brief Handles Modbus parsing errors

	This function causes an exception frame to be built by \ref modbusBuildException,
	but additionally stores the more verbose error code in the provided slave structure.
	This routine should be used for handling parsing errors, whilst the \ref modbusBuildException()
	should be used for building exception frames with no side effects.

	\param status The slave structure to work on
	\param function Function code of the function throwing the exception
	\param code Exception code to be thrown
	\param parseError More specific error code
	\return An \ref ModbusError error code. Unlike most library functions,
	this one returns `MODBUS_ERROR_PARSE` (when exception frame is not supposed
	to be built) or `MODBUS_ERROR_EXCEPTION` (when exception frame has been
	successfully built) on success. This is because, these are the values that
	shall be returned by a parsing function upon facing parsing problem.
*/
static inline ModbusError modbusBuildExceptionErr( ModbusSlave *status, uint8_t function, ModbusExceptionCode code, ModbusFrameError parseError ) //Build an exception and write error to status->parseError
{
	if ( status == NULL ) return MODBUS_ERROR_NULLPTR;
	status->parseError = parseError;
	ModbusError err = modbusBuildException( status, function, code );
	if ( err == MODBUS_ERROR_OK ) return MODBUS_ERROR_PARSE;
	else return err;
}
#endif



#endif
