#ifdef _I7188E_
#include ".\INC\COMPORT.H"
#else
#include "..\INC\COMPORT.H"
#endif

#include <string.h>	// for string data operations

//#define NDEBUG
#include <assert.h>
#ifndef NDEBUG
#include <stdio.h>	// for debug printing
#include <time.h>	// for measuring transfer operations time
#endif // NDEBUG

bool COMPort::WriteDCB()
{
	DCB dcb = { 0 };
	error = GetLastError();
	assert(("COMPort::WriteDCB() Some error in port", !error));
	if (error != 0) return false;
	if (!GetCommState(hCOM, &dcb))
	{
		assert(("COMPort::WriteDCB() Read DCB error", 0));
		return false;
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
	if (parity == NOPARITY) dcb.fParity = false;
	else dcb.fParity = true;
	dcb.Parity = parity;
	// Binary mode (it's the only supported on Windows anyway)
	dcb.fBinary = true;
	// Don't want errors to be blocking
	dcb.fAbortOnError = false;
	// No software handshaking
	dcb.fTXContinueOnXoff = true;
	dcb.fOutX = false;
	dcb.fInX = false;

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
		return false;
	}
#ifndef NDEBUG
	printf("COMPort::WriteDCB() Write DCB success\n");
#endif // NDEBUG
	return true; // Write DCB success
}

