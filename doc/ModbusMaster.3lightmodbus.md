# ModbusMaster 3lightmodbus "19 July 2017" "v1.3"

## NAME
**ModbusMaster** - data type containing all information about current master device status, its configuration and received data.

## SYNOPSIS
Please refer to **lightmodbus/master.h**.
**ModbusMaster** is equivalent of **struct modbusMaster**.

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
| `data.count` | number of received data units - **not bytes!** |
| `data.length` | length of received data in bytes |
| `data.type` | type of received data |
| `data.function` | function that returned the data |
| `data.coils` | received coils or discrete input values |
| `data.regs` | received (input or holding) register values |
| `exception.address` | address of device that threw the exception |
| `exception.function` | function that threw the exception |
| `exception.code` | exception code |

**Note:** In *status.data.coils* each **bit** corresponds to a **single** coil or a discrete input.

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
	//Assume:
	//Slave ID: 17
	//Register ID: 32
	//Register value: 89
  	modbusBuildRequest06( &status, 17, 32, 89 );
`

`  
	//Here send status.request.frame content to slave
	//and get response back into status.response.frame
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
In order to free memory (not always!) used by the received data and finish use of the library, **modbusMasterEnd** should be called on the status structure.

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
