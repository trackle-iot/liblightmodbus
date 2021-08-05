#pragma once
#include <initializer_list>
#include <string>
#include <vector>
#include <functional>

#define LIGHTMODBUS_FULL
#include <lightmodbus/lightmodbus.h>

void build_request(const std::vector<int> &args);
void parse_request();
void parse_response();
void dump_request();
void dump_response();
void dump_data();
void dump_queries();
void dump_master();
void dump_slave();
void dump_regs();
void dump_coils();
void assert_master_err(ModbusErrorInfo err);
void assert_master_ok();
void assert_slave_err(ModbusErrorInfo err);
void assert_slave_ok();
void assert_reg(int index, int value);
void assert_coil(int index, int value);
void assert_slave_ex(ModbusExceptionCode ex);
void set_mode(const std::string &mode);
void set_request(const std::vector<int> &data);
void set_response(const std::vector<int> &data);
void set_reg_count(int n);
void set_coil_count(int n);
void clear_regs(int val);
void clear_coils(int val);
void set_rlock(int index, int lock);
void set_wlock(int index, int lock);
void reset();
void test_info(const std::string &s);
void run_test(const std::string &name, std::function<void()> f);

