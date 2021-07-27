#!/bin/sh
cd "$(dirname "$0")"
rm -rf tmp
mkdir tmp

cp -r ../../include/lightmodbus tmp/lightmodbus
for f in tmp/lightmodbus/*.impl.h; do
	mv "$f" "${f%.impl.h}.c"
done

gcc --coverage -Wall -I tmp tmp/lightmodbus/*.c coverage.c -o tmp/test -imacros macros.h