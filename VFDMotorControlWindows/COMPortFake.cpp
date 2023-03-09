#include "COMPortFake.h"
#include <cstring> // for string data operations

#define NDEBUG
#include <cassert>
#ifndef NDEBUG
#include <cstdio> // for debug printing
#include <ctime> // for measuring transfer operations time
#endif // NDEBUG

// Тут написать ответ на запрос читать 15 регистров, запрос чтения температуры, 
// все запросы записи автоматически успешны и будет норм

// Test requests and responses (ADU with server address but without CRC) //////
unsigned char Write_req[] = { 0x01, 0x06 };

unsigned char ReadParameterRegisters_req[] = { 0x01, 0x03, 0x21, 0x01, 0x00, 0x0F };
unsigned char ReadParameterRegisters_rsp[] = 
{ 0x01, 0x03, 0x1E, 0x00, 0x00, 0x13, 0x88, 0x13, 0x7E, 0x00, 0x4B, 0x0C, 0x27, 0x08, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x00, 0x0C, 0x03, 0xA4, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD };

// 45.6 deg response
unsigned char GetVFDTemperature_req[] = { 0x01, 0x03, 0x22, 0x06, 0x00, 0x01 };
unsigned char GetVFDTemperature_rsp[] = { 0x01, 0x03, 0x02, 0x00, 0x19 };

// 50 Hz response
unsigned char ReadMaxFrequency_req[] = { 0x01, 0x03, 0x01, 0x00, 0x00, 0x01 };
unsigned char ReadMaxFrequency_rsp[] = { 0x01, 0x03, 0x02, 0x13, 0x88 };

// Testing buffers ////////////////////////////////////////////////////////////
unsigned char write_buffer[256] = { 0 };
unsigned char wLength = 0;

unsigned int COMPortFake::CRC16(unsigned char* data, unsigned char length)
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

COMPortFake::COMPortFake(
	const char* name /* = "COM3" */,
	unsigned long baud /* = 9600 */,
	unsigned char dataBit /* = 8 */,
	char parity /* = 'N' */,
	unsigned char stopBit /* = 1 */)
{
	// Init class members
	opened = false;

	// Check name argument
	if (name == NULL || *name == 0)
	{
		assert(("COMPortFake::Constructor() Incorrect device name", (name && *name)));
		throw("Incorrect device name");
	}
	// Copy port name
	strcpy_s(this->name, 9, name);

	SetConfig(baud, dataBit, parity, stopBit);

#ifndef NDEBUG
	printf("COMPortFake::Constructor() Created instance 0x%p with params:\n", this);
	printf("- name: %s\n", name);
	printf("- baud: %u\n", baud);
	printf("- dataBit: %u\n", dataBit);
	printf("- parity: %c\n", parity);
	printf("- stopBit: %u\n", stopBit);
	printf("- opened: %u\n", this->opened);
#endif // NDEBUG
}

COMPortFake::~COMPortFake()
{
	Close();
#ifndef NDEBUG
	printf("COMPortFake::Destructor() Deleted instance 0x%p\n", this);
#endif // NDEBUG
}

bool COMPortFake::Open()
{
	// Create windows device name
	char portFullName[16] = "\\\\.\\";
	strcpy_s(&portFullName[4], 12, name);
#ifndef NDEBUG
	printf("COMPortFake::Open() Full port name: %s\n", portFullName);
#endif // NDEBUG

	if (isOpen() == true) Close(); // close port if it is already opened by accident
	// Here port is totally opened
	opened = true;

#ifndef NDEBUG
	printf("COMPortFake::Open() Port opened\n");
#endif // NDEBUG
	return true;
}

bool COMPortFake::isOpen()
{
	return opened;
}

bool COMPortFake::Close()
{
	opened = false;
#ifndef NDEBUG
	printf("COMPortFake::Close() Port closed\n");
#endif // NDEBUG
	return true;
}

