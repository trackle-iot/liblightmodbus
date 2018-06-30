#Basic stuff
cmake_minimum_required( VERSION 3.9 )
project( liblightmodbus VERSION 1.3 DESCRIPTION "liblightmodbus is a lightweight cross-platform Modbus RTU library." )

#Include dirs
include_directories( "${PROJECT_SOURCE_DIR}/include" )

#Configuration file
configure_file( 
	"${PROJECT_SOURCE_DIR}/include/lightmodbus/libconf.h.in"
	"${PROJECT_SOURCE_DIR}/include/lightmodbus/libconf.h"
	)

#Sources list
add_library( 
	liblightmodbus 
	"${PROJECT_SOURCE_DIR}/src/core.c" 
	"${PROJECT_SOURCE_DIR}/src/slave.c" 
	"${PROJECT_SOURCE_DIR}/src/master.c" 
	"${PROJECT_SOURCE_DIR}/src/slave/scoils.c"
	"${PROJECT_SOURCE_DIR}/src/slave/sregs.c"
	"${PROJECT_SOURCE_DIR}/src/master/mpcoils.c"
	"${PROJECT_SOURCE_DIR}/src/master/mbcoils.c"
	"${PROJECT_SOURCE_DIR}/src/master/mpregs.c"
	"${PROJECT_SOURCE_DIR}/src/master/mbregs.c"
	)

#Set library version
set_target_properties( liblightmodbus PROPERTIES VERSION ${PROJECT_VERSION} )