bool COMPort::TimeoutsSetup()
{
	COMMTIMEOUTS timeouts;
	if (!GetCommTimeouts(hCOM, &timeouts))
	{
		assert(("COMPort::TimeoutsSetup() Read timeouts error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("COMPort::TimeoutsSetup() Current timeouts:\n");
	printf("- ReadIntervalTimeout: %lu\n", timeouts.ReadIntervalTimeout);
	printf("- ReadTotalTimeoutMultiplier: %lu\n", timeouts.ReadTotalTimeoutMultiplier);
	printf("- ReadTotalTimeoutConstant: %lu\n", timeouts.ReadTotalTimeoutConstant);
	printf("- WriteTotalTimeoutMultiplier: %lu\n", timeouts.WriteTotalTimeoutMultiplier);
	printf("- WriteTotalTimeoutConstant: %lu\n", timeouts.WriteTotalTimeoutConstant);
#endif // NDEBUG

	timeouts.ReadIntervalTimeout = tInterval;
	timeouts.ReadTotalTimeoutMultiplier = tMultiplier;
	timeouts.ReadTotalTimeoutConstant = tConstant;

#ifndef NDEBUG
	printf("COMPort::TimeoutsSetup() New timeouts:\n");
	printf("- ReadIntervalTimeout: %lu\n", timeouts.ReadIntervalTimeout);
	printf("- ReadTotalTimeoutMultiplier: %lu\n", timeouts.ReadTotalTimeoutMultiplier);
	printf("- ReadTotalTimeoutConstant: %lu\n", timeouts.ReadTotalTimeoutConstant);
	printf("- WriteTotalTimeoutMultiplier: %lu\n", timeouts.WriteTotalTimeoutMultiplier);
	printf("- WriteTotalTimeoutConstant: %lu\n", timeouts.WriteTotalTimeoutConstant);
#endif // NDEBUG
	if (!SetCommTimeouts(hCOM, &timeouts))
	{
		assert(("COMPort::TimeoutsSetup() Set timeouts error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("COMPort::TimeoutsSetup() Timeouts is set\n");
#endif // NDEBUG
	return true;
}

COMPort::COMPort(
	const char* name /* = "COM3" */,
	unsigned long baud /* = 9600 */,
	unsigned char dataBit /* = 8 */,
	char parity /* = 'N' */,
	unsigned char stopBit /* = 1 */) :
	hCOM(INVALID_HANDLE_VALUE),
	error(0),
	tInterval(0),
	tMultiplier(0),
	tConstant(1)
{
	// Check name argument
	if (name == NULL || *name == 0)
	{
		assert(("COMPort::Constructor() Incorrect port name", (name && *name)));
		throw("Incorrect port name");
	}
	// Copy port name
	strcpy_s(this->name, strlen(name) + 1, name);
	// Set default config
	SetConfig(baud, dataBit, parity, stopBit);

#ifndef NDEBUG
	printf("COMPort::Constructor() Created instance 0x%p with params:\n", this);
	printf("- name: %s\n", name);
	printf("- baud: %u\n", baud);
	printf("- dataBit: %u\n", dataBit);
	printf("- parity: %c\n", parity);
	printf("- stopBit: %u\n", stopBit);
	printf("- tInterval: %u\n", tInterval);
	printf("- tMultiplier: %u\n", tMultiplier);
	printf("- tConstant: %u\n", tConstant);
#endif // NDEBUG
}

COMPort::COMPort(COMPort& other) :
	hCOM(other.hCOM),
	error(other.error),
	baud(other.baud),
	dataBit(other.dataBit),
	parity(other.parity),
	stopBit(other.stopBit),
	tInterval(other.tInterval),
	tMultiplier(other.tMultiplier),
	tConstant(other.tConstant)
{
	memcpy(name, other.name, 9);
	// release handle from other instance to prevent multiple access
	other.hCOM = INVALID_HANDLE_VALUE;
}

COMPort& COMPort::operator =(COMPort& other)
{
	if (this != &other)
	{
		hCOM = other.hCOM;
		error = other.error;
		memcpy(name, other.name, 9);
		baud = other.baud;
		dataBit = other.dataBit;
		parity = other.parity;
		stopBit = other.stopBit;
		tInterval = other.tInterval;
		tMultiplier = other.tMultiplier;
		tConstant = other.tConstant;
		// release handle from other instance to prevent multiple access
		other.hCOM = INVALID_HANDLE_VALUE;
	}
	return (*this);
}

COMPort::COMPort(COMPort&& other) noexcept :
	hCOM(other.hCOM),
	error(other.error),
	baud(other.baud),
	dataBit(other.dataBit),
	parity(other.parity),
	stopBit(other.stopBit),
	tInterval(other.tInterval),
	tMultiplier(other.tMultiplier),
	tConstant(other.tConstant)
{
	// copy port parameters from other instance into this
	memcpy(name, other.name, 9);
	// release handle from other instance to prevent multiple access
	other.hCOM = INVALID_HANDLE_VALUE;
}

COMPort& COMPort::operator =(COMPort&& other) noexcept
{
	if (this != &other)
	{
		// copy port parameters from other instance into this
		hCOM = other.hCOM;
		error = other.error;
		memcpy(name, other.name, 9);
		baud = other.baud;
		dataBit = other.dataBit;
		parity = other.parity;
		stopBit = other.stopBit;
		tInterval = other.tInterval;
		tMultiplier = other.tMultiplier;
		tConstant = other.tConstant;
		// release handle from other instance
		other.hCOM = INVALID_HANDLE_VALUE;
	}
	return (*this);
}

COMPort::~COMPort()
{
	Close();
#ifndef NDEBUG
	printf("COMPort::Destructor() Deleted instance 0x%p\n", this);
#endif // NDEBUG
}

bool COMPort::Open()
{
	// Create windows device name
	char portFullName[16] = "\\\\.\\";
	strcpy_s(&portFullName[4], strlen(name) + 1, name);
#ifndef NDEBUG
	printf("COMPort::Open() Full port name: %s\n", portFullName);
#endif // NDEBUG
	if (isOpen() == true) Close(); // close port if it is already opened by accident

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
		return false;
	}
	else if (error == ERROR_ACCESS_DENIED)
	{
		assert(("COMPort::Open() Port is used by another program", 0));
		return false;
	}
	else if (error != 0)
	{
		assert(("COMPort::Open() Other error in port opening", 0));
		return false;
	}
	// Here port is totally opened

	error = WriteDCB();
	if (error == 0) return false;
	error = TimeoutsSetup();
	if (error == 0) return false;
	// Here port parameters is set
	error = ClearReadBuffer();
	if (error == 0) return false;
#ifndef NDEBUG
	printf("COMPort::Open() Port opened (hCOM: %p)\n", hCOM);
#endif // NDEBUG
	return true;
}

bool COMPort::isOpen()
{
	DWORD portStat = 0; // not used here but required for GetCommModemStatus function
	return GetCommModemStatus(hCOM, &portStat);
}

bool COMPort::Close()
{
	bool closeState = 0;
#ifndef NDEBUG
	printf("COMPort::Close() Port closed (hCOM: %p)\n", hCOM);
#endif // NDEBUG
	closeState = CloseHandle(hCOM);
	hCOM = INVALID_HANDLE_VALUE;
	return closeState;
}

bool COMPort::SetConfig(
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
	// if opened then write parameters into DCB
	error = WriteDCB();
	if (error == 0) return false;
#ifndef NDEBUG
	printf("COMPort::SetConfig() Config is set\n");
#endif // NDEBUG
	return true;
}

bool COMPort::SetReadTimeouts(
	unsigned long interval /* = 0 */,
	unsigned long multiplier /* = 0 */,
	unsigned long constant /* = 1 */)
{
	tInterval = interval;
	tMultiplier = multiplier;
	tConstant = constant;
	// if not opened then return here
	if (isOpen() == false) return true;
	return TimeoutsSetup();
}

bool COMPort::ClearReadBuffer()
{
	if (isOpen() == false) return true;
	error = PurgeComm(hCOM, PURGE_RXABORT | PURGE_RXCLEAR);
	if (error == 0)
	{
		assert(("COMPort::ClearReadBuffer() Clearing read buffer error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("COMPort::ClearReadBuffer() Clearing read buffer success\n");
#endif // NDEBUG
	return true;
}

long COMPort::Write(unsigned char* buf, unsigned char length)
{
	if (isOpen() == false) Open();
#ifndef NDEBUG
	printf("COMPort::Write() Writing %d bytes into port: 0x", length);
	for (unsigned int i = 0; i < length; i++)
		printf(" %02X", buf[i]);
	printf("\n");
	clock_t start_time = clock();
#endif // NDEBUG
	DWORD n_bytes = 0;
	bool writeStatus = WriteFile(hCOM, buf, length, &n_bytes, NULL);
	// Check errors
	error = GetLastError();
	if (error == ERROR_ACCESS_DENIED)
	{
		assert(("COMPort::Write() Connection to port lost", 0));
		return -1;
	}
	if (writeStatus)
	{
#ifndef NDEBUG
		printf("COMPort::Write() %d bytes written in %ldms\n", n_bytes, (clock() - start_time));
#endif // NDEBUG
		return (long)n_bytes;
	}
	else
	{
#ifndef NDEBUG
		printf("COMPort::Write() Write failed error %u\n", error);
#endif // NDEBUG
		return -1;
	}
}

long COMPort::Read(unsigned char* buf, unsigned char length)
{
	if (isOpen() == false) Open();
#ifndef NDEBUG
	clock_t start_time = clock();
	printf("COMPort::Read() Reading %d bytes from port: 0x", length);
#endif // NDEBUG
	DWORD n_bytes = 0;
	bool readStatus = ReadFile(hCOM, buf, length, &n_bytes, NULL);
	// Check errors
	error = GetLastError();
	if (error == ERROR_ACCESS_DENIED)
	{
		assert(("COMPort::Read() Connection to port lost", 0));
		return -1;
	}
	if (readStatus)
	{
#ifndef NDEBUG
		for (unsigned int i = 0; i < n_bytes; i++)
			printf(" %02X", buf[i]);
		printf("\nCOMPort::Read() %d bytes have read in %ldms\n", n_bytes, (clock() - start_time));
#endif // NDEBUG
		return (long)n_bytes;
	}
	else
	{
#ifndef NDEBUG
		printf("COMPort::Read() Read failed error %u\n", error);
#endif // NDEBUG
		return -1;
	}
}