bool COMPortFake::SetConfig(
	unsigned long baud /* = 9600 */,
	unsigned char dataBit /* = 8 */,
	char parity /* = 'N' */,
	unsigned char stopBit /* = 1 */)
{
	// Check baud argument
	switch (baud) {
	case CBR_110:
	case CBR_300:
	case CBR_600:
	case CBR_1200:
	case CBR_2400:
	case CBR_4800:
	case CBR_9600:
	case CBR_14400:
	case CBR_19200:
	case CBR_38400:
	case CBR_57600:
	case CBR_115200:
	case CBR_128000:
	case CBR_256000:
		this->baud = baud;
		break;
	default:
		this->baud = CBR_9600;
		break;
	}
	// Check data bits
	switch (dataBit) {
	case 5:
		this->dataBit = 5;
		break;
	case 6:
		this->dataBit = 6;
		break;
	case 7:
		this->dataBit = 7;
		break;
	case 8:
	default:
		this->dataBit = 8;
		break;
	}
	// Check parity argument
	if (parity == 'E' || parity == 'e' || parity == EVENPARITY) this->parity = EVENPARITY;
	else if (parity == 'O' || parity == 'o' || parity == ODDPARITY) this->parity = ODDPARITY;
	else this->parity = NOPARITY;
	// Check stop bits
	if (stopBit == 1) this->stopBit = ONESTOPBIT;
	else this->stopBit = TWOSTOPBITS;
	// if not opened then return here
	if (isOpen() == false) return true;
#ifndef NDEBUG
	printf("COMPortFake::SetConfig() Config is set\n");
#endif // NDEBUG
	return true;
}

bool COMPortFake::SetReadTimeouts(
	unsigned long interval /* = 0 */,
	unsigned long multiplier /* = 0 */,
	unsigned long constant /* = 1 */)
{
	if (isOpen() == false) return true;
#ifndef NDEBUG
	printf("COMPortFake::SetReadTimeouts() Timeouts is set\n");
#endif // NDEBUG
	return true;
}

bool COMPortFake::ClearReadBuffer()
{
	if (isOpen() == false) Open();
#ifndef NDEBUG
	printf("COMPortFake::ClearReadBuffer() Clearing read buffer success\n");
#endif // NDEBUG
	return true;
}

long COMPortFake::Write(unsigned char* buf, unsigned char length)
{
	if (isOpen() == false) Open();
#ifndef NDEBUG
	printf("COMPortFake::Write() Writing %d bytes into port: 0x", length);
	for (int i = 0; i < length; i++)
		printf(" %02X", buf[i]);
	printf("\n");
	clock_t start_time = clock();
#endif // NDEBUG

	// Fake writing ///////////////////////////////////////////////////////////
	memcpy(write_buffer, buf, length);
	wLength = length;
	if (length < 2) return -1; // received not a modbus request
	// fake CRC check
	unsigned short wCRC;
	wCRC = write_buffer[length - 1];				// CRC Hi
	wCRC = (wCRC << 8) | write_buffer[length - 2];	// CRC Lo
	unsigned short rCalcCRC = CRC16(write_buffer, (length - 2));
	Sleep(2); // Write delay

	if (wCRC == rCalcCRC)
	{
#ifndef NDEBUG
		printf("COMPortFake::Write() %d bytes written in %ldms\n", length, (clock() - start_time));
#endif // NDEBUG
		return length;
	}
	else
	{
#ifndef NDEBUG
		printf("COMPortFake::Write() Write failed\n");
#endif // NDEBUG
		return -1;
	}
}

long COMPortFake::Read(unsigned char* buf, unsigned char length)
{
	if (isOpen() == false) Open();
#ifndef NDEBUG
	clock_t start_time = clock();
	printf("COMPortFake::Read() Reading %d bytes from port:", length);
#endif // NDEBUG

	// Fake read //////////////////////////////////////////////////////////////
	Sleep(10); // Read delay

	if ((wLength < 3) || (length < 3)) // not a modbus message
		return -1;
	// Response to any write request
	if (!memcmp(write_buffer, Write_req, 2))
	{
		memcpy(buf, write_buffer, length);
		return length;
	}
	// Read parameters registers
	else if (!memcmp(write_buffer, ReadParameterRegisters_req, (wLength - 2)))
		memcpy(buf, ReadParameterRegisters_rsp, (length - 2));
	// Read temperature register
	else if (!memcmp(write_buffer, GetVFDTemperature_req, (wLength - 2)))
		memcpy(buf, GetVFDTemperature_rsp, (length - 2));
	// Read max frequency register
	else if (!memcmp(write_buffer, ReadMaxFrequency_req, (wLength - 2)))
		memcpy(buf, ReadMaxFrequency_rsp, (length - 2));
	else
	{
		// return exception ILLEGAL DATA ADDRESS
		buf[0] = write_buffer[0];
		buf[1] = write_buffer[1] + 0x80;
		buf[2] = 0x02;
		length = 5;
	}
	// Calculate response CRC
	unsigned short rCRC = CRC16(buf, (length - 2));
	buf[length - 2] = rCRC & 0xFF;	// CRC Lo
	buf[length - 1] = rCRC >> 8;	// CRC Hi

#ifndef NDEBUG
	printf(" 0x");
	for (int i = 0; i < length; i++)
		printf(" %02X", buf[i]);
	printf("\nCOMPortFake::Read() %d bytes have read in %ldms\n", length, (clock() - start_time));
#endif // NDEBUG
	return length;
}
