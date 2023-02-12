/**
 * @file main.cpp
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief For ModbusRTU library testing
 * @version 0.1
 * @date 2023-02-06
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */
#include <iostream>
#include <cassert>
#include "ModbusRTUMaster.h"

 // подключение функций вывода
using std::cout;
using std::endl;

void TestCRC(ModbusRTUMaster modbus)
{
	unsigned char in_data[] = { 0x01, 0x03, 0x21, 0x02, 0x00, 0x02 };
	size_t arr_length = sizeof(in_data);
	assert((modbus.crc_chk(in_data, arr_length) == 0xf76f));
	cout << "CRC test OK!" << endl;
}

/**
 * @brief Main entry point
 *
 * @return int
 */
int main()
{
	ModbusRTUMaster modbus;
	TestCRC(modbus);
	//throw "Выход за границу диапазона";
	modbus.InitPort((const char* device, int baud, char parity, int data_bit, int stop_bit)); // инициализация ком порта 
	
	std::cout << "All tests OK!" << std::endl;
	return 0;
}

