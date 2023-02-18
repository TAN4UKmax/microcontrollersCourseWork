#include "COMPort.h"
#include <cstring> // for string data operations

//#define NDEBUG
#include <cassert>
#ifndef NDEBUG
#include <cstdio> // for debug printing
#endif // NDEBUG

bool COMPort::WriteDCB()
{
	if (connected == FALSE)
	{
#ifndef NDEBUG
		printf("COMPort::WriteDCB() Port not opened\n");
#endif // NDEBUG
		return FALSE;
	}

	DCB dcb = { 0 };

	error = GetLastError();

	assert(("COMPort::WriteDCB() Some error in port", !error));
	if (error != 0) return FALSE;

	if (!GetCommState(hCOM, &dcb))
	{
		assert(("COMPort::WriteDCB() Read DCB error", 0));
		return FALSE;
	}

#ifndef NDEBUG
	printf("COMPort::WriteDCB() Current DCB params:\n");
	printf("- DCBlength: %lu\n", dcb.DCBlength);
	printf("- BaudRate: %lu\n", dcb.BaudRate);
	printf("- fBinary: %lu\n", dcb.BaudRate);
	printf("- fParity: %lu\n", dcb.fParity);
	printf("- fOutxCtsFlow: %lu\n", dcb.fOutxCtsFlow);
	printf("- fOutxDsrFlow: %lu\n", dcb.fOutxDsrFlow);
	printf("- fDtrControl: %lu\n", dcb.fDtrControl);
	printf("- fDsrSensitivity: %lu\n", dcb.fDsrSensitivity);
	printf("- fTXContinueOnXoff: %lu\n", dcb.fTXContinueOnXoff);
	printf("- fOutX: %lu\n", dcb.fOutX);
	printf("- fInX: %lu\n", dcb.fInX);
	printf("- fErrorChar: %lu\n", dcb.fErrorChar);
	printf("- fNull: %lu\n", dcb.fNull);
	printf("- fRtsControl: %lu\n", dcb.fRtsControl);
	printf("- fAbortOnError: %lu\n", dcb.fAbortOnError);
	printf("- fDummy2: %lu\n", dcb.fDummy2);
	printf("- wReserved: %lu\n", dcb.wReserved);
	printf("- XonLim: %lu\n", dcb.XonLim);
	printf("- XoffLim: %lu\n", dcb.XoffLim);
	printf("- ByteSize: %lu\n", dcb.ByteSize);
	printf("- Parity: %lu\n", dcb.Parity);
	printf("- StopBits: %lu\n", dcb.StopBits);
	printf("- XonChar: %lu\n", dcb.XonChar);
	printf("- XoffChar: %lu\n", dcb.XoffChar);
	printf("- ErrorChar: %lu\n", dcb.ErrorChar);
	printf("- EofChar: %lu\n", dcb.EofChar);
	printf("- EvtChar: %lu\n", dcb.EvtChar);
	printf("- wReserved1: %lu\n", dcb.wReserved1);
#endif // NDEBUG

	// Set DCB parameters
	dcb.BaudRate = baud;
	dcb.ByteSize = dataBit;
	dcb.StopBits = stopBit;
	if (parity == NOPARITY) dcb.fParity = FALSE;
	else dcb.fParity = TRUE;
	dcb.Parity = parity;
	// Binary mode (it's the only supported on Windows anyway)
	dcb.fBinary = TRUE;
	// Don't want errors to be blocking
	dcb.fAbortOnError = FALSE;
	// No software handshaking
	dcb.fTXContinueOnXoff = TRUE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;

#ifndef NDEBUG
	printf("COMPort::WriteDCB() New DCB params:\n");
	printf("- DCBlength: %lu\n", dcb.DCBlength);
	printf("- BaudRate: %lu\n", dcb.BaudRate);
	printf("- fBinary: %lu\n", dcb.BaudRate);
	printf("- fParity: %lu\n", dcb.fParity);
	printf("- fOutxCtsFlow: %lu\n", dcb.fOutxCtsFlow);
	printf("- fOutxDsrFlow: %lu\n", dcb.fOutxDsrFlow);
	printf("- fDtrControl: %lu\n", dcb.fDtrControl);
	printf("- fDsrSensitivity: %lu\n", dcb.fDsrSensitivity);
	printf("- fTXContinueOnXoff: %lu\n", dcb.fTXContinueOnXoff);
	printf("- fOutX: %lu\n", dcb.fOutX);
	printf("- fInX: %lu\n", dcb.fInX);
	printf("- fErrorChar: %lu\n", dcb.fErrorChar);
	printf("- fNull: %lu\n", dcb.fNull);
	printf("- fRtsControl: %lu\n", dcb.fRtsControl);
	printf("- fAbortOnError: %lu\n", dcb.fAbortOnError);
	printf("- fDummy2: %lu\n", dcb.fDummy2);
	printf("- wReserved: %lu\n", dcb.wReserved);
	printf("- XonLim: %lu\n", dcb.XonLim);
	printf("- XoffLim: %lu\n", dcb.XoffLim);
	printf("- ByteSize: %lu\n", dcb.ByteSize);
	printf("- Parity: %lu\n", dcb.Parity);
	printf("- StopBits: %lu\n", dcb.StopBits);
	printf("- XonChar: %lu\n", dcb.XonChar);
	printf("- XoffChar: %lu\n", dcb.XoffChar);
	printf("- ErrorChar: %lu\n", dcb.ErrorChar);
	printf("- EofChar: %lu\n", dcb.EofChar);
	printf("- EvtChar: %lu\n", dcb.EvtChar);
	printf("- wReserved1: %lu\n", dcb.wReserved1);
#endif // NDEBUG

	if (!SetCommState(hCOM, &dcb))
	{
		assert(("COMPort::WriteDCB() Write DCB error", 0));
		return FALSE;
	}

#ifndef NDEBUG
	printf("COMPort::WriteDCB() Write DCB success\n");
#endif // NDEBUG
	return TRUE; // Write DCB success
}

