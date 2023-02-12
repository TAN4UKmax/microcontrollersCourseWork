/**
 * @file ModbusRTU.h
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief ModbusRTU library header
 * @version 0.1
 * @date 2023-02-06
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */

#pragma once
class ModbusRTUMaster
{
//private:
	// пока что паблик для дебага, потом сделаю приват
public:
	// расчет CRC16
	unsigned int crc_chk(unsigned char* data, unsigned char length);
	
public:
	// конструктор (пока никаких параметров не берет)
	ModbusRTUMaster();
	// инициализация порта
	InitPort(const char* device, int baud, char parity, int data_bit, int stop_bit); // здесь значения по умолчани будут
	//("COM1", 9600, 'N', 8, 1)

	// Дальше идут функции из документации
	// 0x01
	// 0x02
	// 0x03
	// 0x04
	// 0x05
	// 0x06
	// 0x0F
	// 0x10
	constexpr auto MODBUS_FC_READ_EXCEPTION_STATUS = 0x07;

	/* Protocol exceptions */
	enum {
		MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,
		MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
		MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
		MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,
		MODBUS_EXCEPTION_ACKNOWLEDGE,
		MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,
		MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
		MODBUS_EXCEPTION_MEMORY_PARITY,
		MODBUS_EXCEPTION_NOT_DEFINED,
		MODBUS_EXCEPTION_GATEWAY_PATH,
		MODBUS_EXCEPTION_GATEWAY_TARGET,
		MODBUS_EXCEPTION_MAX
	};
};

