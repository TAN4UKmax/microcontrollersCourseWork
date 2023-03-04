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
	HANDLE          hCOM;           // COM port handler structure
	DWORD           error;          // variable that receives a mask indicating the type of port error

	char            name[9];	    // COM port name ("COM3" by default)
	unsigned long   baud;           // baudrate of communication (9600 by default)
	unsigned char   dataBit;        // number of bits of data (8 by default)
	char            parity;         // parity parameter ('N' by default)
	unsigned char   stopBit;        // number of stop bits (1 by default)

	unsigned long	tInterval;      // interval timeout for read operation
	unsigned long	tMultiplier;    // multiplier timeout for read operation
	unsigned long	tConstant;      // constant timeout for read operation

	/**
	 * @brief Write parameters into DCB internal structure
	 *
	 * @return true     - if success
	 * @return false    - if fails
	 */
	bool WriteDCB();

    /**
     * @brief Write read timeouts into internal structure
     * 
     * @return true     - if success
     * @return false    - if fails
     */
	bool TimeoutsSetup();
public:
	/**
	 * @brief Construct a new COMPort object
	 *
	 * @param name      - COM port name ("COM3" by default)
	 * @param baud      - baudrate of communication (9600 by default)
	 * @param dataBit   - number of bits of data (8 by default)
	 * @param parity    - parity parameter ('N' or NOPARITY by default)
	 * @param stopBit   - number of stop bits (1 or ONESTOPBIT by default)
	 */
	COMPort(
		const char* name = "COM3",
		unsigned long baud = 9600,
		unsigned char dataBit = 8,
		char parity = 'N',
		unsigned char stopBit = 1);

	/**
	 * @brief Copy constructor for COMPort.
     * Is necessary for correct handle transfer and share resourse.
     * This constructor implements move instead of copy to prevent multiple access to port
	 * 
	 * @param other     - COMPort object that will be copied into current instance
	 */
	COMPort(COMPort& other);

	/**
	 * @brief Copy operaror for COMPort.
     * Is necessary for correct handle transfer and share resourse.
     * This operator implements move instead of copy to prevent multiple access to port
	 * 
	 * @param other     - COMPort object that will be copied into current instance
	 * @return COMPort& - reference to current instance of class
	 */
	COMPort& operator =(COMPort& other);

	/**
	 * @brief Move constructor for COMPort.
     * Is necessary for correct handle transfer and share resourse.
	 * 
	 * @param other     - COMPort object that will be moved into current instance
	 */
	COMPort(COMPort&& other) noexcept;

	/**
	 * @brief Move operator for COMPort.
     * Is necessary for correct handle transfer and share resourse.
	 * 
	 * @param other     - COMPort object that will be moved into current instance
	 * @return COMPort& - reference to current instance of class
	 */
	COMPort& operator =(COMPort&& other) noexcept;

	/**
	 * @brief Destroy the COMPort object (closes the file handle)
	 *
	 */
	~COMPort();

	/**
	 * @brief Open COM port communication
	 *
	 * @return true		- if open success
	 * @return false	- if open fails
	 */
	bool Open();

	/**
	 * @brief Check current state of COM port
	 *
	 * @return true     - Port is opened
	 * @return false    - Port is closed
	 */
	bool isOpen();

	/**
	 * @brief Close COM port communication
	 *
	 * @return true		- if close success
	 * @return false	- if close fails
	 */
	bool Close();

	/**
	 * @brief Set the Config of port, allows to change COM port configuration after creating an object
	 *
	 * @param baud      - baudrate of communication (9600 by default)
	 * @param dataBit   - number of bits of data (8 by default)
	 * @param parity    - parity parameter ('N' or NOPARITY by default)
	 * @param stopBit   - number of stop bits (1 or ONESTOPBIT by default)
	 * @return true     - if port parameters changed
	 * @return false    - if change fails
	 */
	bool SetConfig(
		unsigned long baud = 9600,
		unsigned char dataBit = 8,
		char parity = 'N',
		unsigned char stopBit = 1);

	/**
	 * @brief Set the Read Timeouts for COM port buffer.
	 * (0, 0, 1)
	 * - return immediately with the bytes that have already been received,
	 * even if no bytes have been received
	 * (0, 0, 0)
	 * - wait untill buffer is filled up to specified size
	 * (1, 0, 0)
	 * - If there are any bytes in the input buffer,
	 * ReadFile returns immediately with the bytes in the buffer.
	 * If there are no bytes in the input buffer,
	 * ReadFile waits until a byte arrives and then returns immediately.
	 * If no bytes arrive within the time specified by ReadTotalTimeoutConstant,
	 * ReadFile times out.
	 * @param interval      - The maximum time allowed to elapse before the arrival of the next byte on the communications line, in milliseconds
	 * @param multiplier    - The multiplier used to calculate the total time-out period for read operations, in milliseconds.
	 * @param constant      - A constant used to calculate the total time-out period for read operations, in milliseconds.
	 * @return true         - if timeouts updated
	 * @return false        - if timeouts update fails
	 */
	bool SetReadTimeouts(
		unsigned long interval = 0,
		unsigned long multiplier = 0,
		unsigned long constant = 1);

	/**
	 * @brief Clears input buffer of port and terminates all current read operations
	 *
	 * @return true		- if clear success
	 * @return false	- if clear fails
	 */
	bool ClearReadBuffer();

	/**
	 * @brief Writes an array of binary data into COM port
	 *
	 * @param buf       - pointer to data buffer
	 * @param length    - buffer length
	 * @return long     - number of written bytes or -1 if error
	 */
	long Write(unsigned char* buf, unsigned char length);

	/**
	 * @brief Reads an array of binary data from COM port buffer
	 *
	 * @param buf       - pointer to data buffer
	 * @param length    - buffer length
	 * @return long     - number of read bytes or -1 if error
	 */
	long Read(unsigned char* buf, unsigned char length);
};

#endif // COMPORT_H
