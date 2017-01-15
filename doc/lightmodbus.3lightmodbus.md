# lightmodbus 3lightmodbus "15 January 2017" "v1.3"

## NAME
**lightmodbus** - a lightweight, cross-platform Modbus RTU library.

## DESCRIPTION
The **lightmodbus** library allows communication with use of the Modbus RTU protocol. **lightmodbus** contains
functions for parsing and creating Modbus frames, but **it is not** capable of sending or receiving them.
Modbus functions supported by library include: 01, 02, 03, 04, 05, 06, 15 and 16.
Library itself, is easy to compile and modular - only necessary modules can be included while building. Default version available for
PC is complete and it contains all modules by default. Needless to say, the library is possible to build at any little-endian platform.

## BUILDING
There are three makefiles attached to the library.
Usual `makefile` simply compiles source code, creating object files (`obj` directory), as well as static library files in `lib` directory. Modules to be linked into the library can be specified by `MMODULES` and `SMODULES` variables passed to make from command line. Depending on their value, makefile compiles and links modules. Default settings are:

`   
	MMODULES = master-registers master-coils \
		master-discrete-inputs master-input-registers
	SMODULES = slave-registers slave-coils \
		slave-discrete-inputs slave-input-registers
`

`makefile-coverage` builds library all on its own for coverage testing purposes (you probably don't need that, go on).

## AVR
`makefile-avr` works exactly as normal `makefile`, but instead it uses `avr-gcc` compiler to build the library. Additionally, it requires MCU type to be specified by `MCU` variable passed from command line.

## ROUTINES
Full listing of functions included in `lightmodbus` library.

| Routine name                  | Module name            		 	            |
|-------------------------------|-----------------------------------------------|
| **modbusCRC**                 |  core                   						|
| **modbusSwapEndian**          |  core           								|
| **modbusMaskRead**            |  core               							|
| **modbusMaskWrite**           |  core              							|
| **modbusMasterInit**       	|  master-base          						|
| **modbusMasterEnd**       	|  master-base          						|
| **modbusParseResponse**       |  master-base          						|
| **modbusParseException**      |  master-base         							|
| **modbusSlaveInit**      		|  slave-base     		    					|
| **modbusSlaveEnd**     		|  slave-base     		    					|
| **modbusBuildException**      |  slave-base         							|
| **modbusParseRequest**   	   	|  slave-base         							|
| **modbusBuildRequest01**   	|  master-coils         						|
| **modbusBuildRequest02**   	|  master-discrete-inputs         				|
| **modbusBuildRequest03**   	|  master-registers         					|
| **modbusBuildRequest04**   	|  master-input-registers         				|
| **modbusBuildRequest05**   	|  master-coils         						|
| **modbusParseRequest06**   	|  master-registers         					|
| **modbusBuildRequest15**   	|  master-coils         						|
| **modbusBuildRequest16**   	|  master-registers         					|
| **modbusParseRequest01**   	|  slave-coils         							|
| **modbusParseRequest02**   	|  slave-discrete-inputs         				|
| **modbusParseRequest03**   	|  slave-registers         						|
| **modbusParseRequest04**   	|  slave-input-registers         				|
| **modbusParseRequest05**   	|  slave-coils         							|
| **modbusParseRequest06**   	|  slave-registers          					|
| **modbusParseRequest15**   	|  slave-coils         							|
| **modbusParseRequest16**   	|  slave-registers          					|
| **modbusParseResponse01**   	|  master-coils         						|
| **modbusParseResponse02**   	|  master-discrete-inputs         				|
| **modbusParseResponse03**   	|  master-registers         					|
| **modbusParseResponse04**   	|  master-input-registers         				|
| **modbusParseResponse05**   	|  master-coils         						|
| **modbusParseResponse06**   	|  master-registers        						|
| **modbusParseResponse15**   	|  master-coils         						|
| **modbusParseResponse16**   	|  master-registers         					|

| Routine name                  | Manpage                  		 	            |
|-------------------------------|-----------------------------------------------|
| **modbusCRC**                 |  modbusCRC( 3lightmodbus )                    |
| **modbusSwapEndian**          |  modbusSwapEndian( 3lightmodbus )             |
| **modbusMaskRead**            |  modbusMaskRead( 3lightmodbus )               |
| **modbusMaskWrite**           |  modbusMaskWrite( 3lightmodbus )              |
| **modbusMasterInit**       	|  modbusMasterInit( 3lightmodbus )          	|
| **modbusMasterEnd**       	|  modbusMasterEnd( 3lightmodbus )          	|
| **modbusParseResponse**       |  modbusParseResponse( 3lightmodbus )          |
| **modbusParseException**      |  modbusParseException( 3lightmodbus )         |
| **modbusSlaveInit**      		|  modbusSlaveInit( 3lightmodbus )     		    |
| **modbusSlaveEnd**     		|  modbusSlaveEnd( 3lightmodbus )     		    |
| **modbusBuildException**      |  modbusBuildException( 3lightmodbus )         |
| **modbusParseRequest**   	   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusBuildRequest01**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusBuildRequest02**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusBuildRequest03**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusBuildRequest04**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusBuildRequest05**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusBuildRequest06**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusBuildRequest15**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusBuildRequest16**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusParseRequest01**   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusParseRequest02**   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusParseRequest03**   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusParseRequest04**   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusParseRequest05**   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusParseRequest06**   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusParseRequest15**   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusParseRequest16**   	|  modbusParseRequest( 3lightmodbus )         	|
| **modbusParseResponse01**   	|  modbusParseResponse( 3lightmodbus )         	|
| **modbusParseResponse02**   	|  modbusParseResponse( 3lightmodbus )         	|
| **modbusParseResponse03**   	|  modbusParseResponse( 3lightmodbus )         	|
| **modbusParseResponse04**   	|  modbusParseResponse( 3lightmodbus )         	|
| **modbusParseResponse05**   	|  modbusParseResponse( 3lightmodbus )         	|
| **modbusParseResponse06**   	|  modbusParseResponse( 3lightmodbus )         	|
| **modbusParseResponse15**   	|  modbusParseResponse( 3lightmodbus )         	|
| **modbusParseResponse16**   	|  modbusParseResponse( 3lightmodbus )         	|

## MODBUS FUNCTION CODES
Modbus function codes meanings:

| Function 	| Description														|
|-----------|-------------------------------------------------------------------|
| 1			| read multiple coils												|
| 2			| read multiple discrete inputs										|
| 3			| read multiple holding registers									|
| 4			| read multiple input registers										|
| 5			| write single coil 												|
| 6			| write single holding register										|
| 15		| write multiple coils												|
| 16		| write multiple holding registers									|

## MODBUS EXCEPTIONS
Modbus protocol provides exception codes returned when master request fails. Some of the exceptions have their C macros defined in `lightmodbus/core.h`.

|  Macro                      | Exception | Description                           |
|-----------------------------|-----------|---------------------------------------|
| `MODBUS_EXCEP_ILLEGAL_FUNC` | 1         | illegal function code                 |
| `MODBUS_EXCEP_ILLEGAL_ADDR` | 2         | illegal data address                  |
| `MODBUS_EXCEP_ILLEGAL_VAL`  | 3         | illegal data value                    |
| `MODBUS_EXCEP_SLAVE_FAIL`   | 4         | slave device failure                  |
| `MODBUS_EXCEP_ACK`          | 5         | acknowledge                           |
|                             | 6         | slave device busy                     |
| `MODBUS_EXCEP_NACK`         | 7         | negative acknowledge                  |
|                             | 8         | memory parity error                   |

## RETURN VALUES
Every routine contained in the library, apart from **modbusSwapEndian**, **modbusMaskRead**, **modbusMaskWrite** and **modbusCRC**, returns an error code. Description of those can be found below.

Error code macros are defined in **lightmodbus/core.h**.

| Macro                    | Value | Description                                |
|--------------------------|-------|--------------------------------------------|
| `MODBUS_ERROR_OK`        | 0     | no error occurred, everything is fine      |
| `MODBUS_ERROR_EXCEPTION` | 1     | exception has been thrown or parsed        |
| `MODBUS_ERROR_PARSE`     | 2     | frame parsing error                        |
| `MODBUS_ERROR_CRC`       | 4     | frame CRC invalid (frame ignored)          |
| `MODBUS_ERROR_ALLOC`     | 8     | memory allocation error                    |
| `MODBUS_ERROR_OTHER`     | 16    | function has exited for other reason       |
| `MODBUS_ERROR_FRAME`     | 32    | frame contains incorrect data              |

`MODBUS_ERROR_OK` is returned when no error occurs.

`MODBUS_ERROR_EXCEPTION` is returned either when:
	- (on slave side) slave had to return exception frame
	- (on master side) exception frame has been parsed
	In both cases it's not an actual error, but an information for user.

`MODBUS_ERROR_PARSE` is returned when function code is not supported (library module missing?)

`MODBUS_ERROR_CRC` is returned when CRC attached to frame is invalid. Obviously, frame is ignored in such a case.

`MODBUS_ERROR_ALLOC` is returned when **malloc** or **realloc** call fails.

`MODBUS_ERROR_OTHER` is returned when e.g. user tries to parse frame of 0 length, slave has been initialized with address 0 and in other cases of passing invalid arguments to library routines.

`MODBUS_ERROR_FRAME` is returned by master-side parsing function, when error is found in given frame (e.g. byte count doesn't match register count)

## USAGE

### Master
To use master-side utilities `lightmodbus/master.h` header file is necessary.

Variable of type **ModbusMaster** is needed to contain Modbus configuration data. Later it is passed to every master-side routine called.

`ModbusMaster mstatus;`

Master module needs initiation with **modbusMasterInit**:

`modbusMasterInit( &mstatus );`

Parsing and receiving frames is performed following way:

`modbusBuildRequest06( &mstatus, 32, 3, 10 );`

Request frame will be stored in `mstatus.request.frame`. More on this topic can be found at modbusBuildRequest(3lightmodbus).

Parsing frames is performed following way:

`modbusParseResponse( &mstatus );`

Beforehand, a pointer to response frame from slave must be written into `mstatus.response.frame` and its length in bytes should be written to `mstatus.response.length`.
User mustn't change `mstatus.request` before response frame is parsed, because original request is needed to perform response parsing.


### Slave
To use slave-side utilities `lightmodbus/slave.h` header file is necessary.
Variable of type **ModbusSlave** is needed to contain Modbus configuration data. Later it has to be passed to every slave-side routine called.

`ModbusSlave sstatus;`

Basic example of **ModbusSlave** structure setup:

`  
	uint16_t regs[16];
	sstatus.registers = regs;
	sstatus.registerCount = 16;
	sstatus.address = 27; //Slave device address
`

Now **modbusSlaveInit** can be called.

`modbusSlaveInit( &sstatus );`

After such procedure, slave device is ready to work.
Received data has to be put in `sstatus.request.frame`, and its length in bytes has to be written to `sstatus.request.length` - then **modbusParseRequest** can be called, and finally frame contained in `sstatus.response` structure is ought to be send back to master.



## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
