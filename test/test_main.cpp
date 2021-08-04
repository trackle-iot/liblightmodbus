#include "tester.hpp"

void last_register_tests()
{
	run_test("Reading last coils/registers", [](){
		set_mode("pdu");
		for (int i = 1; i <= 4; i++)
		{
			build_request({1, i, 65535, 1});
			dump_request();
			parse_request();
			assert_slave(1);
			parse_response();
			assert_master(1);
			dump_data();
		}
	});

	run_test("Reading two last coils/registers", [](){
		for (int i = 1; i <= 4; i++)
		{
			set_mode("pdu");
			build_request({1, i, 65534, 2});
			dump_request();
			parse_request();
			assert_slave(1);
			parse_response();
			assert_master(1);
			dump_data();
		}
	});

	run_test("Reading one past the last coil/register", [](){
		set_mode("pdu");
		for (int i = 1; i <= 4; i++)
		{
			build_request({1, i, 65535, 2});
			assert_master(0);

			set_request({i, 255, 255, 0, 2});
			dump_request();
			parse_request();
			dump_slave();
			assert_slave_ex(MODBUS_EXCEP_ILLEGAL_ADDRESS);
		}
	});
}

void modbus_pdu_tests()
{
	run_test("Parse empty PDU frame", [](){
		set_mode("pdu");
		set_request({});
		parse_request();
		assert_slave(0);
	});
}

void modbus_rtu_tests()
{
	run_test("Parse a raw Modbus RTU frame (write register)", [](){
		set_mode("rtu");
		set_request({0x01, 0x06, 0xab, 0xcd, 0x01, 0x23, 0x78, 0x58});
		parse_request();
		assert_slave(1);
		assert_reg(0xabcd, 0x0123);
		dump_response();
		parse_response();
		assert_master(1);
	});

	run_test("Parse a raw Modbus RTU frame with invalid CRC (big-endian)", [](){
		set_mode("rtu");
		set_request({0x01, 0x06, 0xab, 0xcd, 0x01, 0x23, 0x58, 0x78});
		parse_request();
		assert_slave(0);
	});

	run_test("Parse a raw Modbus RTU frame with invalid CRC", [](){
		set_mode("rtu");
		set_request({0x01, 0x06, 0xab, 0xcd, 0x01, 0x23, 0xfa, 0xce});
		parse_request();
		assert_slave(0);
	});

	run_test("Parse empty RTU frame", [](){
		set_mode("rtu");
		set_request({});
		parse_request();
		assert_slave(0);
	});
}

void test_main()
{
	modbus_pdu_tests();
	modbus_rtu_tests();
	last_register_tests();

	run_test("Reading 2000 coils/discrete inputs", [](){
		set_mode("pdu");
		for (int i = 1; i <= 2; i++)
		{
			build_request({1, i, 0, 2000});
			assert_master(1);
			parse_request();
			dump_slave();
			assert_slave_ex(MODBUS_EXCEP_NONE);
			parse_response();
			assert_master(1);
		}
	});

	run_test("Reading 2001 coils", [](){
		set_mode("pdu");
		for (int i = 1; i <= 2; i++)
		{
			build_request({1, i, 0, 2001});
			dump_master();
			assert_master(0);

			set_request({i, 0, 0, 0x07, 0xd1});
			parse_request();
			dump_slave();
			assert_slave_ex(MODBUS_EXCEP_ILLEGAL_VALUE);
			parse_response();
			assert_master(1);
		}
	});

}