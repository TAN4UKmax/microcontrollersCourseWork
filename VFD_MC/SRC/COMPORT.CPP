#include ".\INC\COMPORT.H"	// 7188EX uPAC

#include <string.h>	// for string data operations

//#define NDEBUG
#include <assert.h>

char COMPort::PortSetup(void)
{
	char status = 0;
	// Set baud here
	status = SetBaudrate(port, baud);
	if (status != 0)
	{
#ifndef NDEBUG
		Print("COMPort::PortSetup() Set baudrate error %d\n", status);
#endif // NDEBUG
		return 0;
	}
#ifndef NDEBUG
	Print("COMPort::PortSetup() Set baudrate %lu success\n", baud);
#endif // NDEBUG

	status = SetDataFormat(port, dataBit, parity, stopBit);
	if (status != 0)
	{
#ifndef NDEBUG
		Print("COMPort::PortSetup() Set data format error %d\n", status);
#endif // NDEBUG
		return 0;
	}
#ifndef NDEBUG
	Print("COMPort::PortSetup() New data format:\n");
	Print("- dataBit: %d\n", dataBit);
	Print("- parity: %d\n", parity);
	Print("- stopBit: %d\n", stopBit);
#endif // NDEBUG
	return 1; // Setup success
}

COMPort::COMPort(
	const char* name /* = "COM2" */,
	unsigned long baud /* = 9600 */,
	char dataBit /* = 8 */,
	char parity /* = 'N' */,
	char stopBit /* = 1 */) :
	port(2),
	opened(0),
	tInterval(0),
	tMultiplier(0),
	tConstant(1)
{
	// Check name argument
	if (name == 0 || *name == 0)
	{
		assert(("COMPort::Constructor() Incorrect port name", (name && *name)));
	}
	// Copy port name
	strcpy(this->name, name);
	// Set default config
	SetConfig(baud, dataBit, parity, stopBit);

#ifndef NDEBUG
	Print("COMPort::Constructor() Created instance 0x%p with params:\n", this);
	Print("- name: %s\n", name);
	Print("- baud: %u\n", baud);
	Print("- dataBit: %u\n", dataBit);
	Print("- parity: %c\n", parity);
	Print("- stopBit: %u\n", stopBit);
	Print("- tInterval: %u\n", tInterval);
	Print("- tMultiplier: %u\n", tMultiplier);
	Print("- tConstant: %u\n", tConstant);
#endif // NDEBUG
}

COMPort::COMPort(COMPort& other) :
	port(other.port),
	opened(other.opened),
	baud(other.baud),
	dataBit(other.dataBit),
	parity(other.parity),
	stopBit(other.stopBit),
	tInterval(other.tInterval),
	tMultiplier(other.tMultiplier),
	tConstant(other.tConstant)
{
	memcpy(name, other.name, 12);
	// release handle from other instance to prevent multiple access
	other.port = -1;
}

COMPort& COMPort::operator =(COMPort& other)
{
	if (this != &other)
	{
		port = other.port;
		memcpy(name, other.name, 12);
		baud = other.baud;
		dataBit = other.dataBit;
		parity = other.parity;
		stopBit = other.stopBit;
		tInterval = other.tInterval;
		tMultiplier = other.tMultiplier;
		tConstant = other.tConstant;
		// release handle from other instance to prevent multiple access
		other.port = -1;
	}
	return (*this);
}

COMPort::~COMPort()
{
	Close();
#ifndef NDEBUG
	Print("COMPort::Destructor() Deleted instance 0x%p\n", this);
#endif // NDEBUG
}

char COMPort::Open(void)
{
	// Find number of port from name (ASCII string to number)
	for (char i = 0, n = strlen(name); i <= n; i++)
	{
		if ((name[i] >= '0') && (name[i] <= '8'))
		{
			port = name[i] - '0';
			break;
		}
	}
	// close port if it is already opened by accident
	if (isOpen() == 1) Close();
	char error = 0;
	// Open now with default parameters which will be changed later
	error = InstallCom(port, 9600, 8, 0, 1); // 0 on sucess

	if (error != 0)
	{
		assert(("COMPort::Open() Open port failed", 0));
		opened = 0;
		return 0;
	}
	// Here port is totally opened
	opened = 1;

	error = PortSetup(); // Set parameters here
	if (error == 0) return 0;
	// Here port parameters is set
	error = ClearReadBuffer(); // clear buffer here
	if (error == 0) return 0;
#ifndef NDEBUG
	Print("COMPort::Open() Port opened (COM%d)\n", port);
#endif // NDEBUG
	return 1;
}

char COMPort::isOpen(void)
{
	return opened;
}

char COMPort::Close(void)
{
	char status = RestoreCom(port);
	if (status != 0) return 0;
	opened = 0;
#ifndef NDEBUG
	Print("COMPort::Close() Port closed (COM%d)\n", port);
#endif // NDEBUG
	return 1;
}

