# AVR Modbus RTU slave

This directory contains a fully-featured Modbus RTU slave implementation for
AVR microcontrollers (ATmega328p) utilizing around 3.7kB of the flash memory.

You can provide different configuration options to `make` in order to customize the build:
 - `MCU` - MCU type (default: `atmega328p`)
 - `F_CPU` - Clock speed (deafult: `8000000UL`)
 - `BAUD_RATE` - USART0 baudrate (default: `9600`)
 - `SLAVE_ADDRESS` - Address of the slave (default: `1`)
 - `REG_COUNT` - Number of registers (default: `32`)
 - `MAX_REQUEST` - Max request length (default: `64`)
 - `MAX_RESPONSE` - Max response length (default: `64`)

This implementation uses USART0 for communication with the master. You can use the `linuxmaster` program from the `examples` directory to interact with the slave.