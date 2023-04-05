#include ".\INC\MBRTUCL.H" // 7188EX uPAC

#include <string.h>

//#define NDEBUG
#include <assert.h>

unsigned int ModbusRTUClient::CRC16(unsigned char* data, unsigned char length)
{
	int i = 0;
	unsigned int CRC16 = 0xFFFF;
	while (length > i)
	{
		CRC16 = CRC16 ^ data[i];
		for (char j = 0; j < 8; j++)
		{
			if (CRC16 & 0x01)
			{
				CRC16 = (CRC16 >> 1) ^ 0xA001;
			}
			else
			{
				CRC16 = CRC16 >> 1;
			}
		}
		i = i + 1;
	}
	return CRC16;
}

char ModbusRTUClient::responseCRCCheck(unsigned char pduBytes)
{
	unsigned short rCRC;
	rCRC = rBuf[pduBytes + 2];					// CRC Hi
	rCRC = (rCRC << 8) | rBuf[pduBytes + 1];	// CRC Lo
	unsigned short rCalcCRC = CRC16(rBuf, (pduBytes + 1));
	return rCRC == rCalcCRC;
}

char ModbusRTUClient::Transfer(unsigned char wPDUBytes, unsigned char rPDUBytes)
{
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	// Fill request ADU
	wBuf[0] = devAddress;				// Server device address
	// Calculate CRC
	unsigned short wCRC = CRC16(wBuf, (wPDUBytes + 1));
	wBuf[wPDUBytes + 1] = wCRC & 0xFF;	// CRC Lo
	wBuf[wPDUBytes + 2] = wCRC >> 8;	// CRC Hi
	// Transfer frame
	for (unsigned char attempt = 1; attempt <= transmitAttempts; attempt++)
	{
		// Write buffer to port
		long bytesWritten = COM.Write(wBuf, (wPDUBytes + 3));
		if (bytesWritten != (wPDUBytes + 3))
		{
#ifndef NDEBUG
			Print("ModbusRTUClient::Transfer() Attempt %u: Request write failed\n", attempt);
#endif // NDEBUG
			continue;
		}
		// Clear buffer
		if (COM.ClearReadBuffer() == 0)
		{
#ifndef NDEBUG
			Print("ModbusRTUClient::Transfer() Attempt %u: Clear buffer failed\n", attempt);
#endif // NDEBUG
			continue;
		}

		long bytesRead = COM.Read(rBuf, (rPDUBytes + 3));
		// Check receive errors
		if (bytesRead == -1)
		{
#ifndef NDEBUG
			Print("ModbusRTUClient::Transfer() Attempt %u: Response read failed\n", attempt);
#endif // NDEBUG
			continue;
		}
		if (bytesRead == 0)
		{
			assert(("ModbusRTUClient::Transfer() Transfer timeout", 0));
			return 0;
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
			Print("ModbusRTUClient::Transfer() Attempt %u: Bytes count mismatch\n", attempt);
#endif // NDEBUG
			// Increase timeout if the server is too slow
			COM.SetReadTimeouts((attempt * 2), 0, 1000);
			continue;
		}
		if (!responseCRCCheck(rPDUBytes))
		{
#ifndef NDEBUG
			Print("ModbusRTUClient::Transfer() Attempt %u: Frame CRC check error\n", attempt);
#endif // NDEBUG
			continue;
		}
		if (rBuf[0] == wBuf[0]) // Success transfer
		{
#ifndef NDEBUG
			Print("ModbusRTUClient::Transfer() Transfer success. Attempt: %u, time: %ldms\n",
				attempt, (GetTimeTicks() - start_time));
#endif // NDEBUG
			return 1;
		}
		else
		{
#ifndef NDEBUG
			Print("ModbusRTUClient::Transfer() Attempt %u: Incorrect server address in response\n", attempt);
#endif // NDEBUG
			continue;
		}
	}

	assert(("ModbusRTUClient::Transfer() Attempts to transfer frame ended up", 0));
	return 0;
}

void ModbusRTUClient::PrintException(unsigned char attempt)
{
	unsigned char excepCode = rBuf[2];
	Print("ModbusRTUClient::Transfer() Attempt: %u. Exception occurred: ", attempt);
	if (excepCode == 0x01)		Print("ILLEGAL FUNCTION\n");
	else if (excepCode == 0x02) Print("ILLEGAL DATA ADDRESS\n");
	else if (excepCode == 0x03) Print("ILLEGAL DATA VALUE\n");
	else if (excepCode == 0x04) Print("SERVER DEVICE FAILURE\n");
	else if (excepCode == 0x05) Print("ACKNOWLEDGE\n");
	else if (excepCode == 0x06) Print("SERVER DEVICE BUSY\n");
	else if (excepCode == 0x08) Print("MEMORY PARITY ERROR\n");
	else if (excepCode == 0x0A) Print("GATEWAY PATH UNAVAILABLE\n");
	else if (excepCode == 0x0B) Print("GATEWAY TARGET DEVICE FAILED TO RESPOND\n");
	else                        Print("OTHER EXCEPTION\n");
}