COMPort::COMPort(const char* name /* = "COM3" */,
	unsigned long baud /* = 9600 */,
	char parity /* = 'N' */,
	unsigned char dataBit /* = 8 */,
	unsigned char stopBit /* = 1 */)
{
	// Init class members
	hCOM = INVALID_HANDLE_VALUE;
	error = 0;
	connected = FALSE;

	// Check name argument
	if (name == NULL || *name == 0)
	{
		assert(("COMPort::Constructor() Incorrect device name", (name && *name)));
		throw("Incorrect device name");
	}
	// Copy port name
	strcpy_s(this->name, 9, name);

	SetConfig(baud, parity, dataBit, stopBit);

#ifndef NDEBUG
	printf("COMPort::Constructor() Created instance\n");
#endif // NDEBUG
}

COMPort::~COMPort()
{
	Close();
}

bool COMPort::Open()
{
	// Create windows device name
	char portFullName[16] = "\\\\.\\";
	strcpy_s(&portFullName[4], 12, name);

	// Try to open port
	hCOM = CreateFileA(portFullName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	// Handle errors
	error = GetLastError();
	if (error == ERROR_FILE_NOT_FOUND)
	{
		assert(("COMPort::Open() Device not connected", 0));
		// Connect device first
		//printf("Сначала подключите устройство.\n");
		return FALSE;
	}
	else if (error == ERROR_ACCESS_DENIED)
	{
		assert(("COMPort::Open() Port is used by another program", 0));
		return FALSE;
	}
	else if (error != 0)
	{
		assert(("COMPort::Open() Other error in port opening", 0));
		return FALSE;
	}
	// Here port is totally opened
	connected = TRUE;

	error = WriteDCB();
	if (error == 0)
	{
		assert(("COMPort::Open() Error setting config to DCB", 0));
		return FALSE;
	}
	error = SetReadTimeouts(0, 0, 1);
	if (error == 0)
	{
		assert(("COMPort::Open() Error setting timeouts", 0));
		return FALSE;
	}
	// Here port parameters is set

	error = ClearBuffers();
	if (error == 0)
	{
		assert(("COMPort::Open() Clearing buffers error", 0));
		return FALSE;
	}
#ifndef NDEBUG
	printf("COMPort::Open() Port opened\n");
#endif // NDEBUG
	return TRUE;
}

bool COMPort::Close()
{
	bool closeState = 0;
	connected = FALSE;
	closeState = CloseHandle(hCOM);
	hCOM = INVALID_HANDLE_VALUE;
	connected = FALSE;
#ifndef NDEBUG
	printf("COMPort::Close() Port closed\n");
#endif // NDEBUG
	return closeState;
}

bool COMPort::SetConfig(unsigned long baud /* = 9600 */,
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
	error = WriteDCB();
	if (error == 0)
	{
#ifndef NDEBUG
		printf("COMPort::SetConfig() Error setting config to DCB\n");
#endif // NDEBUG
		return FALSE;
	}
#ifndef NDEBUG
	printf("COMPort::SetConfig() Config is set\n");
#endif // NDEBUG
	return TRUE;
}

bool COMPort::SetReadTimeouts(unsigned long interval /* = 0 */,
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

	COMMTIMEOUTS timeouts;
	if (!GetCommTimeouts(hCOM, &timeouts))
	{
		assert(("COMPort::SetReadTimeouts() Read timeouts error", 0));
		return FALSE;
	}

#ifndef NDEBUG
	printf("COMPort::SetReadTimeouts() Current timeouts:\n");
	printf("- ReadIntervalTimeout: %lu\n", timeouts.ReadIntervalTimeout);
	printf("- ReadTotalTimeoutMultiplier: %lu\n", timeouts.ReadTotalTimeoutMultiplier);
	printf("- ReadTotalTimeoutConstant: %lu\n", timeouts.ReadTotalTimeoutConstant);
	printf("- WriteTotalTimeoutMultiplier: %lu\n", timeouts.WriteTotalTimeoutMultiplier);
	printf("- WriteTotalTimeoutConstant: %lu\n", timeouts.WriteTotalTimeoutConstant);
#endif // NDEBUG

	timeouts.ReadIntervalTimeout = interval;
	timeouts.ReadTotalTimeoutMultiplier = multiplier;
	timeouts.ReadTotalTimeoutConstant = constant;

#ifndef NDEBUG
	printf("COMPort::SetReadTimeouts() New timeouts:\n");
	printf("- ReadIntervalTimeout: %lu\n", timeouts.ReadIntervalTimeout);
	printf("- ReadTotalTimeoutMultiplier: %lu\n", timeouts.ReadTotalTimeoutMultiplier);
	printf("- ReadTotalTimeoutConstant: %lu\n", timeouts.ReadTotalTimeoutConstant);
	printf("- WriteTotalTimeoutMultiplier: %lu\n", timeouts.WriteTotalTimeoutMultiplier);
	printf("- WriteTotalTimeoutConstant: %lu\n", timeouts.WriteTotalTimeoutConstant);
#endif // NDEBUG

	if (!SetCommTimeouts(hCOM, &timeouts))
	{
		assert(("COMPort::SetReadTimeouts() Set timeouts error", 0));
		return FALSE;
	}
#ifndef NDEBUG
	printf("COMPort::SetReadTimeouts() Timeouts is set\n");
#endif // NDEBUG
	return TRUE;
}

bool COMPort::ClearBuffers()
{
	error = PurgeComm(hCOM, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
	if (error == 0)
	{
		assert(("COMPort::ClearBuffers() Clearing buffers error", 0));
		return FALSE;
	}
#ifndef NDEBUG
	printf("COMPort::ClearBuffers() Clearing buffers success\n");
#endif // NDEBUG
	return TRUE;
}

long COMPort::Write(char* buf, unsigned char length)
{
#ifndef NDEBUG
	printf("COMPort::Write() Writing %d bytes into port: 0x", length);
	for (int i = 0; i < length; i++)
		printf(" %02X", buf[i]);
	printf("\n");
#endif // NDEBUG

	DWORD n_bytes = 0;
	bool writeStatus = WriteFile(hCOM, buf, length, &n_bytes, NULL);

	error = GetLastError();
	if (error == ERROR_ACCESS_DENIED)
	{
	assert(("COMPort::Write() Connection to port lost", 0));
	return FALSE;
	}

#ifndef NDEBUG
	printf("COMPort::Write() %d bytes written\n", n_bytes);
#endif // NDEBUG

	if (writeStatus)
		return (long)n_bytes;
	else
		return -1;
}

long COMPort::Read(char* buf, unsigned char length)
{
#ifndef NDEBUG
	printf("COMPort::Read() Reading %d bytes from port:", length);
#endif // NDEBUG

	DWORD n_bytes = 0;
	bool readStatus = ReadFile(hCOM, buf, length, &n_bytes, NULL);

	error = GetLastError();
	if (error == ERROR_ACCESS_DENIED)
	{
		assert(("COMPort::Read() Connection to port lost", 0));
		return FALSE;
	}

#ifndef NDEBUG
	printf(" 0x");
	for (int i = 0; i < n_bytes; i++)
		printf(" %02X", buf[i]);
	printf("\n");
	printf("COMPort::Read() %d bytes have read\n", n_bytes);
#endif // NDEBUG
	if (readStatus)
		return (long)n_bytes;
	else
		return -1;
}
