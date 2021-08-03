CXX = g++
CXXFLAGS = -Wall -Wextra -Wno-unused-parameter -g -I../include/ --std=c++17
TESTERSRC = tester.cpp
SANITIZERS = -fsanitize=address -fsanitize=undefined -fsanitize=alignment

main-test: FORCE 
	$(CXX) $(CXXFLAGS) $(TESTERSRC) test_main.cpp -o $@ 

sanitizer-test: FORCE
	$(CXX) $(CXXFLAGS) $(SANITIZERS) $(TESTERSRC) test_main.cpp -o $@

coverage-test: FORCE
	$(CXX) $(CXXFLAGS) $(TESTERSRC) --coverage -fprofile-arcs -DCOVERAGE_TEST test_main.cpp -o $@ \
	-DLIGHTMODBUS_FULL \
	-DLIGHTMOBUS_DEBUG \
	-DLIGHTMODBUS_F01S -DLIGHTMODBUS_F01M \
	-DLIGHTMODBUS_F02S -DLIGHTMODBUS_F02M \
	-DLIGHTMODBUS_F03S -DLIGHTMODBUS_F03M \
	-DLIGHTMODBUS_F04S -DLIGHTMODBUS_F04M \
	-DLIGHTMODBUS_F05S -DLIGHTMODBUS_F05M \
	-DLIGHTMODBUS_F06S -DLIGHTMODBUS_F06M \
	-DLIGHTMODBUS_F15S -DLIGHTMODBUS_F15M \
	-DLIGHTMODBUS_F16S -DLIGHTMODBUS_F16M \
	-DLIGHTMODBUS_F22S -DLIGHTMODBUS_F22M \
	-x c ../include/lightmodbus/base.impl.h \
	-x c ../include/lightmodbus/debug.impl.h \
	-x c ../include/lightmodbus/master.impl.h \
	-x c ../include/lightmodbus/master_func.impl.h \
	-x c ../include/lightmodbus/slave.impl.h \
	-x c ../include/lightmodbus/slave_func.impl.h

FORCE: