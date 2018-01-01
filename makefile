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

# This is makefile for liblightmodbus - a lightweight Modbus library
# It creates object files and a static library, so you can link it to your project
# If you want, liblightmodbus can be installed on your system using 'make install', or later removed using 'make uninstall'
# If you want to build it yourself for platform like AVR, this is not the makefile you are looking for
# Beside, files building order is cruicial - modules first, then base, and finally linking, otherwise things will go wrong

# This makefile is part of source deb package

include .makeconf

ARCH = $(shell arch)
CC = gcc
CFLAGS = -Wall -I./include
LD = ld
LDFLAGS =
AR = ar

ifneq ("$(wildcard $(MODCONF))","")
MODULES = $(shell cat $(MODCONF))
else
MODULES = slave-coils slave-registers slave-base slave-link master-coils master-registers master-base master-link addon-examine
endif

all: $(MODULES)
all: .modules.conf include/lightmodbus/libconf.h clean force core
	$(call linkHeader,full object file)
	echo "[linking] full object file (obj/lightmodbus.o)" >> $(BUILDLOG)
	$(LD) $(LDFLAGS) -r obj/*.o obj/addons/*.o -o obj/lightmodbus.o
	$(call linkHeader,static library file)
	echo "[creating] static library file (lib/liblightmodbus.a)" >> $(BUILDLOG)
	$(AR) -cvq lib/liblightmodbus.a obj/lightmodbus.o
	$(AR) -t  lib/liblightmodbus.a
	echo -n "[success] build finished - " >> $(BUILDLOG)
	date >> $(BUILDLOG)
	$(call infoHeader,build finished successfully)

install:
	$(call infoHeader,installing liblightmodbus)
	-mkdir -p $(DESTDIR)/usr
	-mkdir -p $(DESTDIR)/usr/include
	cp -R include/lightmodbus $(DESTDIR)/usr/include
	-mkdir $(DESTDIR)/usr/lib
	cp -R lib/liblightmodbus.a $(DESTDIR)/usr/lib

uninstall:
	$(call infoHeader,removing liblightmodbus)
	rm -rf $(DESTDIR)/usr/include/lightmodbus
	rm -f $(DESTDIR)/usr/lib/liblightmodbus.a

force:
	$(call infoHeader,starting build)
	./genconf.sh -v
	echo -n "" > $(BUILDLOG)
	echo -n "[arch] " >> $(BUILDLOG)
	echo $(ARCH) >> $(BUILDLOG)
	echo -n "[start] build started - " >> $(BUILDLOG)
	date >> $(BUILDLOG)
	-mkdir obj
	-mkdir obj/slave
	-mkdir obj/master
	-mkdir obj/addons
	-mkdir lib

clean:
	$(call infoHeader,cleaning up build environment)
	-find . -name "*.gch" -type f -delete
	-rm -rf obj
	-rm -rf lib
	-rm -f $(BUILDLOG)
	-rm -f $(CONFLOG)
	-rm -f *.gcno
	-rm -f *.gcda
	-rm -f *.o
	-rm -f coverage-test
	-rm -f coverage-test.log
	-rm -f valgrind.xml
	-rm -f massif.out

conf: configure
configure:
	$(call infoHeader,generating default configuration)
	./genconf.sh

confclean: deconfigure
deconfigure:
	$(call infoHeader,cleaning up configuration)
	./genconf.sh -r

################################################################################

#This target should never be reached
.modules.conf:
	$(call warnHeader,library modules config header file missing - run ./genconf.sh)

include/lightmodbus/libconf.h:
	$(call warnHeader,library config header file missing - run ./genconf.sh)
	$(call warnHeader,generating default configuration)
	./genconf.sh

core: src/core.c include/lightmodbus/core.h
	$(call compileHeader,core module)
	echo "[compiling] core module (obj/core.o)" >> $(BUILDLOG)
	$(CC) $(CFLAGS) -c src/core.c -o obj/core.o

addon-examine: src/addons/examine.c include/lightmodbus/addons/examine.h
	$(call compileHeader,frame examination addon)
	echo "[compiling] frame examination addon (obj/addons/examine.o)" >> $(BUILDLOG)
	$(CC) $(CFLAGS) -c src/addons/examine.c -o obj/addons/examine.o

master-base: src/master.c include/lightmodbus/master.h
	$(call compileHeader,master base module)
	echo "[compiling] master module (obj/master/mbase.o)" >> $(BUILDLOG)
	$(CC) $(CFLAGS) -c src/master.c -o obj/master/mbase.o

master-registers: src/master/mpregs.c include/lightmodbus/master/mpregs.h src/master/mbregs.c include/lightmodbus/master/mbregs.h
	$(call compileHeader,master registers module)
	echo "[compiling] master registers module (obj/master/mregisters.o)" >> $(BUILDLOG)
	$(CC) $(CFLAGS) -c src/master/mpregs.c -o obj/master/mpregs.o
	$(CC) $(CFLAGS) -c src/master/mbregs.c -o obj/master/mbregs.o

master-coils: src/master/mpcoils.c include/lightmodbus/master/mpcoils.h src/master/mbcoils.c include/lightmodbus/master/mbcoils.h
	$(call compileHeader,master coils module)
	echo "[compiling] master coils module (obj/master/mcoils.o)" >> $(BUILDLOG)
	$(CC) $(CFLAGS) -c src/master/mpcoils.c -o obj/master/mpcoils.o
	$(CC) $(CFLAGS) -c src/master/mbcoils.c -o obj/master/mbcoils.o

master-link:
	$(call linkHeader,master modules)
	echo "[linking] master module (obj/master.o)" >> $(BUILDLOG)
	$(LD) $(LDFLAGS) -r obj/master/*.o -o obj/master.o

slave-base: src/slave.c include/lightmodbus/slave.h
	$(call compileHeader,slave base module)
	echo "[compiling] slave module (obj/slave/sbase.o)" >> $(BUILDLOG)
	$(CC) $(CFLAGS) -c src/slave.c -o obj/slave/sbase.o

slave-registers: src/slave/sregs.c include/lightmodbus/slave/sregs.h
	$(call compileHeader,slave registers module)
	echo "[compiling] slave registers module (obj/slave/sregisters.o)" >> $(BUILDLOG)
	$(CC) $(CFLAGS) -c src/slave/sregs.c -o obj/slave/sregs.o

slave-coils: src/slave/scoils.c include/lightmodbus/slave/scoils.h
	$(call compileHeader,slave coils module)
	echo "[compiling] slave coils module (obj/slave/scoils.o)" >> $(BUILDLOG)
	$(CC) $(CFLAGS) -c src/slave/scoils.c -o obj/slave/scoils.o

slave-link:
	$(call linkHeader,slave modules)
	echo "[linking] slave module (obj/slave.o)" >> $(BUILDLOG)
	$(LD) $(LDFLAGS) -r obj/slave/*.o -o obj/slave.o
