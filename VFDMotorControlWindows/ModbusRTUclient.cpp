#include "ModbusRTUclient.h"

//#define NDEBUG
#include <cassert>
#ifndef NDEBUG
#include <cstdio>   // for debug printing
#include <ctime>    // for transfer time measure
#endif // NDEBUG

unsigned int ModbusRTUClient::CRC16(unsigned char* data, unsigned char length)
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

bool ModbusRTUClient::responseCRCCheck(unsigned char pduBytes)
{
	unsigned short rCRC;
	rCRC = rBuf[pduBytes + 2];					// CRC Hi
	rCRC = (rCRC << 8) | rBuf[pduBytes + 1];	// CRC Lo
	unsigned short rCalcCRC = CRC16(rBuf, (pduBytes + 1));
	return rCRC == rCalcCRC;
}

bool ModbusRTUClient::Transfer(unsigned char wPDUBytes, unsigned char rPDUBytes)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG

	wBuf[0] = devAddress;				// Server device address
	// Calculate CRC
	unsigned short wCRC = CRC16(wBuf, (wPDUBytes + 1));
	wBuf[wPDUBytes + 1] = wCRC & 0xFF;	// CRC Lo
	wBuf[wPDUBytes + 2] = wCRC >> 8;	// CRC Hi
	// Transmit request
	for (unsigned int attempt = 1; attempt <= transmitAttempts; attempt++)
	{
		// Write buffer to port
		long bytesWritten = COM.Write(wBuf, (wPDUBytes + 3));
		if (bytesWritten != (wPDUBytes + 3))
		{
#ifndef NDEBUG
			printf("ModbusRTUClient::Transfer() Attempt %u: Request write failed\n", attempt);
#endif // NDEBUG
			continue;
		}
		// Clear buffer
		if (COM.ClearReadBuffer() == 0)
		{
			assert(("ModbusRTUClient::Transfer() Clear buffer failed", 0));
			return false;
		}

		long bytesRead = COM.Read(rBuf, (rPDUBytes + 3));
		// Check receive errors
		if (bytesRead == -1)
		{
#ifndef NDEBUG
			printf("ModbusRTUClient::Transfer() Attempt %u: Response read failed\n", attempt);
#endif // NDEBUG
			continue;
		}
		if (bytesRead == 0)
		{
			assert(("ModbusRTUClient::Transfer() Transfer timeout", 0));
			return false;
		}
		if (bytesRead != (rPDUBytes + 3))
		{
			// 2 bytes of PDU into CRC check (error function + exception code)
			if ((bytesRead == 5) && responseCRCCheck(2) && (rBuf[1] > 0x80))
			{
				PrintException(attempt);
				continue;
			}
#ifndef NDEBUG
			printf("ModbusRTUClient::Transfer() Attempt %u: Bytes count mismatch\n", attempt);
#endif // NDEBUG
			// Increase timeout if the server is too slow
			COM.SetReadTimeouts((attempt + 1), 0, 1000);
			continue;
		}
		if (!responseCRCCheck(rPDUBytes))
		{
#ifndef NDEBUG
			printf("ModbusRTUClient::Transfer() Attempt %u: Frame CRC check error\n", attempt);
#endif // NDEBUG
			continue;
		}
		if (rBuf[0] == wBuf[0]) // Success transfer
		{
#ifndef NDEBUG
			printf("ModbusRTUClient::Transfer() Transfer success. Attempt: %u, time: %ldms\n",
				attempt, (clock() - start_time));
#endif // NDEBUG
			return true;
		}
		else
		{
#ifndef NDEBUG
			printf("ModbusRTUClient::Transfer() Attempt %u: Incorrect server address in response\n", attempt);
#endif // NDEBUG
			continue;
		}
	}

	assert(("ModbusRTUClient::Transfer() Attempts to transfer frame ended up", 0));
	return false;
}

void ModbusRTUClient::PrintException(unsigned int attempt)
{
	unsigned char excepCode = rBuf[2];
#ifndef NDEBUG
	printf("ModbusRTUClient::Transfer() Attempt: %u. Exception occurred: ", attempt);
	if (excepCode == 0x01)		printf("ILLEGAL FUNCTION\n");
	else if (excepCode == 0x02) printf("ILLEGAL DATA ADDRESS\n");
	else if (excepCode == 0x03) printf("ILLEGAL DATA VALUE\n");
	else if (excepCode == 0x04) printf("SERVER DEVICE FAILURE\n");
	else if (excepCode == 0x05) printf("ACKNOWLEDGE\n");
	else if (excepCode == 0x06) printf("SERVER DEVICE BUSY\n");
	else if (excepCode == 0x08) printf("MEMORY PARITY ERROR\n");
	else if (excepCode == 0x0A) printf("GATEWAY PATH UNAVAILABLE\n");
	else if (excepCode == 0x0B) printf("GATEWAY TARGET DEVICE FAILED TO RESPOND\n");
	else                        printf("OTHER EXCEPTION\n");
#endif // NDEBUG
}

