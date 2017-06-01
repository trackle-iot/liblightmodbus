# lightmodbus 3lightmodbus "25 March 2017" "v1.3"

## NAME
**lightmodbus** - a lightweight, cross-platform Modbus RTU library.

## DESCRIPTION
The **lightmodbus** library allows communication with use of the Modbus RTU protocol. **lightmodbus** contains functions for parsing and creating Modbus frames, but **it is not** capable of sending or receiving them. Modbus functions supported by the library include: 01, 02, 03, 04, 05, 06, 15, 16 and 22.
Library itself is easy to compile and modular - only necessary modules can be included while building. Version available for PC through Debian packages is complete and contains all modules by default. Needless to say, the library is possible to build at any little-endian platform.

## BUILDING
There are three makefiles attached to the library.
Usual `makefile` simply compiles source code, leaving behind object files (`obj` directory), as well as static library files in `lib` directory.
Included modules, as well as other build settings **can** (by default, everything is built with no fixed buffer sizes) be adjusted with `./genconf.sh` script, which affects `include/lightmodbus/libconf.h` and `.modules.conf` files.
Use `./genconf.sh --help` to get more information.

Example build process can be:
`  
	./genconf.sh -s "" --mdat 256 --mres 256 --mreq 256
	make
`

Which effects in having only master side of the library built with *response.frame*, *request.frame* and *data.coils* (shared with *data.regs*) arrays in *ModbusMaster* structure of fixed 256b size (the highest reasonable value, because of Modbus 256b per frame limit)

`makefile-coverage` builds library on its own for coverage testing purposes (you probably don't need that, go on).

## AVR
`makefile-avr` bases on normal `makefile`, but instead it uses `avr-gcc` compiler to build the library. Additionally, it requires MCU type to be specified by `MCU` variable passed from command line.

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
| **modbusBuildRequest0102**   	|  master-coils         						|
| **modbusBuildRequest0304**   	|  master-registers         					|
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
| **modbusBuildRequest0102**   	|  modbusBuildRequest( 3lightmodbus )         	|
| **modbusBuildRequest0304**   	|  modbusBuildRequest( 3lightmodbus )         	|
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
The Modbus protocol provides exception codes returned when master request fails. Some of the exceptions have their C macros defined in **lightmodbus/core.h**.

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
Most of functions in the library return an error code. Description of those can be found below.

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

`MODBUS_ERROR_ALLOC` is returned when **malloc** or **realloc** call fails (or static buffer, set up during compilation process, is not long enough).

`MODBUS_ERROR_OTHER` can be returned when:
	- user attempts to parse frame of 0 length
	- user attempts to create frame that would be invalid (e.g. read to much data)
	- user attempts to throw exception 0
	- null pointer is passed as crucial function argument
	- in mask functions: read/write out of allowed range
	- slave is initialized with address 0
	- given frame was shorter than 4 bytes
	- sub-parser function somehow gets wrong frame
	- something goes really wrong inside the library itself

`MODBUS_ERROR_FRAME` is returned by master-side parsing function when error is contained in the Modbus frame (e.g. byte count doesn't match register count)

## EXAMPLES
Please refer to the **examples** folder.

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
