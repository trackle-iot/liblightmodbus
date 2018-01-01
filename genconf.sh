#!/bin/bash
# liblightmodbus - a lightweight, multiplatform Modbus library
# Copyright (C) 2017 Jacek Wieczorek <mrjjot@gmail.com>

# This file is part of liblightmodbus.

# Liblightmodbus is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Liblightmodbus is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#The default configuration
DEFAULTS=1
MMODULES="01 02 03 04 05 06 15 16 22"
SMODULES="01 02 03 04 05 06 15 16 22"
ADDONS="examine"
STATICMEM_SRES=
STATICMEM_SREQ=
STATICMEM_MRES=
STATICMEM_MREQ=
STATICMEM_MDAT=
ENDIAN=auto

#Function to display help message
function showhelp()
{
	cat <<EOM
genconf.sh is a bash script intended for generation of liblightmodbus library
configuration files (include/lightmodbus/libconf.h and .modules.conf), which
are crucial for it to be built. It is ought to be run before make.

Supported options:
	-h - display this help message
	-v - update only version number in configuration
	-r - remove current configuration
	-s <modules> - select slave modules to be included
	-m <modules> - select master modules to be included
	-a <addons> - select addons to be included
	--sres <size> - enable fixed-size slave response data buffer
	--sreq <size> - enable fixed-size slave request data buffer
	--mres <size> - enable fixed-size master response data buffer
	--mreq <size> - enable fixed-size master request data buffer
	--mdat <size> - enable fixed-size master parsed data buffer
	--endian <endianness> - force endianness

Where:
	<modules> is a list of two-digit Modbus function codes, which you want to
	be suported by either master or slave. For instance:
	"01 15 22 16"

	If an empty string is passed, all modules (including base) will be omitted.
	To prevent such a behavior, "forcebase" can be used instead - it guarantees,
	that base module will be compiled and linked.

	<addons> represents list of to be added to the library. Currently available
	ones are: "examine". Please refer to manpages to get more information about
	supported addons.

	<size> is fixed-length of given data buffer in bytes. Has to be an integer
	greater than 0. The highest reasonable value for those settings is 256,
	because it's the maximum Modbus frame size.

	<endianness> can be either 'auto', 'little' or 'big'.
EOM
}

#Function for logging configuration process
function log()
{
	echo $1 | tee -a $CONFLOG
}

#Removes configuration as well as log file
function rmconf()
{
	rm -f $LIBCONF
	rm -f $MODCONF
	rm -f $CONFLOG
	make clean &> /dev/null
}

#Generates module configuration based on addon list
function genaddons()
{
	if [[ $ADDONS == *"examine"* ]]; then
		log "[info] frame examination module is going to be included"
		echo "#define LIGHTMODBUS_ADDON_EXAMINE" >> $LIBCONF
		echo "addon-examine" >> $MODCONF
	fi
}

#Generate module configuration based on given arguments (side name, desired functions)
function genmodules()
{
	local suffix=$(echo $1 | tr /a-z/ /A-Z/ | head -c 1)
	local modules=( \
			[01]="coils" \
			[02]="coils" \
			[03]="registers" \
			[04]="registers" \
			[05]="coils" \
			[06]="registers" \
			[15]="coils" \
			[16]="registers" \
			[22]="registers" )
	local addbase=0

	if [[ $2 == *"forcebase"* ]]; then
		addbase=1
	fi

	for i in ${!modules[@]}; do

		if [[ $2 == *"$i"* ]]; then
			printf "#define LIGHTMODBUS_F%02d%01s\n" $i $suffix >> $LIBCONF

			log "[info] $1 function $i is going to be included"

			if [[ -n ${modules[$i]} ]]; then
				echo "$1-${modules[$i]}" >> $MODCONF
				addbase=1
			fi
		fi
	done

	if [[ $addbase -eq 1 ]]; then
		echo "$1-base $1-link" >> $MODCONF
		log "[info] $1 base is going to be included"
	else
		log "[warning] skipping $1 base module"
	fi;
}

#Generate fixed-size buffer macros based on given arguments (macro name, human readable name, value)
function genstaticmem()
{
	if [[ -n "${3// }" ]]; then
		case $3 in
	    	''|*[!0-9]*)
				log "[error] $2 must be an integer value"
				exit 1
				;;
		esac

		if [[ $3 -le 0 ]]; then
			log "[error] $2 must be greater than 0"
			exit 1
		fi

		log "[info] $2 is $3 bytes"
		echo "#define $1 $3" >> $LIBCONF
	fi;
}

