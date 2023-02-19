#include "ModbusRTUclient.h"
#include "COMPort.h"

//#define NDEBUG
#include <cassert>
#ifndef NDEBUG
#include <cstdio> // for debug printing
#endif // NDEBUG

 /**
  * @brief calculates CRC16        
  *   ��� 1 : �������� 16-bit �������� (����������� CRC ���������) � FFFFH;
  *   ��� 2: ����������� ��� ������� 8-bit ����� �� ���������� ��������� � ������ ��������
  *   ������� �� 16-bit �������� CRC, ��������� ���������� � CRC �������.
  *   ��� 3: ����� ������ ���� �������� CRC ������ � MSB ������� �����������. ���������� �
  *   �������� LSB.
  *   ��� 4: ���� LSB CRC �������� ����� 0, ��������� ��� 3, � ��������� ������ ����������� ���
  *   CRC �������� � �������������� ��������� A001H.
  *   ��� 5: ���������� ��� 3 � 4, �� ��� ���, ���� ������ ������� �� ����� ���������. �����, ������
  *   8-bit ���� ����� ���������.
  *   ��� 6: ��������� ��� �� 2 �� 5 ��� ��������� 8-bit ������ �� ���������� ���������.
  *   ����������� ���� ��� ����� �� ����� ����������. �������� ���������� CRC �������� CRC
  *   ��������. ��� �������� �������� CRC � ���������, ������� � ������� ����� �������� CRC ������
  *   ��������, �� ���� ������� ����� ������� ������� ����.
  *
  * @param data a pointer to the message buffer
  * @param length the quantity of bytes in the message buffer
  * @return unsigned int
  */
unsigned int ModbusRTUClient::crc_chk(unsigned char* data, unsigned char length)
{
    int j;
    unsigned int reg_crc = 0xFFFF;
    while (length--)
    {
        unsigned char tmp = *data;
        data++;
        reg_crc ^= tmp;
        for (j = 0; j < 8; j++)
        {
            if (reg_crc & 0x01) /* LSB(b0) = 1 */
            {
                reg_crc = (reg_crc >> 1) ^ 0xA001;
            }
            else
            {
                reg_crc = reg_crc >> 1;
            }
        }
    }
    return reg_crc;
}

ModbusRTUClient::ModbusRTUClient()
{

}

ModbusRTUClient::~ModbusRTUClient()
{

}

bool ModbusRTUClient::ReadHoldingRegisters(
    unsigned short regAddress,
    unsigned char nRegisters,
    unsigned char* buf)
{

    return FALSE;
}

bool ModbusRTUClient::WriteSingleRegster(
    unsigned short regAddress,
    unsigned short regValue)
{

    return FALSE;
}

// ������ ���� ������� �� ������������
// 0x01
// 0x02
// 0x03
// 0x04
// 0x05
// 0x06
// 0x0F
// 0x10
//constexpr auto MODBUS_FC_READ_EXCEPTION_STATUS = 0x07;
//
///* Protocol exceptions */
//enum {
//	MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,
//	MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
//	MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
//	MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,
//	MODBUS_EXCEPTION_ACKNOWLEDGE,
//	MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,
//	MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
//	MODBUS_EXCEPTION_MEMORY_PARITY,
//	MODBUS_EXCEPTION_NOT_DEFINED,
//	MODBUS_EXCEPTION_GATEWAY_PATH,
//	MODBUS_EXCEPTION_GATEWAY_TARGET,
//	MODBUS_EXCEPTION_MAX
//};
