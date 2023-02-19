#include "COMPortFake.h"
#include <cstring> // for string data operations

//#define NDEBUG
#include <cassert>
#ifndef NDEBUG
#include <cstdio> // for debug printing
#endif // NDEBUG

// Testing 
unsigned char write_buffer[256] = { 0 };
unsigned char read_buffer[256] = { 0 };

bool COMPortFake::WriteDCB()
{
	if (connected == FALSE)
	{
#ifndef NDEBUG
		printf("COMPortFake::WriteDCB() Port not opened\n");
#endif // NDEBUG
		return FALSE;
	}


#ifndef NDEBUG
	printf("COMPortFake::WriteDCB() Write DCB success\n");
#endif // NDEBUG
	return TRUE; // Write DCB success
}

COMPortFake::COMPortFake(
	const char* name /* = "COM3" */,
	unsigned long baud /* = 9600 */,
	char parity /* = 'N' */,
	unsigned char dataBit /* = 8 */,
	unsigned char stopBit /* = 1 */)
{
	// Init class members
	connected = FALSE;

	// Check name argument
	if (name == NULL || *name == 0)
	{
		assert(("COMPortFake::Constructor() Incorrect device name", (name && *name)));
		throw("Incorrect device name");
	}
	// Copy port name
	strcpy_s(this->name, 9, name);

	SetConfig(baud, parity, dataBit, stopBit);

#ifndef NDEBUG
	printf("COMPortFake::Constructor() Created instance\n");
#endif // NDEBUG
}

COMPortFake::~COMPortFake()
{
	Close();
}

bool COMPortFake::Open()
{
	// Create windows device name
	char portFullName[16] = "\\\\.\\";
	strcpy_s(&portFullName[4], 12, name);
#ifndef NDEBUG
	printf("COMPortFake::Open() Full port name: %s\n", portFullName);
#endif // NDEBUG
	
	// Here port is totally opened
	connected = TRUE;

#ifndef NDEBUG
	printf("COMPortFake::Open() Port opened\n");
#endif // NDEBUG
	return TRUE;
}

bool COMPortFake::Close()
{	
	connected = FALSE;
#ifndef NDEBUG
	printf("COMPortFake::Close() Port closed\n");
#endif // NDEBUG
	return TRUE;
}

bool COMPortFake::SetConfig(
	unsigned long baud /* = 9600 */,
	char parity /* = 'N' */,
	unsigned char dataBit /* = 8 */,
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
	// Check parity argument
	if (parity == 'E' || parity == 'e' || parity == EVENPARITY) this->parity = EVENPARITY;
	else if (parity == 'O' || parity == 'o' || parity == ODDPARITY) this->parity = ODDPARITY;
	else this->parity = NOPARITY;
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
	// Check stop bits
	if (stopBit == 1) this->stopBit = ONESTOPBIT;
	else this->stopBit = TWOSTOPBITS;
	// if connected then write dcb block

#ifndef NDEBUG
	printf("COMPortFake::SetConfig() Config is set\n");
#endif // NDEBUG
	return TRUE;
}

bool COMPortFake::SetReadTimeouts(
	unsigned long interval /* = 0 */,
	unsigned long multiplier /* = 0 */,
	unsigned long constant /* = 1 */)
{
	// 0, 0, 1
	// return immediately with the bytes that have already been received,
	// even if no bytes have been received
	// 0, 0, 0
	// wait untill buffer is filled up to specified size
	// 1, 0, 0
	// If there are any bytes in the input buffer,
	// ReadFile returns immediately with the bytes in the buffer.
	// If there are no bytes in the input buffer,
	// ReadFile waits until a byte arrives and then returns immediately.
	// If no bytes arrive within the time specified by ReadTotalTimeoutConstant,
	// ReadFile times out.

#ifndef NDEBUG
	printf("COMPortFake::SetReadTimeouts() Timeouts is set\n");
#endif // NDEBUG
	return TRUE;
}

bool COMPortFake::ClearBuffers()
{

#ifndef NDEBUG
	printf("COMPortFake::ClearBuffers() Clearing buffers success\n");
#endif // NDEBUG
	return TRUE;
}

long COMPortFake::Write(unsigned char* buf, unsigned char length)
{
#ifndef NDEBUG
	printf("COMPortFake::Write() Writing %d bytes into port: 0x", length);
	for (int i = 0; i < length; i++)
		printf(" %02X", buf[i]);
	printf("\n");
#endif // NDEBUG

	if (length > 255) length = 255;
	memcpy(write_buffer, buf, length);
	//strcpy_s((char*)write_buffer, length, buf);


#ifndef NDEBUG
	printf("COMPortFake::Write() %d bytes written\n", length);
#endif // NDEBUG
	return length;
}

long COMPortFake::Read(unsigned char* buf, unsigned char length)
{
#ifndef NDEBUG
	printf("COMPortFake::Read() Reading %d bytes from port:", length);
#endif // NDEBUG

	Sleep(20); // Read delay
	if (length > 255) length = 255;

	// Here I can make comparison
	memcpy(buf, "Test message", length);
	//strncpy_s(buf, "Test message", length);
	//strncpy_s()

#ifndef NDEBUG
	printf(" 0x");
	for (int i = 0; i < length; i++)
		printf(" %02X", buf[i]);
	printf("\n");
	printf("COMPortFake::Read() %d bytes have read\n", length);
#endif // NDEBUG
	return length;
}