#Update version number in configuration header
function genversion()
{
	local line=$(grep -no 'LIGHTMODBUS_VERSION' $LIBCONF | grep -Eo '^[^:]+')
	local version=""
	if gitver=$(git describe --abbrev=6 --dirty --always --tag 2> /dev/null); then
		version=$gitver
	else
		version="no-vcs-found"
	fi
	log "[info] updating version number"
	sed -i "$line s/.*/#define LIGHTMODBUS_VERSION \"$version\"/" $LIBCONF
}

#Generates new configuration
function genconf()
{
	rmconf

	touch $LIBCONF
	touch $MODCONF
	touch $CONFLOG

	log "[info] cleaned build environment"

	if [[ $DEFAULTS -eq 1 ]]; then
		log "[info] using defaults"
	fi

	#Write lovely header to library configuration header
	cat >> $LIBCONF <<EOM
/*
	This file gets automatically generated by makefile

	Its purpose is to configure each liblightmodbus header file,
	so the user does not have to pass tons of compiler options each time
	he/she wants to compile the library.
	It also serves as a pretty nice source of information about current
	library setup.

	Do not edit this file manually unless you know what you are doing.
*/
#ifndef LIGHTMODBUS_LIBCONF_H
#define LIGHTMODBUS_LIBCONF_H
#define LIGHTMODBUS_VERSION
EOM

	#Generate version number
	genversion

	#Adapt library to big-endian systems if necessary
	case $ENDIAN in
		auto) ENDIAN=-1;;
		little) ENDIAN=1;;
		big) ENDIAN=0;;
		*) ENDIAN=-1;;
	esac

	if [[ $ENDIAN -eq -1 ]]; then
		ENDIAN=$(echo -n I | od -to2 | head -n1 | cut -f2 -d" " | cut -c 6)
	fi

	if [[ $ENDIAN -eq 0 ]]; then
		log "[info] using big-endian build configuration"
		echo "#define LIGHTMODBUS_BIG_ENDIAN" >> $LIBCONF
	else
		log "[info] using little-endian build configuration"
	fi

	#Manage static buffers confiuration
	genstaticmem "LIGHTMODBUS_STATIC_MEM_SLAVE_REQUEST" "slave request buffer size" $STATICMEM_SREQ
	genstaticmem "LIGHTMODBUS_STATIC_MEM_SLAVE_RESPONSE" "slave response buffer size" $STATICMEM_SRES
	genstaticmem "LIGHTMODBUS_STATIC_MEM_MASTER_REQUEST" "master request buffer size" $STATICMEM_MREQ
	genstaticmem "LIGHTMODBUS_STATIC_MEM_MASTER_RESPONSE" "master response buffer size" $STATICMEM_MRES
	genstaticmem "LIGHTMODBUS_STATIC_MEM_MASTER_DATA" "master parsed data buffer size" $STATICMEM_MDAT

	#Manage modules
	genmodules "slave" "$SMODULES"
	genmodules "master" "$MMODULES"
	genaddons

	#Nicely end preprocessor if
	echo "#endif" >> $LIBCONF

	#Sort and remove repeated modules
 	echo "$(sort -r $MODCONF | uniq)" > $MODCONF
}

#Check if library configuration path or log path are set
if [ -z $LIBCONF ]; then
	LIBCONF=include/lightmodbus/libconf.h
fi
if [ -z $MODCONF ]; then
	MODCONF=.modules.conf
fi
if [ -z $CONFLOG ]; then
	CONFLOG=conf.log
fi

while [[ $# -gt 0 ]]; do
	case $1 in

	    -h|--help)
	    	showhelp
			exit 0
	    	;;

		-v)
		    genversion
			exit 0
		    ;;

	    -r)
	    	rmconf
			echo "[info] removed current configuration files"
			exit 0
	    	;;

		-s)
			SMODULES=$2
			DEFAULTS=0
			shift
			;;

		-m)
			MMODULES=$2
			DEFAULTS=0
			shift
			;;

		-a)
			ADDONS=$2
			DEFAULTS=0
			shift
			;;

		--sres)
			STATICMEM_SRES=$2
			DEFAULTS=0
			shift
			;;

		--sreq)
			STATICMEM_SREQ=$2
			DEFAULTS=0
			shift
			;;

		--mres)
			STATICMEM_MRES=$2
			DEFAULTS=0
			shift
			;;

		--mreq)
			STATICMEM_MREQ=$2
			DEFAULTS=0
			shift
			;;

		--mdat)
			STATICMEM_MDAT=$2
			DEFAULTS=0
			shift
			;;

		--endian)
			ENDIAN=$2
			DEFAULTS=0
			shift
			;;

		*)
			echo "$0: unknown option '$1'" >&2
			exit 1
	    	;;
	esac
	shift
done

genconf
exit 0
