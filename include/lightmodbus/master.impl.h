#include "master.h"
#include "master_func.h"

ModbusMasterFunctionHandler modbusMasterDefaultFunctions[] =
{
#ifdef LIGHTMODBUS_F01M
	{1, modbusParseResponse01020304},
#endif

#ifdef LIGHTMODBUS_F02M
	{2, modbusParseResponse01020304},
#endif

#ifdef LIGHTMODBUS_F03M
	{3, modbusParseResponse01020304},
#endif

#ifdef LIGHTMODBUS_F04M
	{4, modbusParseResponse01020304},
#endif

#ifdef LIGHTMODBUS_F05M
	{5, modbusParseResponse0506},
#endif

#ifdef LIGHTMODBUS_F06M
	{6, modbusParseResponse0506},
#endif

#ifdef LIGHTMODBUS_F15M
	{15, modbusParseResponse1516},
#endif

#ifdef LIGHTMODBUS_F16M
	{16, modbusParseResponse1516},
#endif

#ifdef LIGHTMODBUS_F22M
	{22, modbusParseResponse22},
#endif
};

ModbusMaster *modbusBeginRequestPDU(ModbusMaster *status)
{
	status->request.pduOffset = 0;
	status->request.padding = 0;
	return status;
}

LIGHTMODBUS_RET_ERROR modbusEndRequestPDU(ModbusMaster *status, ModbusError err)
{
	return err;
}

ModbusMaster *modbusBeginRequestRTU(ModbusMaster *status)
{
	status->request.pduOffset = 1;
	status->request.padding = 3;
	return status;
}

/**
	\param err Used for error propagation from modbusBuildRequestxx
	\returns MODBUS_ERROR_LENGTH if the allocated frame is too short 
*/
LIGHTMODBUS_RET_ERROR modbusEndRequestRTU(ModbusMaster *status, uint8_t address, ModbusError err)
{
	if (err) return err;
	if (status->request.length < 4) return MODBUS_ERROR_LENGTH;

	// Put in slave address
	status->request.data[0] = address;

	// Compute and put in CRC
	uint16_t crc = modbusCRC(&status->request.data[0], status->request.length - 2);
	modbusWLE(&status->request.data[status->request.length - 2], crc);

	return MODBUS_OK;
}

ModbusMaster *modbusBeginRequestTCP(ModbusMaster *status)
{
	status->request.pduOffset = 0;
	status->request.padding = 7;
	return status;
}

/**
	\param err Used for error propagation from modbusBuildRequestxx
	\returns MODBUS_ERROR_LENGTH if the allocated frame is too short 
*/
LIGHTMODBUS_RET_ERROR modbusEndRequestTCP(ModbusMaster *status, uint16_t transaction, uint8_t unit, ModbusError err)
{
	if (err) return err;
	if (status->request.length < 7) return MODBUS_ERROR_LENGTH;

	uint16_t length = status->request.length - 6;
	modbusWBE(&status->request.data[0], transaction); // Transaction ID
	modbusWBE(&status->request.data[2], 0);           // Protocol ID
	modbusWBE(&status->request.data[4], length);      // Data length
	status->request.data[6] = unit;                   // Unit ID

	return MODBUS_OK;
}