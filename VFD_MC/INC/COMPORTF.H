/**
 * @file COMPortFake.h
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief Fake class for testing Modbus (returns predefined data instead of reading the port itself)
 * All methods which are marked as not used in their description are not necessary to use.
 * Only 
 * @version 0.1
 * @date 2023-02-18
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */

#ifndef COMPORTFAKE_H
#define COMPORTFAKE_H

//#include ".\Lib\7188E.H" // 7188EX uPAC header
//#include "..\Lib\7188E.H" // 7188EX uPAC header
#include <Windows.h>

class COMPortFake
{
private:
    char            name[9];    // COM port name ("COM3" by default)
    unsigned long   baud;       // baudrate of communication (9600 by default)
    unsigned char   dataBit;    // number of bits of data (8 by default)
    char            parity;     // parity parameter ('N' by default)
    unsigned char   stopBit;    // number of stop bits (1 by default)

    bool            opened;     // current status of COM port

    /**
     * @brief Calculates CRC16
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
     * @param data[in]          - a pointer to the message buffer
     * @param length[in]        - the message buffer length
     * @return unsigned int     - calculated CRC
     */
    unsigned int CRC16(unsigned char* data, unsigned char length);
public:
    /**
     * @brief (It will work with any configuration in COMPortFake) Construct a new COMPortFake object
     * 
     * @param name[in]      - COM port name ("COM3" by default)
     * @param baud[in]      - baudrate of communication (9600 by default)
     * @param dataBit[in]   - number of bits of data (8 by default)
     * @param parity[in]    - parity parameter ('N' or NOPARITY by default)
     * @param stopBit[in]   - number of stop bits (1 or ONESTOPBIT by default)
     */
    COMPortFake(
        const char* name = "COM3",
        unsigned long baud = 9600,
        unsigned char dataBit = 8,
        char parity = 'N',
        unsigned char stopBit = 1);

    /**
     * @brief Destroy the COMPortFake object (closes the file handle)
     * 
     */
    ~COMPortFake();

    /**
     * @brief Open COM port communication
     * 
     * @return true     - if open success
     * @return false    - if open fails
     */
    bool Open();

    /**
     * @brief Check current state of COM port
     *
     * @return true     - Port is opened
     * @return false    - Port is closed
     */
    bool isOpen();

    /**
     * @brief Close COM port communication
     * 
     * @return true     - if close success
     * @return false    - if close fails
     */
    bool Close();

    /**
     * @brief Set the Config of port, allows to change COM port configuration after creating an object
     * 
     * @param baud[in]      - baudrate of communication (9600 by default)
     * @param dataBit[in]   - number of bits of data (8 by default)
     * @param parity[in]    - parity parameter ('N' or NOPARITY by default)
     * @param stopBit[in]   - number of stop bits (1 or ONESTOPBIT by default)
     * @return true[in]     - if port parameters changed
     * @return false        - if change fails
     */
    bool SetConfig(
        unsigned long baud = 9600,
        unsigned char dataBit = 8,
        char parity = 'N',
        unsigned char stopBit = 1);
    
    /**
     * @brief Set the Read Timeouts for COM port buffer.
     * (0, 0, 1)
     * - return immediately with the bytes that have already been received,
     * even if no bytes have been received
     * (0, 0, 0)
     * - wait untill buffer is filled up to specified size
     * (1, 0, 0)
     * - If there are any bytes in the input buffer,
     * ReadFile returns immediately with the bytes in the buffer.
     * If there are no bytes in the input buffer,
     * ReadFile waits until a byte arrives and then returns immediately.
     * If no bytes arrive within the time specified by ReadTotalTimeoutConstant,
     * ReadFile times out.
     * @param interval[in]      - The maximum time allowed to elapse before the arrival of the next byte on the communications line, in milliseconds
     * @param multiplier[in]    - The multiplier used to calculate the total time-out period for read operations, in milliseconds.
     * @param constant[in]      - A constant used to calculate the total time-out period for read operations, in milliseconds.
     * @return true             - if timeouts updated
     * @return false            - if timeouts update fails
     */
    bool SetReadTimeouts(
        unsigned long interval = 0,
        unsigned long multiplier = 0,
        unsigned long constant = 1);
    
    /**
     * @brief Clears input buffer of port and terminates all current read operations
     *
     * @return true     - if clear success
     * @return false    - if clear fails
     */
    bool ClearReadBuffer();

    /**
     * @brief Writes an array of binary data into COM port
     * 
     * @param buf[in]       - pointer to data buffer
     * @param length[in]    - buffer length
     * @return long         - number of written bytes or -1 if error
     */
    long Write(unsigned char* buf, unsigned char length);

    /**
     * @brief Reads an array of binary data from COM port buffer
     * 
     * @param buf[out]      - pointer to data buffer
     * @param length[in]    - buffer length
     * @return long         - number of read bytes or -1 if error
     */
    long Read(unsigned char* buf, unsigned char length);
};

#endif // COMPORTFAKE_H
