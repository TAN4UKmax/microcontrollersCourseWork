/**
 * @file ModbusRTU.cpp
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-06
 * 
 * @copyright Copyright (c) 2023 TAN4UK
 * 
 */
#include "ModbusRTUMaster.h"

 // ����������� (���� ������� ���������� �� �����)
ModbusRTUMaster::ModbusRTUMaster()
{

}

 /**
  * @brief calculates CRC16
  * @details
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
unsigned int ModbusRTUMaster::crc_chk(unsigned char* data, unsigned char length)
{
    int j;
    unsigned int reg_crc = 0xFFFF;
    while (length--) {
        reg_crc ^= *data++;
        for (j = 0; j < 8; j++) {
            if (reg_crc & 0x01) { /*LSB(b0)=1 */

                reg_crc = (reg_crc >> 1) ^ 0xA001;
            }
            else {
                reg_crc = reg_crc >> 1;
            }
        }
    }
    return reg_crc;
}