#ifdef FAKE_PORT
ModbusRTUClient::ModbusRTUClient(unsigned char devAddress /* = 1 */,
	COMPortFake com /* = { "COM3", 19200, 8, 'E', 1 } */) :
	COM(com),
	transmitAttempts(5)
{
	memset(wBuf, 0, 256);
	memset(rBuf, 0, 256);
	// check device address
	if (devAddress > 247) // Modbus_over_serial_line_V1_02.pdf (chapter 2.2)
	{
		assert(("ModbusRTUClient::Constructor() Incorrect server device address", 0));
		throw("Incorrect server device address");
	}
	this->devAddress = devAddress;
	if (!COM.Open())
	{
		assert(("ModbusRTUClient::Constructor() Port open error", 0));
		throw("Port open error");
	}
	// Set wait response timeout to 1 second (default) and for read not full message
	// Modbus_over_serial_line_V1_02.pdf (chapter 2.4.1)
	if (!COM.SetReadTimeouts(1, 0, 1000))
	{
		assert(("ModbusRTUClient::Constructor() Error setting port timeouts", 0));
		throw("Error setting port timeouts");
	}
#ifndef NDEBUG
	printf("ModbusRTUClient::Constructor() Created instance 0x%p with params:\n", this);
	printf("- COM: 0x%p\n", &(this->COM));
	printf("- devAddress: %u\n", this->devAddress);
	printf("- transmitAttempts: %u\n", this->transmitAttempts);
#endif // NDEBUG
}
#else
ModbusRTUClient::ModbusRTUClient(unsigned char devAddress /* = 1 */,
	COMPort com /* = { "COM3", 19200, 8, 'E', 1 } */) :
	COM(com),
	transmitAttempts(5)
{
	memset(wBuf, 0, 256);
	memset(rBuf, 0, 256);
	// check device address
	if (devAddress > 247) // Modbus_over_serial_line_V1_02.pdf (chapter 2.2)
	{
		assert(("ModbusRTUClient::Constructor() Incorrect server device address", 0));
		throw("Incorrect server device address");
	}
	this->devAddress = devAddress;
	if (!COM.Open())
	{
		assert(("ModbusRTUClient::Constructor() Port open error", 0));
		throw("Port open error");
	}
	// Set wait response timeout to 1 second (default) and for read not full message
	// Modbus_over_serial_line_V1_02.pdf (chapter 2.4.1)
	if (!COM.SetReadTimeouts(1, 0, 1000))
	{
		assert(("ModbusRTUClient::Constructor() Error setting port timeouts", 0));
		throw("Error setting port timeouts");
	}
#ifndef NDEBUG
	printf("ModbusRTUClient::Constructor() Created instance 0x%p with params:\n", this);
	printf("- COM: 0x%p\n", &(this->COM));
	printf("- devAddress: %u\n", this->devAddress);
	printf("- transmitAttempts: %u\n", this->transmitAttempts);
#endif // NDEBUG
}
#endif

ModbusRTUClient::ModbusRTUClient(ModbusRTUClient& other) :
	COM(other.COM),
	devAddress(other.devAddress),
	transmitAttempts(other.transmitAttempts)
{
	memcpy(wBuf, other.wBuf, 256);
	memcpy(rBuf, other.rBuf, 256);
}

//ModbusRTUClient& ModbusRTUClient::operator =(ModbusRTUClient& other)
//{
//	if (this != &other)
//	{
//		COM = other.COM;
//		devAddress = other.devAddress;
//		transmitAttempts = other.transmitAttempts;
//		memcpy(wBuf, other.wBuf, 256);
//		memcpy(rBuf, other.rBuf, 256);
//	}
//	return (*this);
//}

ModbusRTUClient::ModbusRTUClient(ModbusRTUClient&& other) noexcept :
	COM(other.COM),
	devAddress(other.devAddress),
	transmitAttempts(other.transmitAttempts)
{
	memcpy(wBuf, other.wBuf, 256);
	memcpy(rBuf, other.rBuf, 256);
}

