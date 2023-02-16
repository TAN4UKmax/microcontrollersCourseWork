/**
 * @file COMPort.h
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief Class for communication with COM port through WinAPI
 * @version 0.1
 * @date 2023-02-13
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */

#ifndef COMPORT_H
#define COMPORT_H

#include <Windows.h>

class COMPort
{
private:
    HANDLE          hCOM;       // COM port handler structure
    char            name[13];   // COM port name ("COM3" by default)
    unsigned long   baud;       // baudrate of communication (9600 by default)
    char            parity;     // parity parameter ('N' by default)
    unsigned char   dataBit;    // number of bits of data (8 by default)
    unsigned char   stopBit;    // number of stop bits (1 by default)
public:
    // конструктор по умолчанию
    COMPort(const char* name = "COM3",
        unsigned long baud = 9600,
        char parity = 'N',
        unsigned char dataSize = 8,
        unsigned char stop_bit = 1);
    // деструктор
    ~COMPort();

    bool Open();
    bool Close();

    bool SetReadTimeout(unsigned long ms);

    /**
     * @brief Writes an array of binary data into COM port
     *
     * @param buf - pointer to data buffer
     * @param length - buffer legth
     * @return true - if write successful
     * @return false - if write fails
     */
    bool Write(char* buf, unsigned char& length);
    bool Read(char* buf, unsigned char& length);

};

#endif // COMPORT_H
