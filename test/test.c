/*
This is really simple test suite, it covers ~95% of library code
AKA. The Worst Test File Ever
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include <lightmodbus/core.h>
#include <lightmodbus/master.h>
#include <lightmodbus/slave.h>
#include <lightmodbus/addons/examine.h>

#define DUMPMF( ) printf( "Dump the frame:\n\t" ); for ( i = 0; i < mstatus.request.length; i++ ) printf( "%.2x%s", mstatus.request.frame[i], ( i == mstatus.request.length - 1 ) ? "\n" : "-" );
#define DUMPSF( ) printf( "Dump response - length = %d:\n\t", sstatus.response.length ); for ( i = 0; i < sstatus.response.length; i++ ) printf( "%x%s", sstatus.response.frame[i], ( i == sstatus.response.length - 1 ) ? "\n" : ", " );

ModbusMaster mstatus;
struct modbusSlave sstatus; //This can be done this way as well
uint16_t registers[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
uint8_t coils[4] = { 0, 0, 0, 0 };
uint8_t discreteInputs[2] = { 255, 0 };
uint16_t inputRegisters[4] = { 1, 2, 3, 4 };
uint16_t defaults[512] = { 0xdead, 0xface, 0x1570, 0x01 };
uint16_t TestValues[512] = { 0xface, 0xdead, 0xCC, 0xDD, 0xEE, 0xFF, 0xAAFF, 0xBBFF };
uint16_t TestValues2[512] = {0};
uint8_t TestValues3[512] = { 0b11001100, 0x00 };
uint8_t ec;

void TermRGB( unsigned char R, unsigned char G, unsigned char B )
{
	if ( R > 5u || G > 5u || B > 5u ) return;
	printf( "\033[38;5;%dm", 16 + B + G * 6 + R * 36 );
}

void examinedump( struct modbusFrameInfo info )
{
	int i;
	
	printf( "Frame examine:\n"\
			"\tdirection: %s (%d)\n"\
 			"\taddress: %d\n"\
			"\tfunction: %d\n"\
			"\texception: %d\n"\
			"\ttype: %s (%d)\n"\
			"\tindex: %d\n"\
			"\tcount: %d\n"\
			"\taccess: %s (%d)\n"\
			"\tcrc: %04x\n"\
			"\tandmask: %04x\n"\
			"\tormask: %04x\n"\
			"\tdataptr: %p\n"\
			"\tdatalen: %d\n"\
			"\tdata: ",\

			info.direction == MODBUS_EXAMINE_REQUEST ? "request" : ( info.direction == MODBUS_EXAMINE_RESPONSE ? "response" : "bad" ),\
			info.direction,\
			info.address,\
			info.function,\
			info.exception,\
			info.type  == MODBUS_HOLDING_REGISTER ? "reg" : ( info.type == MODBUS_COIL ? "coil" : ( info.type == MODBUS_DISCRETE_INPUT ? "discrete" : ( info.type == MODBUS_INPUT_REGISTER ? "input reg" : "bad"))),\
			info.type,\
			info.index,\
			info.count,\
			info.access == MODBUS_EXAMINE_READ ? "read" : ( info.access == MODBUS_EXAMINE_WRITE ? "write" : "bad" ),\
			info.access,\
			info.crc,\
			info.andmask,\
			info.ormask,\
			info.data,\
			info.length\
		);

	if ( info.data != NULL )
	{
		for ( i = 0; i < info.length; i++ )
		{
			printf( "%02x ", ((uint8_t*)info.data)[i] );
		}
	}
	printf( "\n" );
}

void examinem( )
{
	struct modbusFrameInfo info;
	uint8_t err = modbusExamine( &info, MODBUS_EXAMINE_REQUEST, mstatus.request.frame, mstatus.request.length );
	if ( err == MODBUS_OK || err == MODBUS_ERROR_EXCEPTION ) examinedump( info );
	else printf( "request frame examination error: %d\n", err );
}

void examines( )
{
	struct modbusFrameInfo info;
	uint8_t err = modbusExamine( &info, MODBUS_EXAMINE_RESPONSE, sstatus.response.frame, sstatus.response.length );
	if ( err == MODBUS_OK || err == MODBUS_ERROR_EXCEPTION ) examinedump( info );
	else printf( "response frame examination error: %d\n", err );
}

void maxlentest( )
{
	#define CK2( n ) printf( "mec=%d, sec=%d\n", mec, sec ); printf( memcmp( mstatus.data.regs, bak, mstatus.data.length ) ? "ERROR!\n" : "OK\n" );
	#define CK( n ) printf( "mec=%d, sec=%d\n", mec, sec ); printf( memcmp( vals, bak, n ) ? "ERROR!\n" : "OK\n" );
	#define GEN( n ) for ( i = 0; i < n; i++ ) bak[i] = rand( );
	uint8_t vals[250];
	uint8_t bak[250];
	uint8_t i = 0;
	uint8_t sec, mec;

	printf( "\n-------Checking max-size operations--------\n" );
	sstatus.coils = vals;
	sstatus.coilCount = 2000;
	sstatus.coilMaskLength = 0;
	sstatus.discreteInputs = bak;
	sstatus.discreteInputCount = 2000;
	sstatus.coilMaskLength = 0;
	sstatus.registers = (uint16_t*) vals;
	sstatus.registerCount = 125;
	sstatus.inputRegisters = (uint16_t*) bak;
	sstatus.inputRegisterCount = 125;
	sstatus.registerMaskLength = 0;

	GEN( 246 );
	mec = modbusBuildRequest15( &mstatus, 0x20, 0, 1968, bak );
	#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST
		memcpy( sstatus.request.frame, mstatus.request.frame, mstatus.request.length );
	#else
		sstatus.request.frame = mstatus.request.frame;
	#endif
	sstatus.request.length = mstatus.request.length;
	sec = modbusParseRequest( &sstatus );
	CK( 246 );

	GEN( 246 );
	mec = modbusBuildRequest16( &mstatus, 0x20, 0, 123, (uint16_t*)bak );
	#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST
		memcpy( sstatus.request.frame, mstatus.request.frame, mstatus.request.length );
	#else
		sstatus.request.frame = mstatus.request.frame;
	#endif
	sstatus.request.length = mstatus.request.length;
	sec = modbusParseRequest( &sstatus );
	CK( 246 );

	sstatus.coils = bak;
	sstatus.registers = (uint16_t*) bak;

	GEN( 250 );
	mec = modbusBuildRequest03( &mstatus, 0x20, 0, 125 );
	#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST
		memcpy( sstatus.request.frame, mstatus.request.frame, mstatus.request.length );
	#else
		sstatus.request.frame = mstatus.request.frame;
	#endif
	sstatus.request.length = mstatus.request.length;
	sec = modbusParseRequest( &sstatus );

	#ifdef LIGHTMODBUS_STATIC_MEM_MASTER_RESPONSE
		memcpy( mstatus.response.frame, sstatus.response.frame, sstatus.response.length );
	#else
		mstatus.response.frame = sstatus.response.frame;
	#endif
	mstatus.response.length = sstatus.response.length;
	mec = modbusParseResponse( &mstatus );
	CK2( 250 );

	GEN( 250 );
	mec = modbusBuildRequest04( &mstatus, 0x20, 0, 125 );
	#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST
		memcpy( sstatus.request.frame, mstatus.request.frame, mstatus.request.length );
	#else
		sstatus.request.frame = mstatus.request.frame;
	#endif
	sstatus.request.length = mstatus.request.length;
	sec = modbusParseRequest( &sstatus );
	#ifdef LIGHTMODBUS_STATIC_MEM_MASTER_RESPONSE
		memcpy( mstatus.response.frame, sstatus.response.frame, sstatus.response.length );
	#else
		mstatus.response.frame = sstatus.response.frame;
	#endif
	mstatus.response.length = sstatus.response.length;
	mec = modbusParseResponse( &mstatus );
	CK2( 250 );

	GEN( 250 );
	mec = modbusBuildRequest01( &mstatus, 0x20, 0, 2000 );
	#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST
		memcpy( sstatus.request.frame, mstatus.request.frame, mstatus.request.length );
	#else
		sstatus.request.frame = mstatus.request.frame;
	#endif
	sstatus.request.length = mstatus.request.length;
	sec = modbusParseRequest( &sstatus );
	DUMPSF( );
	#ifdef LIGHTMODBUS_STATIC_MEM_MASTER_RESPONSE
		memcpy( mstatus.response.frame, sstatus.response.frame, sstatus.response.length );
	#else
		mstatus.response.frame = sstatus.response.frame;
	#endif
	mstatus.response.length = sstatus.response.length;
	mec = modbusParseResponse( &mstatus );
	CK2( 250 );

	GEN( 250 );
	mec = modbusBuildRequest02( &mstatus, 0x20, 0, 2000 );
	#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST
		memcpy( sstatus.request.frame, mstatus.request.frame, mstatus.request.length );
	#else
		sstatus.request.frame = mstatus.request.frame;
	#endif
	sstatus.request.length = mstatus.request.length;
	sec = modbusParseRequest( &sstatus );
	#ifdef LIGHTMODBUS_STATIC_MEM_MASTER_RESPONSE
		memcpy( mstatus.response.frame, sstatus.response.frame, sstatus.response.length );
	#else
		mstatus.response.frame = sstatus.response.frame;
	#endif
	mstatus.response.length = sstatus.response.length;
	mec = modbusParseResponse( &mstatus );
	CK2( 250 );

}

void Test( )
{
	uint8_t i = 0;
	uint8_t SlaveError, MasterError;
	uint8_t mok, sok;

	//Clear registers
	//memset( registers, 0, 8 * 2 );
	//memset( inputRegisters, 0, 4 * 2 );
	memset( coils, 0, 4 );
	memset( discreteInputs, 0, 2 );
	memcpy( registers, defaults, 16 );
	memcpy( inputRegisters, defaults, 4 * 2 );
	//memcpy( coils, defaults, 4 );
	//memcpy( discreteInputs, defaults, 2 );

	//Start test
	//TermRGB( 4, 2, 0 );
	printf( "Test started...\n" );

	//Dump frame
	//TermRGB( 2, 4, 0 );
	printf( "Dump the frame:\n\t" );
	for ( i = 0; i < mstatus.request.length; i++ )
		printf( "%.2x%s", mstatus.request.frame[i], ( i == mstatus.request.length - 1 ) ? "\n" : "-" );

	//Dump slave registers
	//TermRGB( 2, 4, 2 );
	printf( "Dump registers:\n\t" );
	for ( i = 0; i < sstatus.registerCount; i++ )
		printf( "0x%.2x%s", registers[i], ( i == sstatus.registerCount - 1 ) ? "\n" : ", " );

	printf( "Dump input registers:\n\t" );
	for ( i = 0; i < sstatus.inputRegisterCount; i++ )
		printf( "0x%.2x%s", inputRegisters[i], ( i == sstatus.inputRegisterCount - 1 ) ? "\n" : ", " );

	printf( "Dump coils:\n\t" );
	for ( i = 0; i < sstatus.coilCount; i++ )
		printf( "%d%s", modbusMaskRead( coils, sstatus.coilCount, i ), ( i == sstatus.coilCount - 1 ) ? "\n" : ", " );

	printf( "Dump discrete inputs:\n\t" );
	for ( i = 0; i < sstatus.discreteInputCount; i++ )
		printf( "%d%s", modbusMaskRead( discreteInputs, sstatus.discreteInputCount, i ), ( i == sstatus.discreteInputCount - 1 ) ? "\n" : ", " );

	//Parse request

	uint8_t f1[256];
	uint8_t l = mstatus.request.length;
	memcpy( f1, mstatus.request.frame, l );

	printf( "Let slave parse frame...\n" );
	#ifdef LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST
		memcpy( sstatus.request.frame, mstatus.request.frame, mstatus.request.length );
	#else
		sstatus.request.frame = mstatus.request.frame;
	#endif
	sstatus.request.length = mstatus.request.length;
	sok = SlaveError = modbusParseRequest( &sstatus );
	printf( "\tError - %d\n\tFinished - %d\n", SlaveError, 1 );

	if ( memcmp( f1, mstatus.request.frame, l ) ) printf( "!!!Slave has malformed the frame!!!\n" );

	//Dump slave registers
	printf( "Dump registers:\n\t" );
	for ( i = 0; i < sstatus.registerCount; i++ )
		printf( "0x%.2x%s", registers[i], ( i == sstatus.registerCount - 1 ) ? "\n" : ", " );

	printf( "Dump input registers:\n\t" );
	for ( i = 0; i < sstatus.inputRegisterCount; i++ )
		printf( "0x%.2x%s", inputRegisters[i], ( i == sstatus.inputRegisterCount - 1 ) ? "\n" : ", " );

	printf( "Dump coils:\n\t" );
	for ( i = 0; i < sstatus.coilCount; i++ )
		printf( "%d%s", modbusMaskRead( coils, sstatus.coilCount, i ), ( i == sstatus.coilCount - 1 ) ? "\n" : ", " );

	printf( "Dump discrete inputs:\n\t" );
	for ( i = 0; i < sstatus.discreteInputCount; i++ )
		printf( "%d%s", modbusMaskRead( discreteInputs, sstatus.discreteInputCount, i ), ( i == sstatus.discreteInputCount - 1 ) ? "\n" : ", " );

	//Dump response
	printf( "Dump response - length = %d:\n\t", sstatus.response.length );
	for ( i = 0; i < sstatus.response.length; i++ )
		printf( "%x%s", sstatus.response.frame[i], ( i == sstatus.response.length - 1 ) ? "\n" : ", " );

	//Process response
	l = sstatus.response.length;
	memcpy( f1, sstatus.response.frame, l );
	printf( "Let master process response...\n" );
	#ifdef LIGHTMODBUS_STATIC_MEM_MASTER_RESPONSE
		memcpy( mstatus.response.frame, sstatus.response.frame, sstatus.response.length );
	#else
		mstatus.response.frame = sstatus.response.frame;
	#endif
	mstatus.response.length = sstatus.response.length;
	mok = MasterError = modbusParseResponse( &mstatus );
	if ( !SlaveError && mstatus.predictedResponseLength != sstatus.response.length && sstatus.response.length )
		printf( "Response prediction doesn't match!! (p. %d vs a. %d)\n", mstatus.predictedResponseLength, \
			sstatus.response.length );

	if ( memcmp( f1, sstatus.response.frame, l ) ) printf( "!!!Master has malformed the frame!!!\n" );

	//Dump parsed data
	printf( "\tError - %d\n\tFinished - 1\n", MasterError );

	if ( mstatus.data.length )
		for ( i = 0; i < mstatus.data.count; i++ )
		{
			printf( "\t - { addr: 0x%x, type: 0x%x, reg: 0x%x, val: 0x%x }\n", mstatus.data.address, mstatus.data.type, mstatus.data.index + i,\
			( mstatus.data.type == MODBUS_HOLDING_REGISTER || mstatus.data.type == MODBUS_INPUT_REGISTER ) ? mstatus.data.regs[i] : \
			modbusMaskRead( mstatus.data.coils, mstatus.data.length, i ) );
		}

	if ( MasterError == MODBUS_ERROR_EXCEPTION )
	{
		//TermRGB( 4, 1, 0 );
		printf( "\t - ex addr: 0x%x, fun: 0x%x, code: 0x%x\n\r", mstatus.exception.address, mstatus.exception.function, mstatus.exception.code );
	}

	if ( mok == MODBUS_OK ) examinem( );
	if ( sok == MODBUS_OK || sok == MODBUS_ERROR_EXCEPTION ) examines( );

	printf( "----------------------------------------\n\n" );
}

void libinit( )
{
	//Init slave and master
	sstatus.registers = registers;
	sstatus.registerCount = 8;

	sstatus.coils = coils;
	sstatus.coilCount = 32;

	sstatus.discreteInputs = discreteInputs;
	sstatus.discreteInputCount = 16;

	sstatus.inputRegisters = inputRegisters;
	sstatus.inputRegisterCount = 4;
	sstatus.address = 32;

	printf( "slave init - %d\n", modbusSlaveInit( &sstatus ) );
	printf( "master init - %d\n\n\n", modbusMasterInit( &mstatus ) );
}

void MainTest( )
{
	//request03 - ok
	printf( "\t\t03 - correct request...\n" );
	modbusBuildRequest03( &mstatus, 0x20, 0x00, 0x08 );
	Test( );

	//request03 - bad CRC
	printf( "\t\t03 - bad CRC...\n" );
	modbusBuildRequest03( &mstatus, 0x20, 0x00, 0x08 );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request03 - bad first register
	printf( "\t\t03 - bad first register...\n" );
	modbusBuildRequest03( &mstatus,0x20, 0xff, 0x08 );
	Test( );

	//request03 - bad register count
	printf( "\t\t03 - bad register count...\n" );
	modbusBuildRequest03( &mstatus, 0x20, 0x00, 65 );
	Test( );

	//request03 - bad register count and first register
	printf( "\t\t03 - bad register count and first register...\n" );
	modbusBuildRequest03( &mstatus,0x20, 9, 32 );
	Test( );

	//request03 - broadcast
	printf( "\t\t03 - broadcast...\n" );
	modbusBuildRequest03( &mstatus,0x00, 0x00, 0x08 );
	Test( );

	//request03 - other slave address
	printf( "\t\t03 - other address...\n" );
	modbusBuildRequest03( &mstatus,0x10, 0x00, 0x08 );
	Test( );

	//request06 - ok
	printf( "\t\t06 - correct request...\n" );
	modbusBuildRequest06( &mstatus,0x20, 0x06, 0xface );
	Test( );

	//request06 - bad CRC
	printf( "\t\t06 - bad CRC...\n" );
	modbusBuildRequest06( &mstatus,0x20, 0x06, 0xf6 );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request06 - bad register
	printf( "\t\t06 - bad register...\n" );
	modbusBuildRequest06( &mstatus,0x20, 0xf6, 0xf6 );
	Test( );

	//request06 - broadcast
	printf( "\t\t06 - broadcast...\n" );
	modbusBuildRequest06( &mstatus, 0x00, 0x06, 0xf6 );
	Test( );

	//request06 - other slave address
	printf( "\t\t06 - other address...\n" );
	modbusBuildRequest06( &mstatus, 0x10, 0x06, 0xf6 );
	Test( );

	//request16 - ok
	printf( "\t\t16 - correct request...\n" );
	modbusBuildRequest16( &mstatus, 0x20, 0x00, 0x04, TestValues );
	Test( );

	//request16 - bad CRC
	printf( "\t\t16 - bad CRC...\n" );
	modbusBuildRequest16( &mstatus, 0x20, 0x00, 0x04, TestValues );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request16 - bad start reg
	printf( "\t\t16 - bad first register...\n" );
	modbusBuildRequest16( &mstatus, 0x20, 0xFF, 0x04, TestValues );
	Test( );

	//request16 - bad register range
	printf( "\t\t16 - bad register range...\n" );
	modbusBuildRequest16( &mstatus, 0x20, 0x00, 65, TestValues2 );
	Test( );

	//request16 - bad register range 2
	printf( "\t\t16 - bad register range 2...\n" );
	modbusBuildRequest16( &mstatus, 0x20, 0x00, 0x20, TestValues2 );
	Test( );

	//request16 - confusing register range
	printf( "\t\t16 - confusing register range...\n" );
	modbusBuildRequest16( &mstatus, 0x20, 0x00, 0x08, TestValues );
	Test( );

	//request16 - confusing register range 2
	printf( "\t\t16 - confusing register range 2...\n" );
	modbusBuildRequest16( &mstatus, 0x20, 0x01, 0x08, TestValues );
	Test( );

	//request16 - broadcast
	printf( "\t\t16 - broadcast...\n" );
	modbusBuildRequest16( &mstatus, 0x00, 0x00, 0x04, TestValues );
	Test( );

	//request16 - other slave address
	printf( "\t\t16 - other address...\n" );
	modbusBuildRequest16( &mstatus, 0x10, 0x00, 0x04, TestValues );
	Test( );

	//request02 - ok
	printf( "\t\t02 - correct request...\n" );
	modbusBuildRequest02( &mstatus, 0x20, 0x00, 0x10 );
	Test( );

	//request02 - bad CRC
	printf( "\t\t02 - bad CRC...\n" );
	modbusBuildRequest02( &mstatus, 0x20, 0x00, 0x10 );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request02 - bad first discrete input
	printf( "\t\t02 - bad first discrete input...\n" );
	modbusBuildRequest02( &mstatus, 0x20, 0xff, 0x10 );
	Test( );

	//request02 - bad discrete input count
	printf( "\t\t02 - bad discrete input count...\n" );
	modbusBuildRequest02( &mstatus, 0x20, 0x00, 0xff );
	Test( );

	//request02 - bad register count and first discrete input
	printf( "\t\t02 - bad register count and first discrete input...\n" );
	modbusBuildRequest02( &mstatus, 0x20, 0xff, 0xff );
	Test( );

	//request02 - broadcast
	printf( "\t\t02 - broadcast...\n" );
	modbusBuildRequest02( &mstatus, 0x00, 0x00, 0x10 );
	Test( );

	//request02 - other slave address
	printf( "\t\t02 - other address...\n" );
	modbusBuildRequest02( &mstatus, 0x10, 0x00, 0x10 );
	Test( );

	printf( "\nREINIT OF LIBRARY\n" );
	modbusSlaveEnd( &sstatus );
	modbusMasterEnd( &mstatus );
	libinit( );

	//request01 - ok
	printf( "\t\t01 - correct request...\n" );
	modbusBuildRequest01( &mstatus, 0x20, 0x00, 0x04 );
	Test( );

	//request01 - bad CRC
	printf( "\t\t01 - bad CRC...\n" );
	modbusBuildRequest01( &mstatus, 0x20, 0x00, 0x04 );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request01 - bad first register
	printf( "\t\t01 - bad first coil...\n" );
	modbusBuildRequest01( &mstatus, 0x20, 0xff, 0x04 );
	Test( );

	//request01 - bad register count
	printf( "\t\t01 - bad coil count...\n" );
	modbusBuildRequest01( &mstatus, 0x20, 0x00, 0xff );
	Test( );

	//request01 - bad register count and first register
	printf( "\t\t01 - bad coil count and first coil...\n" );
	modbusBuildRequest01( &mstatus, 0x20, 0xffff, 1250 );
	Test( );

	//request01 - broadcast
	printf( "\t\t01 - broadcast...\n" );
	modbusBuildRequest01( &mstatus, 0x00, 0x00, 0x04 );
	Test( );

	//request01 - other slave address
	printf( "\t\t01 - other address...\n" );
	modbusBuildRequest01( &mstatus, 0x10, 0x00, 0x04 );
	Test( );

	//request05 - ok
	printf( "\t\t05 - correct request...\n" );
	modbusBuildRequest05( &mstatus, 0x20, 0x03, 0xff00 );
	Test( );

	//request05 - bad CRC
	printf( "\t\t05 - bad CRC...\n" );
	modbusBuildRequest05( &mstatus, 0x20, 0x03, 0xff00 );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request05 - ok
	printf( "\t\t05 - correct request, non 0xff00 number...\n" );
	modbusBuildRequest05( &mstatus, 0x20, 0x03, 1 );
	Test( );

	//request05 - bad register
	printf( "\t\t05 - bad coil...\n" );
	modbusBuildRequest05( &mstatus, 0x20, 0xf3, 0xff00 );
	Test( );

	//request05 - broadcast
	printf( "\t\t05 - broadcast...\n" );
	modbusBuildRequest05( &mstatus, 0x00, 0x03, 0xff00 );
	Test( );

	//request05 - other slave address
	printf( "\t\t05 - other address...\n" );
	modbusBuildRequest05( &mstatus, 0x10, 0x03, 0xff00 );
	Test( );

	//request15 - ok
	printf( "\t\t15 - correct request...\n" );
	modbusBuildRequest15( &mstatus, 0x20, 0x00, 0x04, TestValues3 );
	Test( );

	//request15 - bad CRC
	printf( "\t\t15 - bad CRC...\n" );
	modbusBuildRequest15( &mstatus, 0x20, 0x00, 0x04, TestValues3 );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request15 - bad start reg
	printf( "\t\t15 - bad first coil...\n" );
	modbusBuildRequest15( &mstatus, 0x20, 0xFF, 0x04, TestValues3 );
	Test( );

	//request15 - bad register range
	printf( "\t\t15 - bad coil range...\n" );
	modbusBuildRequest15( &mstatus, 0x20, 0x00, 65, TestValues3 );
	Test( );

	//request15 - bad register range 2
	printf( "\t\t15 - bad coil range 2...\n" );
	modbusBuildRequest15( &mstatus, 0x20, 0x00, 0x20, TestValues3 );
	Test( );

	//request15 - confusing register range
	printf( "\t\t15 - confusing coil range...\n" );
	modbusBuildRequest15( &mstatus, 0x20, 0x00, 0x40, TestValues3 );
	Test( );

	//request15 - confusing register range 2
	printf( "\t\t15 - confusing coil range 2...\n" );
	modbusBuildRequest15( &mstatus, 0x20, 0x01, 0x40, TestValues3 );
	Test( );

	//request15 - broadcast
	printf( "\t\t15 - broadcast...\n" );
	modbusBuildRequest15( &mstatus, 0x00, 0x00, 0x04, TestValues3 );
	Test( );

	//request15 - other slave address
	printf( "\t\t15 - other address...\n" );
	modbusBuildRequest15( &mstatus, 0x10, 0x00, 0x04, TestValues3 );
	Test( );

	//request04 - ok
	printf( "\t\t04 - correct request...\n" );
	modbusBuildRequest04( &mstatus, 0x20, 0x00, 0x04 );
	Test( );

	//request04 - bad CRC
	printf( "\t\t04 - bad CRC...\n" );
	modbusBuildRequest04( &mstatus, 0x20, 0x00, 0x04 );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request04 - bad first register
	printf( "\t\t04 - bad first register...\n" );
	modbusBuildRequest04( &mstatus, 0x20, 0x05, 0x04 );
	Test( );

	//request04 - bad register count
	printf( "\t\t04 - bad register count...\n" );
	modbusBuildRequest04( &mstatus, 0x20, 0x00, 0x05 );
	Test( );

	//request04 - bad register count and first register
	printf( "\t\t04 - bad register count and first register...\n" );
	modbusBuildRequest04( &mstatus, 0x20, 0x01, 0x05 );
	Test( );

	//request04 - broadcast
	printf( "\t\t04 - broadcast...\n" );
	modbusBuildRequest04( &mstatus, 0x00, 0x00, 0x04 );
	Test( );

	//request04 - other slave address
	printf( "\t\t04 - other address...\n" );
	modbusBuildRequest04( &mstatus, 0x10, 0x00, 0x04 );
	Test( );

	//request06 - ok
	printf( "\t\t22 - correct request...\n" );
	modbusBuildRequest22( &mstatus,0x20, 0x06, 14 << 8, 56 << 8 );
	Test( );

	//request06 - bad CRC
	printf( "\t\t22 - bad CRC...\n" );
	modbusBuildRequest22( &mstatus,0x20, 0x06, 14 << 8, 56 << 8 );
	mstatus.request.frame[mstatus.request.length - 1]++;
	Test( );

	//request06 - bad register
	printf( "\t\t22 - bad register...\n" );
	modbusBuildRequest22( &mstatus,0x20, 0xf6, 14 << 8, 56 << 8 );
	Test( );

	//request06 - broadcast
	printf( "\t\t22 - broadcast...\n" );
	modbusBuildRequest22( &mstatus, 0x00, 0x06, 14 << 8, 56 << 8 );
	Test( );

	//request06 - other slave address
	printf( "\t\t22 - other address...\n" );
	modbusBuildRequest22( &mstatus, 0x10, 0x06, 14 << 8, 56 << 8 );
	Test( );

	//WRITE PROTECTION TEST
	printf( "\t\t--Register write protection test--\n" );
	uint8_t mask[1] = { 0 };
	sstatus.registerMask = mask;
	sstatus.registerMaskLength = 1;

	modbusMaskWrite( mask, 1, 2, 1 );

	modbusBuildRequest06( &mstatus, 0x20, 2, 16 );
	Test( );
	modbusBuildRequest06( &mstatus, 0x20, 0, 16 );
	Test( );
	modbusBuildRequest22( &mstatus,0x20, 0x02, 14 << 8, 56 << 8 );
	Test( );

	modbusBuildRequest16( &mstatus, 0x20, 0, 4, TestValues2 );
	Test( );
	modbusBuildRequest16( &mstatus, 0x20, 0, 2, TestValues2 );
	Test( );
	modbusBuildRequest22( &mstatus,0x20, 0x00, 14 << 8, 56 << 8 );
	Test( );

	//WRITE PROTECTION TEST 2
	printf( "\t\t--Coil write protection test--\n" );
	sstatus.coilMask = mask;
	sstatus.coilMaskLength = 1;

	modbusMaskWrite( mask, 1, 2, 1 );

	modbusBuildRequest05( &mstatus, 0x20, 2, 16 );
	Test( );
	modbusBuildRequest05( &mstatus, 0x20, 0, 16 );
	Test( );

	modbusBuildRequest15( &mstatus, 0x20, 0, 16, TestValues3 );
	Test( );
	modbusBuildRequest15( &mstatus, 0x20, 0, 2, TestValues3 );
	Test( );

	printf( "Bitval: %d\r\n", modbusMaskRead( mask, 1, 0 ) );
	printf( "Bitval: %d\r\n", modbusMaskRead( mask, 1, 1 ) );
	printf( "Bitval: %d\r\n", modbusMaskRead( mask, 1, 2 ) );
	printf( "Bitval: %d\r\n", modbusMaskRead( mask, 1, 3 ) );
	printf( "Bitval: %d\r\n", modbusMaskRead( mask, 1, 4 ) );

	sstatus.registerMaskLength = 0;
}


int main( )
{
	printf( "\n\t\t======LIBLIGHTMODBUS LIBRARY COVERAGE TEST LOG======" );
	printf( "\n\t\t======LIBLIGHTMODBUS VERSION: %s\n\n\n", LIGHTMODBUS_VERSION );
	memset( TestValues2, 0xAA, 1024 );
	libinit( );
	MainTest( );
	maxlentest( );

	modbusSlaveEnd( &sstatus );
	modbusMasterEnd( &mstatus );

	//Reset terminal colors
	printf( "\t\t" );

	return 0;
}