//ModbusRTUClient& ModbusRTUClient::operator =(ModbusRTUClient&& other) noexcept
//{
//	if (this != &other)
//	{
//		COM = other.COM;
//		devAddress = other.devAddress;
//		transmitAttempts = other.transmitAttempts;
//		memcpy(wBuf, other.wBuf, 256);
//		memcpy(rBuf, other.rBuf, 256);
//	}
//	return (*this);
//}

ModbusRTUClient::~ModbusRTUClient()
{
#ifndef NDEBUG
	printf("ModbusRTUClient::Destructor() Deleted instance 0x%p\n", this);
#endif // NDEBUG
}

bool ModbusRTUClient::ReadHoldingRegisters(
	unsigned short startAddress,
	unsigned char nRegisters,
	unsigned short* buf)
{
#ifndef NDEBUG
	printf("ModbusRTUClient::ReadHoldingRegisters() Reading %d registers from starting address 0x%04X\n",
		nRegisters, startAddress);
#endif // NDEBUG
	// Check input data
	if (nRegisters > 125)
	{
		assert(("ModbusRTUClient::ReadHoldingRegisters() Too many registers to read", 0));
		return false;
	}
	if ((startAddress + (nRegisters - 1)) < startAddress)
	{
		assert(("ModbusRTUClient::ReadHoldingRegisters() Address range exceeded", 0));
		return false;
	}
	// Create PDU frame // Modbus_Application_Protocol_V1_1b3.pdf (chapter 6.3)
	wBuf[1] = 0x03;					// Function code
	wBuf[2] = startAddress >> 8;	// Starting Address Hi
	wBuf[3] = startAddress & 0xFF;	// Starting Address Lo
	wBuf[4] = 0x00;					// N of Registers Hi
	wBuf[5] = nRegisters;			// N of Registers Lo

	// (Number of PDU bytes to read) = (Function code) + (Byte count) + 2 * (Quantity of Registers)
	if (!Transfer(5, (1 + 1 + 2 * nRegisters))) return false;

	// Check byte count
	if (rBuf[2] != (2 * nRegisters))
	{
		assert(("ModbusRTUClient::ReadHoldingRegisters() Byte count mismatch", 0));
		return false;
	}
#ifndef NDEBUG
	printf("ModbusRTUClient::ReadHoldingRegisters() Registers data: 0x");
#endif // NDEBUG
	// Copy bytes into 16-bit integers
	for (unsigned int i = 0; i < nRegisters; i++)
	{
		buf[i] = rBuf[2 * i + 3];
		buf[i] = (buf[i] << 8) | rBuf[2 * i + 4];
#ifndef NDEBUG
		printf(" %04X", buf[i]);
#endif // NDEBUG
	}
#ifndef NDEBUG
	printf("\n");
#endif // NDEBUG
	return true;
}

bool ModbusRTUClient::WriteSingleRegister(
	unsigned short regAddress,
	unsigned short regValue)
{
#ifndef NDEBUG
	printf("ModbusRTUClient::WriteSingleRegister() Write value 0x%04X into address 0x%04X\n",
		regValue, regAddress);
#endif // NDEBUG
	// Create PDU frame // Modbus_Application_Protocol_V1_1b3.pdf (chapter 6.6)
	wBuf[1] = 0x06;				// Function code
	wBuf[2] = regAddress >> 8;	// Register Address Hi
	wBuf[3] = regAddress & 0xFF;// Register Address Lo
	wBuf[4] = regValue >> 8;	// Register Value Hi
	wBuf[5] = regValue & 0xFF;	// Register Value Lo

	// Write and read PDU size is the same
	if (Transfer(5, 5) == 0) return false;

	// The normal response is an echo of the request. Check it
	if (memcmp(&(wBuf[1]), &(rBuf[1]), 5) != 0)
	{
		assert(("ModbusRTUClient::WriteSingleRegister() Response check mismatch", 0));
		return false;
	}
#ifndef NDEBUG
	printf("ModbusRTUClient::WriteSingleRegister() Write register success\n");
#endif // NDEBUG
	return true;
}

void ModbusRTUClient::SetNumberOfTransmitAttempts(unsigned char attempts /* = 1 */)
{
	if (attempts == 0) attempts = 1;
	this->transmitAttempts = attempts;
#ifndef NDEBUG
	printf("ModbusRTUClient::SetNumberOfTransmitAttempts() Set %u transmit attempts\n", attempts);
#endif // NDEBUG
}
