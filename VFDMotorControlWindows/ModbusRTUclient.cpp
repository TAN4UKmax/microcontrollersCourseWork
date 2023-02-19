#include "ModbusRTUclient.h"
#include "COMPort.h"

//#define NDEBUG
#include <cassert>
#ifndef NDEBUG
#include <cstdio> // for debug printing
#endif // NDEBUG

 /**
  * @brief calculates CRC16        
  *   Шаг 1 : Загрузка 16-bit регистра (называемого CRC регистром) с FFFFH;
  *   Шаг 2: Исключающее ИЛИ первому 8-bit байту из командного сообщения с байтом младшего
  *   порядка из 16-bit регистра CRC, помещение результата в CRC регистр.
  *   Шаг 3: Сдвиг одного бита регистра CRC вправо с MSB нулевым заполнением. Извлечение и
  *   проверка LSB.
  *   Шаг 4: Если LSB CRC регистра равно 0, повторите шаг 3, в противном случае исключающее ИЛИ
  *   CRC регистра с полиномиальным значением A001H.
  *   Шаг 5: Повторяйте шаг 3 и 4, до тех пор, пока восемь сдвигов не будут выполнены. Затем, полный
  *   8-bit байт будет обработан.
  *   Шаг 6: Повторите шаг со 2 по 5 для следующих 8-bit байтов из командного сообщения.
  *   Продолжайте пока все байты не будут обработаны. Конечное содержание CRC регистра CRC
  *   значение. При передачи значения CRC в сообщении, старшие и младшие байты значения CRC должны
  *   меняться, то есть сначала будет передан младший байт.
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

// Дальше идут функции из документации
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
