# modbusMasterInit 3lightmodbus "29 July 2016" "v1.2"

## NAME
**modbusMasterInit** - setup **ModbusMaster** structure for use.

## SYNOPSIS
`#include <lightmodbus/master.h>`

`uint8_t modbusMasterInit( ModbusMaster *status );`

## DESCRIPTION
The **modbusMasterInit** function sets crucial members of **ModbusMaster** structure and allocates memory for necessary data. This function should always be called before using any other Modbus master-side functions.
It is also worth mentioning, that memory for *status.response* is **not** allocated (user should perform simple pointer assignment, not data copying).
Needless to say, when returned value is not equal 0 an error occured.

Memory can be later freed with **modbusMasterEnd**.

## SEE ALSO
ModbusMaster(3lightmodbus), modbusMasterEnd(3lightmodbus)

## AUTHORS
Jacek Wieczorek (Jacajack) - mrjjot@gmail.com
