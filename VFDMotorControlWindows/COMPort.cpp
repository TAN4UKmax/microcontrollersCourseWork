#include "COMPort.h"

COMPort::COMPort(const char* name /* = "COM3" */,
    unsigned long baud /* = 9600 */,
    char parity /* = 'N' */,
    unsigned char dataBit /* = 8 */,
    unsigned char stopBit /* = 1 */)
{
	// Здесь проверка всех значений на корректность и запись их в члены класса
}

COMPort::~COMPort()
{
    
}

bool COMPort::Open()
{

}

bool COMPort::Close()
{

}

bool COMPort::SetReadTimeouts(unsigned long interval,
    unsigned long multiplier,
    unsigned long constant)
{

}

bool COMPort::Write(char* buf, unsigned char& length)
{

}

bool COMPort::Read(char* buf, unsigned char& length)
{

}
