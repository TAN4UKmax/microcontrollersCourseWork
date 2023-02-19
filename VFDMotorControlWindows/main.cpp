/**
 * @file main.cpp
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief VFDMotorControl main file
 * @version 0.1
 * @date 2023-02-13
 * 
 * @copyright Copyright (c) 2023 TAN4UK
 * 
 */

#include <cstdio>
#include <clocale>
#include <ctime>

 //#define NDEBUG
#include <cassert>

//#include "VFD.h"
//#include "ModbusRTUclient.h"
#include "COMPort.h"
//#include "COMPortFake.h"

using namespace std;

char writebuf[10] = "Hello";
char readbuf[10] = { 0 };
clock_t start_time = 0;
clock_t stop_time = 0;
clock_t operation_time = 0;

int main()
{
	// Set locale
	setlocale(LC_ALL, "rus");

	// Setup and open port
	COMPort COM("COM3");
	//COMPortFake COM("COM3", 9600, 'N', 8, 1);
	assert(COM.Open());
	COM.SetReadTimeouts(1, 0, 0);
	/* Transfer data */
	 //Write
	printf("\nData to write: %s\n", writebuf);
	start_time = clock();
	COM.Write((unsigned char*)writebuf, 5);
	stop_time = clock();
	operation_time = stop_time - start_time;
	printf("Write time: %d\n", operation_time);
	//small delay between write and read
	//Sleep(10);
	// Read
	start_time = clock();
	COM.Read((unsigned char*)readbuf, 8);
	stop_time = clock();
	printf("Read data: %s\n", readbuf);
	operation_time = stop_time - start_time;
	printf("Read time: %d\n", operation_time);

	/* Clear buffers test */

	COM.ClearBuffers();
	start_time = clock();
	COM.Read((unsigned char*)readbuf, 10);
	stop_time = clock();
	printf("Read data: %s\n", readbuf);
	operation_time = stop_time - start_time;
	printf("Read time: %d\n", operation_time);
	/*ERROR_IO_PENDING*/
	// port closes by destructor
	return 0;
}
