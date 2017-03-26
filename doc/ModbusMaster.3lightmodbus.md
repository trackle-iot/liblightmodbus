# ModbusMaster 3lightmodbus "25 March 2017" "v1.3"

## NAME
**ModbusMaster** - data type containing all information about current master device status, its configuration and received data.

## SYNOPSIS
`  
	#include <lightmodbus/master.h>
`

`  
	#define MODBUS_HOLDING_REGISTER 1
	#define MODBUS_INPUT_REGISTER 2
	#define MODBUS_COIL 4
	#define MODBUS_DISCRETE_INPUT 8
`

`  
	typedef struct
	{
		uint8_t predictedResponseLength; //If everything goes fine, slave will return this amount of data
`

`  
		struct //Formatted request for slave
		{
			uint8_t *frame;
			uint8_t length;
		} request;
`

`  
		struct //Response from slave should be put here
		{
			uint8_t *frame;
			uint8_t length;
		} response;
`

`  
		struct //Data read from slave
		{
			uint8_t address; //Address of slave
			uint16_t index; //Address of the first element (in slave device)
			uint16_t count; //Count of data units (coils, registers, etc.)
			uint8_t length; //Length of data in bytes
			uint8_t type; //Type of data
			uint8_t function; //Function that accessed the data
			//Two separate pointers are used in case pointer size differed between types (possible on some weird architectures)
			uint8_t *coils; //Received data
			uint16_t *regs; //And the same received data, but converted to uint16_t pointer for convenience
		} data;
`

`  
		struct //Exceptions read are stored in this structure
		{
			uint8_t address; //Device address
			uint8_t function; //In which function exception occurred
			uint8_t code; //Exception code
		} exception;
`

`  
	} ModbusMaster; //Type containing master device configuration data
`

## DESCRIPTION
The **ModbusMaster** contains information about master device configuration, status and received data.

| member name | description |
|---|---|
| `predictedResponseLength` | the predicted length of response |
| `request.frame` | request frame for slave device |
| `request.length`| request frame length |
| `response.frame` | incoming response frame |
| `response.length`| response frame length |
| `data.address` | address of the slave the data comes from |
| `data.index` | starting index of received data |
| `data.count` | number of received data units |
| `data.length` | number of received data bytes |
| `data.type` | type of received data |
| `data.function` | number of function that returned the data |
| `data.coils` | received coils or discrete input values |
| `data.regs` | received (input or holding) register values |
| `exception.address` | address of device that threw the exception |
| `exception.function` | number of function that threw the exception |
| `exception.code` | Modbus exception code |

**Note:** In *status.data.coils* each **bit** corresponds to a **single** coil or an discrete input.

### Initialization
Unlike on slave side, there aren't many things to be done before start. User only needs to call **modbusMasterInit** on the structure.

Simple initialization example:
`  
	ModbusMaster status;
	modbusMasterInit( &status );
`

### Normal use
Normally, master initiates the transmission, by sending a request to slave device, and then awaits its response. Let's say we want to write a single holding register - that's how it looks in code:

`  
	//Slave 17
	//Register 32
	//Value 89
  	modbusBuildRequest06( &status, 17, 32, 89 );
`

`  
	//Here send status.request.frame content to slave
	//and get response into status.response.frame
`

`  
	modbusParseResponse( &status );
	//After successful parsing you should get:
	//status.data.address = 17
	//status.data.function = 6
	//status.data.index = 32
	//status.data.type = 1
	//status.data.count = 1
	//status.data.length = 2
	//status.data.regs[0] = 89
`

To see more examples, please take a look into examples folder.

### Tidying up
In order to free memory used by the received data and finish use of the library, **modbusMasterEnd** should be called on the status structure.

## NOTES
There are 4 macros responsible for describing Modbus data types (*status.data.type*). Please refer to the code sample below.

`  
	#define MODBUS_HOLDING_REGISTER 1
	#define MODBUS_INPUT_REGISTER 2
	#define MODBUS_COIL 4
	#define MODBUS_DISCRETE_INPUT 8
`

## SEE ALSO
modbusMasterInit(3lightmodbus), modbusMasterEnd(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
