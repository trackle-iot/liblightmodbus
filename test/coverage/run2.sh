#!/bin/sh
cd "$(dirname "$0")"
rm -rf tmp
mkdir tmp

REPO=../..

gcc --coverage -Wall -I $REPO/include -o tmp/test -imacros macros.h \
	-x c $REPO/include/lightmodbus/base.impl.h \
	-x c $REPO/include/lightmodbus/master.impl.h \
	-x c $REPO/include/lightmodbus/slave.impl.h \
	-x c $REPO/include/lightmodbus/slave_func.impl.h \
	-x c $REPO/include/lightmodbus/master_func.impl.h \
	-x c $REPO/include/lightmodbus/debug.impl.h \
	coverage.c

