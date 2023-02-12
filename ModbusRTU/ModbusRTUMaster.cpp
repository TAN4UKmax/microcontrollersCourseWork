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

 // конструктор (пока никаких параметров не берет)
ModbusRTUMaster::ModbusRTUMaster()
{

}

 /**
  * @brief calculates CRC16
  * @details
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