ModbusRTUClient::ModbusRTUClient(unsigned char devAddress, COMPort com) :
	COM(com),
	transmitAttempts(5)
{
	memset(wBuf, 0, 256);
	memset(rBuf, 0, 256);
	// check device address
	if (devAddress > 247) // Modbus_over_serial_line_V1_02.pdf (chapter 2.2)
	{
		assert(("ModbusRTUClient::Constructor() Incorrect server device address", 0));
	}
	this->devAddress = devAddress;
	if (!COM.Open())
	{
		assert(("ModbusRTUClient::Constructor() Port open error", 0));
	}
	// Set wait response timeout to 1 second (default) and for read not full message
	// Modbus_over_serial_line_V1_02.pdf (chapter 2.4.1)
	if (!COM.SetReadTimeouts(2, 0, 1000))
	{
		assert(("ModbusRTUClient::Constructor() Error setting port timeouts", 0));
	}
#ifndef NDEBUG
	Print("ModbusRTUClient::Constructor() Created instance 0x%p with params:\n", this);
	Print("- COM: 0x%p\n", &(this->COM));
	Print("- devAddress: %u\n", this->devAddress);
	Print("- transmitAttempts: %u\n", this->transmitAttempts);
#endif // NDEBUG
}

ModbusRTUClient::ModbusRTUClient(ModbusRTUClient& other) :
	COM(other.COM),
	devAddress(other.devAddress),
	transmitAttempts(other.transmitAttempts)
{
	memcpy(wBuf, other.wBuf, 256);
	memcpy(rBuf, other.rBuf, 256);
}

ModbusRTUClient::~ModbusRTUClient()
{
#ifndef NDEBUG
	Print("ModbusRTUClient::Destructor() Deleted instance 0x%p\n", this);
#endif // NDEBUG
}

char ModbusRTUClient::ReadHoldingRegisters(
	unsigned short startAddress,
	unsigned char nRegisters,
	unsigned short* buf)
{
#ifndef NDEBUG
	Print("ModbusRTUClient::ReadHoldingRegisters() Reading %d registers from starting address 0x%04X\n",
		nRegisters, startAddress);
#endif // NDEBUG
	// Check input data
	if ((nRegisters > 125) || (nRegisters < 1))
	{
		assert(("ModbusRTUClient::ReadHoldingRegisters() Insufficient quality of registers", 0));
		return 0;
	}
	if ((startAddress + (nRegisters - 1)) < startAddress)
	{
		assert(("ModbusRTUClient::ReadHoldingRegisters() Address range exceeded", 0));
		return 0;
	}
	// Create PDU frame // Modbus_Application_Protocol_V1_1b3.pdf (chapter 6.3)
	wBuf[1] = 0x03;					// Function code
	wBuf[2] = startAddress >> 8;	// Starting Address Hi
	wBuf[3] = startAddress & 0xFF;	// Starting Address Lo
	wBuf[4] = 0x00;					// N of Registers Hi
	wBuf[5] = nRegisters;			// N of Registers Lo

	// (Number of PDU bytes to read) = (Function code) + (Byte count) + 2 * (Quantity of Registers)
	if (!Transfer(5, (1 + 1 + 2 * nRegisters))) return 0;

	// Check byte count
	if (rBuf[2] != (2 * nRegisters))
	{
		assert(("ModbusRTUClient::ReadHoldingRegisters() Byte count mismatch", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("ModbusRTUClient::ReadHoldingRegisters() Registers data: 0x");
#endif // NDEBUG
	// Copy bytes into 16-bit integers
	for (unsigned char i = 0; i < nRegisters; i++)
	{
		buf[i] = rBuf[2 * i + 3];
		buf[i] = (buf[i] << 8) | rBuf[2 * i + 4];
#ifndef NDEBUG
		Print(" %04X", buf[i]);
#endif // NDEBUG
	}
#ifndef NDEBUG
	Print("\n");
#endif // NDEBUG
	return 1;
}

char ModbusRTUClient::WriteSingleRegister(
	unsigned short regAddress,
	unsigned short regValue)
{
#ifndef NDEBUG
	Print("ModbusRTUClient::WriteSingleRegister() Write value 0x%04X into address 0x%04X\n",
		regValue, regAddress);
#endif // NDEBUG
	// Create PDU frame // Modbus_Application_Protocol_V1_1b3.pdf (chapter 6.6)
	wBuf[1] = 0x06;				// Function code
	wBuf[2] = regAddress >> 8;	// Register Address Hi
	wBuf[3] = regAddress & 0xFF;// Register Address Lo
	wBuf[4] = regValue >> 8;	// Register Value Hi
	wBuf[5] = regValue & 0xFF;	// Register Value Lo

	// Write and read PDU size is the same
	if (!Transfer(5, 5)) return 0;

	// The normal response is an echo of the request. Check it
	if (memcmp(&(wBuf[1]), &(rBuf[1]), 5) != 0)
	{
		assert(("ModbusRTUClient::WriteSingleRegister() Response check mismatch", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("ModbusRTUClient::WriteSingleRegister() Write register success\n");
#endif // NDEBUG
	return 1;
}

void ModbusRTUClient::SetNumberOfTransmitAttempts(unsigned char attempts /* = 1 */)
{
	if (attempts == 0) attempts = 1;
	this->transmitAttempts = attempts;
#ifndef NDEBUG
	Print("ModbusRTUClient::SetNumberOfTransmitAttempts() Set %u transmit attempts\n", attempts);
#endif // NDEBUG
}