char COMPort::SetConfig(
	unsigned long baud /* = 9600 */,
	char dataBit /* = 8 */,
	char parity /* = 'N' */,
	char stopBit /* = 1 */)
{
	// Check baud argument
	switch (baud) {
	case 110:
	case 300:
	case 600:
	case 1200:
	case 2400:
	case 4800:
	case 9600:
	case 14400:
	case 19200:
	case 38400:
	case 57600:
	case 115200:
		this->baud = baud;
		break;
	default:
		this->baud = 9600;
		break;
	}
	// Check data bits
	switch (dataBit) {
	case 7:
		this->dataBit = 7;
		break;
	case 8:
	default:
		this->dataBit = 8;
		break;
	}
	// Check parity argument
	if (parity == 'E' || parity == 'e' || parity == 1) this->parity = 1;
	else if (parity == 'O' || parity == 'o' || parity == 2) this->parity = 2;
	else this->parity = 0; // 'N' parity
	// Check stop bits
	if (stopBit == 2) this->stopBit = 2;
	else this->stopBit = 1;
	// if not opened then return here
	if (isOpen() == 0) return 1;
	// if opened then write parameters into internal structure
	char error = PortSetup();
	if (error == 0) return 0;
#ifndef NDEBUG
	Print("COMPort::SetConfig() Config is set\n");
#endif // NDEBUG
	return 1;
}

char COMPort::SetReadTimeouts(
	unsigned long interval /* = 0 */,
	unsigned long multiplier /* = 0 */,
	unsigned long constant /* = 1 */)
{
	tInterval = interval;
	tMultiplier = multiplier;
	tConstant = constant;
	return 1;
}

char COMPort::ClearReadBuffer(void)
{
	if (isOpen() == 0) return 1;
	char error = ClearCom(port);

	if (error != 0)
	{
		assert(("COMPort::ClearReadBuffer() Clearing read buffer error", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("COMPort::ClearReadBuffer() Clearing read buffer success\n");
#endif // NDEBUG
	return 1;
}

long COMPort::Write(unsigned char* buf, unsigned char length)
{
	if (isOpen() == 0) Open();
#ifndef NDEBUG
	Print("COMPort::Write() Writing %d bytes into port: 0x", length);
	for (unsigned char i = 0; i < length; i++)
		Print(" %02X", buf[i]);
	Print("\n");
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG

	// Send data
	int n_bytes = ToComBufn(port, (char*)buf, length);
	// Possible returns:
	// positive number of data that has sent
	// 0 no data sent
	// -1 send error
	if (n_bytes != length)
	{
#ifndef NDEBUG
		Print("COMPort::Write() Write failed error %d\n", n_bytes);
#endif // NDEBUG
		return -1;
	}
	// Wait all data in the output buffer of the software
	// and the hardware of the COM Port to be sent.
	char error = WaitTransmitOver(port);
	if (error != 0)
	{
#ifndef NDEBUG
		Print("COMPort::Write() Write failed error %d\n", error);
#endif // NDEBUG
		return -1;
	}

#ifndef NDEBUG
	Print("COMPort::Write() %d bytes written in %ldms\n", n_bytes, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return (long)n_bytes;
}

long COMPort::Read(unsigned char* buf, unsigned char length)
{
	if (isOpen() == 0) Open();
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
	Print("COMPort::Read() Reading %d bytes from port: 0x", length);
#endif // NDEBUG

	// Wait common timeout
	unsigned long commonTimeout = tMultiplier * length + tConstant;
	unsigned long readTime = GetTimeTicks();
	while (IsCom(port) == 0)
	{
		if ((GetTimeTicks() - readTime) > commonTimeout)
		{
#ifndef NDEBUG
			Print("\nCOMPort::Read() Read timeout\n");
#endif // NDEBUG
			return 0; // read timeout
		}
	}

	// Read port
	int n_bytes = 0;
	if (tInterval != 0)
	{
		// Here is reading with timeout realization
		// timeout is refreshed when new byte comes
		int bufPos = 0;
		readTime = GetTimeTicks();
		while (bufPos < length)
		{
			if (IsCom(port)) // read byte from port
			{
				buf[bufPos] = ReadCom(port);
				bufPos++;
				readTime = GetTimeTicks(); // refresh data timeout
			}
			if ((GetTimeTicks() - readTime) > tInterval) break;

		}
		n_bytes = bufPos;
	}
	else
	{
		// Just read all the data from the buffer immediately
		n_bytes = ReadComn(port, buf, length);
		if (n_bytes == -1)
		{
#ifndef NDEBUG
			Print("\nCOMPort::Read() Read failed\n");
#endif // NDEBUG
			return -1;
		}
	}

#ifndef NDEBUG
	for (unsigned char i = 0; i < n_bytes; i++)
		Print(" %02X", buf[i]);
	Print("\nCOMPort::Read() %d bytes have read in %ldms\n", n_bytes, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return (long)n_bytes;
}
