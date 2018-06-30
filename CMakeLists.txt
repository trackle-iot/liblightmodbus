#Basic stuff
cmake_minimum_required( VERSION 3.3 )
project( liblightmodbus VERSION 1.3 DESCRIPTION "liblightmodbus is a lightweight cross-platform Modbus RTU library." )

#Get git version (LIGHTMODBUS_GIT_VERSION)
execute_process( COMMAND "git" "describe" "--abbrev=6" "--dirty" "--always" "--tag" RESULT_VARIABLE GIT_EXIT_CODE OUTPUT_VARIABLE PROJECT_GIT_VERSION )
if ( GIT_EXIT_CODE )
	set( PROJECT_GIT_VERSION "no-vcs-found" )
endif( )
string( REGEX REPLACE "\n$" "" PROJECT_GIT_VERSION "${PROJECT_GIT_VERSION}" ) #Strip newline

#Include dirs
include_directories( "${PROJECT_SOURCE_DIR}/include" )

#Sources list
add_library( 
	liblightmodbus 
	"${PROJECT_SOURCE_DIR}/src/lightmodbus.c" 
	"${PROJECT_SOURCE_DIR}/src/slave.c" 
	"${PROJECT_SOURCE_DIR}/src/master.c" 
	"${PROJECT_SOURCE_DIR}/src/slave/scoils.c"
	"${PROJECT_SOURCE_DIR}/src/slave/sregs.c"
	"${PROJECT_SOURCE_DIR}/src/master/mpcoils.c"
	"${PROJECT_SOURCE_DIR}/src/master/mbcoils.c"
	"${PROJECT_SOURCE_DIR}/src/master/mpregs.c"
	"${PROJECT_SOURCE_DIR}/src/master/mbregs.c"
	)

#List of all modules that can be enabled
set( AVAILABLE_MODULES 
	#Slave functions
	"F01S"
	"F02S"
	"F03S"
	"F04S"
	"F05S"
	"F06S"
	"F15S"
	"F16S"
	"F22S"
	
	#Master functions
	"F01M"
	"F02M"
	"F03M"
	"F04M"
	"F05M"
	"F06M"
	"F15M"
	"F16M"
	"F22M"

	#Other modules
	"ADDON_EXAMINE"
	"USER_FUNCTIONS"
)

#If module set is not specified, include all known modules
if ( NOT DEFINED MODULES )
	set( MODULES ${AVAILABLE_MODULES} )
	message( WARNING "MODULES not set. Enabling all by default.")
endif( )

#Iterate through modules list
set( MODULES_DEFINES "" )
foreach( MODULE ${MODULES} )
	#If that module doesn't exit raise an error
	if ( NOT ${MODULE} IN_LIST AVAILABLE_MODULES )
		message( FATAL_ERROR "`${MODULE}' is not a valid module name. Available modules are: `${AVAILABLE_MODULES}'")
	endif( )

	#Generate proper #define directive
	set( MODULES_DEFINES "${MODULES_DEFINES}#define LIGHTMODBUS_${MODULE}\n" )
	message( STATUS "Enabling module ${MODULE}" )
endforeach( )

#Emit configuration file
configure_file( 
	"${PROJECT_SOURCE_DIR}/include/lightmodbus/libconf.h.in"
	"${PROJECT_SOURCE_DIR}/include/lightmodbus/libconf.h"
	)

#Set library version
set_target_properties( liblightmodbus PROPERTIES VERSION ${PROJECT_VERSION} )