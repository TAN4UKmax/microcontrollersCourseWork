/**
 * @file COMPort.h
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief Class for communication with COM port on 7188 uPAC
 * @version 0.1
 * @date 2023-02-13
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */

#ifndef COMPORT_H
#define COMPORT_H

#include ".\Lib\7188E.H"  // 7188EX uPAC

class COMPort
{
private:
    char			port;           // 0-8 (COM0-COM8) port number
    char			opened;         // current status of COM port

	char            name[12];	    // COM port name ("COM2" by default)
	unsigned long   baud;			// baudrate of communication (9600 by default)
	char			dataBit;        // number of bits of data (8 by default)
	char			parity;         // parity parameter ('N' by default)
	char			stopBit;        // number of stop bits (1 by default)

	unsigned long	tInterval;      // interval timeout for read operation
	unsigned long	tMultiplier;	// multiplier timeout for read operation
	unsigned long	tConstant;      // constant timeout for read operation

    /**
     * @brief Writes COM port connection parameters into internal structures
     * 
	 * @return 1    - if success
	 * @return 0    - if fails
     */
    char PortSetup(void);
public:
	/**
	 * @brief Construct a new COMPort object
	 *
	 * @param name[in]      - COM port name ("COM2" by default)
	 * @param baud[in]      - baudrate of communication (9600 by default)
	 * @param dataBit[in]   - number of bits of data (8 by default)
	 * @param parity[in]    - parity parameter ('N' or NOPARITY by default)
	 * @param stopBit[in]   - number of stop bits (1 or ONESTOPBIT by default)
	 */
	COMPort(
		const char* name = "COM2",
		unsigned long baud = 9600,
		char dataBit = 8,
		char parity = 'N',
		char stopBit = 1);

	/**
	 * @brief Copy constructor for COMPort.
     * Is necessary for correct handle transfer and share resourse.
     * This constructor implements move instead of copy to prevent multiple access to port
	 * 
	 * @param other[in]     - COMPort object that will be copied into current instance
	 */
	COMPort(COMPort& other);

	/**
	 * @brief Copy operaror for COMPort.
     * Is necessary for correct handle transfer and share resourse.
     * This operator implements move instead of copy to prevent multiple access to port
	 * 
	 * @param other[in]	- COMPort object that will be copied into current instance
	 * @return COMPort& - reference to current instance of class
	 */
	COMPort& operator =(COMPort& other);

	/**
	 * @brief Destroy the COMPort object (closes the file handle)
	 *
	 */
	~COMPort();

	/**
	 * @brief Open COM port communication
	 *
	 * @return 1    - if success
	 * @return 0    - if fails
	 */
	char Open(void);

	/**
	 * @brief Check current state of COM port
	 *
	 * @return 1    - Port is opened
	 * @return 0    - Port is closed
	 */
	char isOpen(void);

	/**
	 * @brief Close COM port communication
	 *
	 * @return 1    - if close success
	 * @return 0    - if close fails
	 */
	char Close(void);

	/**
	 * @brief Set the Config of port, allows to change COM port configuration after creating an object
	 *
	 * @param baud[in]      - baudrate of communication (9600 by default)
	 * @param dataBit[in]   - number of bits of data (8 by default)
	 * @param parity[in]    - parity parameter ('N' or NOPARITY by default)
	 * @param stopBit[in]   - number of stop bits (1 or ONESTOPBIT by default)
	 * @return 1            - if port parameters changed
	 * @return 0            - if change fails
	 */
	char SetConfig(
		unsigned long baud = 9600,
		char dataBit = 8,
		char parity = 'N',
		char stopBit = 1);

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
	 * 
	 * @param interval[in]      - The maximum time allowed to elapse before the arrival of the next byte on the communications line, in milliseconds
	 * @param multiplier[in]    - The multiplier used to calculate the total time-out period for read operations, in milliseconds.
	 * @param constant[in]      - A constant used to calculate the total time-out period for read operations, in milliseconds.
	 * @return 1                - if timeouts updated
	 * @return 0                - if timeouts update fails
	 */
	char SetReadTimeouts(
		unsigned long interval = 0,
		unsigned long multiplier = 0,
		unsigned long constant = 1);

	/**
	 * @brief Clears input buffer of port and terminates all current read operations
	 *
	 * @return 1    - if clear success
	 * @return 0    - if clear fails
	 */
	char ClearReadBuffer(void);

	/**
	 * @brief Writes an array of binary data into COM port
	 *
	 * @param buf[in]       - pointer to data buffer
	 * @param length[in]    - buffer length
	 * @return long			- number of written bytes or -1 if error
	 */
	long Write(unsigned char* buf, unsigned char length);

	/**
	 * @brief Reads an array of binary data from COM port buffer
	 *
	 * @param buf[out]      - pointer to data buffer
	 * @param length[in]    - buffer length
	 * @return long			- number of read bytes or -1 if error (0 when timeout)
	 */
	long Read(unsigned char* buf, unsigned char length);
};

#endif // COMPORT_H
