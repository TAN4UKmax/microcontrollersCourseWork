/**
 * @file ModbusRTUClient.h
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief Class for manage ModbusRTU protocol
 * @version 0.1
 * @date 2023-02-18
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */

#ifndef MODBUSRTUCLIENT_H
#define MODBUSRTUCLIENT_H

//#define FAKE_PORT // uncomment it for use fake port and test modbus

#ifdef FAKE_PORT
#include "COMPortFake.h"
#else
#include "COMPort.h"
#endif

class ModbusRTUClient
{
private:
#ifdef FAKE_PORT
	COMPortFake     COM;        // Instance of fake COM port class
#else
	COMPort         COM;        // Instance of COM port class
#endif
	unsigned char   devAddress; // Server device address
	// Number of repeated transmit attempts when transmit fails (default: 5)
	unsigned char   transmitAttempts;
	unsigned char   wBuf[256];  // Buffer for write frame
	unsigned char   rBuf[256];  // Buffer for read frame

	/**
	 * @brief Calculates CRC16
	 * 
	 * Step 1: Load a 16-bit register (called CRC register) with FFFFH.
	 * Step 2: Exclusive OR the first 8-bit byte of the command message
	 * with the low order byte of the 16-bit CRC register,
	 * putting the result in the CRC register. 
	 * Step 3: Examine the LSB of CRC register. 
	 * Step 4: If the LSB of CRC register is 0, shift the CRC register
	 * one bit to the right with MSB zero filling, then repeat step 3.
	 * If the LSB of CRC register is 1, shift the CRC register
	 * one bit to the right with MSB zero filling, Exclusive OR the
	 * CRC register with the polynomial value A001H, then repeat step 3.
	 * Step 5: Repeat step 3 and 4 until eight shifts have been performed.
	 * When this is done, a complete 8-bit byte will have been processed.
	 * Step 6: Repeat step 2 to 5 for the next 8-bit byte of the command message.
	 * Continue doing this until all bytes have been processed.
	 * The final contents of the CRC register are the CRC value.
	 * When transmitting the CRC value in the message,
	 * the upper and lower bytes of the CRC value must be swapped,
	 * i.e. the lower order byte will be transmitted first.
	 *
	 * @param data[in]		- a pointer to the message buffer
	 * @param length[in]	- the message buffer length
	 * @return unsigned int - calculated CRC
	 */
	unsigned int CRC16(unsigned char* data, unsigned char length);

	/**
	 * @brief Check CRC of response message
	 *
	 * @param pduBytes[in]	- PDU size in bytes
	 * @return true			- If frame CRC check success
	 * @return false		- If CRC check mismatch
	 */
	bool responseCRCCheck(unsigned char pduBytes);

	/**
	 * @brief Transfers one frame to server.
	 * Creares ADU frame (Modbus_over_serial_line_V1_02.pdf).
	 * Writes request into port.
	 * Reads response (with timeout check)
	 * Checks frame using CRC
	 * Check if exception occurred and prints it
	 * Returns result if frame is correct
	 *
	 * @param wPDUBytes[in]	- Number of PDU bytes to write
	 * @param rPDUBytes[in]	- Number of PDU bytes to read
	 * @return true			- If transfer success
	 * @return false		- If some error occurred
	 */
	bool Transfer(unsigned char wPDUBytes, unsigned char rPDUBytes);

	/**
	 * @brief Print Modbus exception by its code
	 *
	 */
	void PrintException(unsigned int attempt);
public:
#ifdef FAKE_PORT
	/**
	 * @brief Construct a new ModbusRTUClient object with fake COM port object for testing.
	 * Setup port communication parameters
	 *
	 * @param devAddress[in]	- Modbus server address. (Range: 0-247, Default: 1)
	 * @param com[in]           - COMPortFake class instance with its parameters
	 * (Defaults according to: Modbus_over_serial_line_V1_02.pdf)
	 */
	ModbusRTUClient(unsigned char devAddress = 1,
		COMPortFake com = { "COM3", 19200, 8, 'E', 1 });
#else
	/**
	 * @brief Construct a new ModbusRTUClient object.
	 * Setup port communication parameters
	 *
	 * @param devAddress[in]    - Modbus server address. (Range: 0-247, Default: 1)
	 * @param com[in]           - COMPort class instance with its parameters
	 * (Defaults according to: Modbus_over_serial_line_V1_02.pdf)
	 */
	ModbusRTUClient(unsigned char devAddress = 1,
		COMPort com = { "COM3", 19200, 8, 'E', 1 });
#endif

	/**
	 * @brief Copy constructor for ModbusRTUClient.
	 * Is necessary for correct handle transfers and share resourse.
	 *
	 * @param other[in]     - ModbusRTUClient object that will be copied into current instance
	 */
	ModbusRTUClient(ModbusRTUClient& other);

	/**
	 * @brief Move constructor for ModbusRTUClient.
	 * Is necessary for correct handle transfers and share resourse.
	 *
	 * @param other[in]     - ModbusRTUClient object that will be moved into current instance
	 */
	ModbusRTUClient(ModbusRTUClient&& other) noexcept;

	/**
	 * @brief Destroy the Modbus RTU Client object
	 *
	 */
	~ModbusRTUClient();

	/**
	 * @brief Read Holding Registers (0x03 Function code).
	 * Read the contents of a contiguous block of holding registers in a remote device.
	 * ILLEGAL DATA ADDRESS error occurs when (regAddress + nRegisters > 0xFFFF).
	 *
	 * @param startAddress[in]	- Starting Address (0x0000 to 0xFFFF)
	 * @param nRegisters[in]    - Quantity of Registers (1 to 125 (0x7D))
	 * @param buf[out]          - Buffer to store the read result as 16-bit HEX values.
	 * @return true				- If read success
	 * @return false			- If some error occurred
	 */
	bool ReadHoldingRegisters(
		unsigned short startAddress,
		unsigned char nRegisters,
		unsigned short* buf);

	/**
	 * @brief Write Single Register (0x06 Function code).
	 * Write a single holding register in a remote device
	 *
	 * @param regAddress[in]	- Register Address (0x0000 to 0xFFFF)
	 * @param regValue[in]      - Register Value (0x0000 to 0xFFFF)
	 * @return true				- If write success
	 * @return false			- If some error occurred
	 */
	bool WriteSingleRegister(
		unsigned short regAddress,
		unsigned short regValue);

	/**
	 * @brief Set the Number Of Transmit Attempts when frame transfer fails.
	 * Default value (1) means that after first transmit and its fail an error will be returned.
	 * Value (2) for example means that after first transmit fail
	 * the program will try transmitting frame one more time.
	 *
	 * @param attempts[in] - number of repeated transmit attempts
	 */
	void SetNumberOfTransmitAttempts(unsigned char attempts = 1);
};

#endif // MODBUSRTUCLIENT_H
