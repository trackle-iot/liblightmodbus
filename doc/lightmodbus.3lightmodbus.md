# lightmodbus 3lightmodbus "21 January 2017" "v1.3"

## NAME
**lightmodbus** - a lightweight, cross-platform Modbus RTU library.

## DESCRIPTION
The **lightmodbus** library allows communication with use of the Modbus RTU protocol. **lightmodbus** contains
functions for parsing and creating Modbus frames, but **it is not** capable of sending or receiving them.
Modbus functions supported by library include: 01, 02, 03, 04, 05, 06, 15, 16 and 22.
Library itself, is easy to compile and modular - only necessary modules can be included while building. Default version available for PC is complete and it contains all modules by default. Needless to say, the library is possible to build at any little-endian platform.

## BUILDING
There are three makefiles attached to the library.
Usual `makefile` simply compiles source code, creating object files (`obj` directory), as well as static library files in `lib` directory. Modules to be linked into the library can be specified by `MMODULES` and `SMODULES` variables passed to make from command line. Depending on their value, makefile compiles and links modules. Default settings are:

`   
	MMODULES = master-registers master-coils
	SMODULES = slave-registers slave-coils
`

Example make call would be: `make MMODULES="master-reisters" SMODULES=""`

`makefile-coverage` builds library all on its own for coverage testing purposes (you probably don't need that, go on).

## AVR
`makefile-avr` works exactly as normal `makefile`, but instead it uses `avr-gcc` compiler to build the library. Additionally, it requires MCU type to be specified by `MCU` variable passed from command line.

## ROUTINES
Full listing of routines included in `lightmodbus` library available for user.

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
| **modbusBuildRequest02**   	|  master-coils         						|
| **modbusBuildRequest03**   	|  master-registers         					|
| **modbusBuildRequest04**   	|  master-registers         					|
| **modbusBuildRequest05**   	|  master-coils         						|
| **modbusParseRequest06**   	|  master-registers         					|
| **modbusBuildRequest15**   	|  master-coils         						|
| **modbusBuildRequest16**   	|  master-registers         					|
| **modbusBuildRequest22**   	|  master-registers         					|


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
| **modbusBuildRequest22**   	|  modbusBuildRequest( 3lightmodbus )         	|


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
| 22		| mask write single holding register								|

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
Most of routines contained in library return an error code. Description of those can be found below.

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

## EXAMPLE OF USAGE


## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
