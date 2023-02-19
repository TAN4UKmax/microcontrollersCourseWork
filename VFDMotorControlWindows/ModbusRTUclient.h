/**
 * @file ModbusRTUClient.h
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief Class for manage ModbusRTU protocol
 * @version 0.1
 * @date 2023-02-18
 * 
 * @copyright Copyright (c) 2023 TAN4UK
 * 
 */

#ifndef MODBUSRTUCLIENT_H
#define MODBUSRTUCLIENT_H

class ModbusRTUClient
{
private:
	unsigned char deviceAddress;

	unsigned int crc_chk(unsigned char* data, unsigned char length);
public:
	ModbusRTUClient();
	~ModbusRTUClient();

	bool ReadHoldingRegisters(
		unsigned short regAddress,
		unsigned char nRegisters,
		unsigned char* buf);
	bool WriteSingleRegster(
		unsigned short regAddress,
		unsigned short regValue);
};

#endif // MODBUSRTUCLIENT_H
