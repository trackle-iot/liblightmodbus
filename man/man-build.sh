#!/bin/bash

rm -r man/*.*
cd doc

for f in *.md; do
  md2man-roff ./"$f" > ../man/"${f%.md}"
done

cd ../man

cp modbusBuildRequest.3lightmodbus modbusBuildRequest0102.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest0304.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest01.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest02.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest03.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest04.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest05.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest06.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest15.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest16.3lightmodbus
cp modbusBuildRequest.3lightmodbus modbusBuildRequest22.3lightmodbus
