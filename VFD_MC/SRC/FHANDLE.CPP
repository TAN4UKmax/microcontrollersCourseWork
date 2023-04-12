/**
 * @file FileHandle.cpp
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief Set of functions which allow to read a file with diagram and run
 * the motor according to that diagram
 * @version 0.1
 * @date 2023-03-04
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */

#include "MAIN.H"

 /**
  * @brief Functions is used to reset this program
  *
  */
void (far* ResetProgram)(void) = (void(*)())0xFFFF0000L;    //Program start address.

/**
 * @brief Get the Next Time and Frequency pair from file with diagram coordinates
 *
 * @param diagramFile[in]	- pointer to FILE handle with diagram
 * @param curTime[in]       - current time
 * @param curFreq[in]       - current frequency
 * @param nextTime[out]     - pointer to variable when the next time will be stored
 * @param nextFreq[out]     - pointer to variable when the next frequency will be stored
 * @return 1				- if new coordinates have read
 * @return 0				- if no new coordinates (end of file reached)
 */
char GetNextTimeAndFrequency(FILE_DATA far* diagramFile,
	float curTime, float curFreq,
	float* nextTime, float* nextFreq);

/**
 * @brief Prints measured parameters into screen and into file
 *
 * @param time[in]  - time when parameters measured
 */
void OutParameters(float time);

char RunDiagramFromFile(VFD& motor)
{
	// 0) Wait for button press
	while (ReadInitPin() == 0);
	Delay(50); // debounce prevent

	// 1) Update max frequency parameter from VFD (and check connection by doing this)
	// On 7188 uPAC this request will be sending until VFD respond
	while (!motor.ReadMaxFrequency());
	// Attention LED blinking
	for (char i = 0; i < 10; i++)
	{
		LedOn();
		Delay(100);
		LedOff();
		Delay(100);
	}
	LedOn(); // LED is on when diagram works

	// 2) Open file with required diagram and check open error
	static FILE_DATA far* diagram_FILE = 0;
	diagram_FILE = GetFileInfoByName(diagramFileName);
#ifndef NDEBUG
	Print("main::RunDiagramFromFile() File %p opened\n", diagram_FILE);
#endif // NDEBUG
	if (diagram_FILE == 0)
	{
		assert(("main::RunDiagramFromFile(): Read coords file error", 0));
		// Reset 7188 controller
		ResetProgram();
	}
	// 3) Print output parameters table header to screen
	// 3.1) Read and print initial parameters
	PrintParametersHeader(1); // Prints with time
	// 4) Update max frequency and read parameters to determine current frequency
	// (This will allow to start motor not only from zero frequency)
	if (!GetMotorParameters(motor))
		// Reset 7188 controller
		ResetProgram();
	OutParameters(0); // Out parameters at 0 time
	// 5) Set watchdog 
	if (!motor.SetWatchdog(1))
	{
		assert(("main::RunDiagramFromFile(): Set watchdog timer error", 0));
		// Reset 7188 controller
		ResetProgram();
	}
	// 6) Create initial variables
	// parameters from file
	float	fileFreqNext = 0;	// next frequency from file
	float	fileTimeNext = 0;	// next time from file
	float	fileFreqCur = 0;	// current frequency from file
	float	fileTimeCur = 0;	// current time from file
	// timers
	float	timeStart = GetTimeTicks() / 1000.0;	// time of start following diagram in seconds
	float	timeLastOperation = 0;			// last time of parameters measure
	const float readInterval = 0.1;		// time interval between read parameters
#ifndef NDEBUG
	Print("main::RunDiagramFromFile() Diagram started in %g\n", timeStart);
#endif // NDEBUG
	// Start following diagram ////////////////////////////////////////////////
	while (1)
	{
		float timeNow = (GetTimeTicks() / 1000.0) - timeStart; // current moment time (seconds)
		// Set new motor parameters
		if (timeNow >= fileTimeNext) // if current time is greater than assigned time from file
		{
#ifndef NDEBUG
			Print("main::RunDiagramFromFile() Write new parameter in %g\n", timeNow);
#endif // NDEBUG
			fileTimeCur = timeNow;		// timeNow here to calculate parameters more precise
			fileFreqCur = motorParams.OutFrequency; // update frequency
			// Read new time and frequency parameters from file
			if (!GetNextTimeAndFrequency(diagram_FILE, fileTimeCur, fileFreqCur, &fileTimeNext, &fileFreqNext))
			{
				// Reached end of file
				LedOff(); // Switch off LED on 7188 uPAC
				// 1) Print last coordinate parameters
				if (!GetMotorParameters(motor))
					// Reset 7188 controller
					ResetProgram();
				timeNow = (GetTimeTicks() / 1000.0) - timeStart; // get new fresh time
				OutParameters(timeNow);

				// 2) Stop motor at the minimal deceleration (0 deceleration time is dangerous)
				if (!motor.SetDecelerationTime(0) || !motor.Stop())
				{
					assert(("main::RunDiagramFromFile(): Stop motor error", 0));
					// Reset 7188 controller
					ResetProgram();
				}

				// 3) Reset 7188 controller
				ResetProgram();
			}
#ifndef NDEBUG
			Print("main::RunDiagramFromFile() Change frequency to %g in %g s\n", fileFreqNext, fileTimeNext);
#endif // NDEBUG
			// Set new parameters
			if (!motor.ChangeFrequency(fileFreqCur, fileFreqNext, fileTimeNext - fileTimeCur))
			{
				assert(("main::RunDiagramFromFile(): Change frequency error", 0));
				// Reset 7188 controller
				ResetProgram();
			}
		}
		// Read current motor parameters (every 100 ms)
		// read only when no write operations in the next 100 ms
		if (((timeNow - timeLastOperation) > readInterval) && ((timeNow + readInterval) < fileTimeNext))
		{
#ifndef NDEBUG
			Print("main::RunDiagramFromFile() Read parameters in %g\n", timeNow);
#endif // NDEBUG
			timeLastOperation = timeNow;
			if (!GetMotorParameters(motor))
				// Reset 7188 controller
				ResetProgram();
			timeNow = (GetTimeTicks() / 1000.0) - timeStart; // get new fresh time
			OutParameters(timeNow);
		}
		// Small delay between iterations for stability
		Delay(1);

		//Stop motor from INIT* button
		if (ReadInitPin())
		{
			// Small delay for debounce prevent
			Delay(10);
			// 0) Wait for button release
			while (ReadInitPin());
			// 1) Switch off the LED
			LedOff();
			// 2) Stop motor at the minimal deceleration (0 deceleration time is dangerous)
			if (!motor.SetDecelerationTime(0) || !motor.Stop())
			{
				assert(("main::RunDiagramFromFile(): Stop motor error", 0));
				// Reset 7188 controller
				ResetProgram();
			}
			// 3) Reset 7188 controller program
			ResetProgram();
		}
	}
}

char GetNextTimeAndFrequency(FILE_DATA far* diagramFile,
	float curTime, float curFreq,
	float* nextTime, float* nextFreq)
{
	// temporary variables (necessary for part of diagram when direction changes)
	static char		dirChange = 0;		// 1 if direction is going to change
	static float	tempTime = 0;		// stores temporary time
	static float	tempFreq = 0;		// stores temporary frequency

	static unsigned long fPos = 0; // Stores current file pointer position

#ifndef NDEBUG
	Print("main::GetNextTimeAndFrequency() Read new data from file %p in pos %u\n", diagramFile, fPos);
#endif // NDEBUG
	if (!dirChange) // no direction change
	{
		while (1)
		{
			if (fPos >= diagramFile->size) return 0;// reached end of file
			int read_result = sscanf(&(diagramFile->addr[fPos]), "%f%f", nextTime, nextFreq);
			// GET NEXT LINE POSITION
			while (fPos < diagramFile->size)
			{
				char c = diagramFile->addr[fPos];
				if (c == '\n' || c == '\r')
				{
					fPos++;
					c = diagramFile->addr[fPos];
					if (c == '\n' || c == '\r')
					{
						fPos++;
						break;
					}
					break;
				}
				fPos++;
			}
			// success read check
#ifndef NDEBUG
			Print("main::GetNextTimeAndFrequency() New point %g, %g\n", *nextTime, *nextFreq);
#endif // NDEBUG
			// 0.25 is necessary to prevent updating frequency to fast
			if ((read_result == 2) && (*nextTime >= (curTime + 0.25)))
			{
				// check direction change
				if ((curFreq * (*nextFreq)) < 0)
				{
					dirChange = 1;
					tempTime = *nextTime;
					tempFreq = *nextFreq;
					*nextFreq = 0;
					// Calculate time when frequency should be 0 with interpolation
					*nextTime = curTime + (*nextFreq - curFreq) *
						((tempTime - curTime) / (tempFreq - curFreq));
				}
				return 1;
			}
		}
	}
	else // direction is going to change
	{
		*nextTime = tempTime;
		*nextFreq = tempFreq;
		dirChange = 0;
	}
	return 1;
}

void OutParameters(float time)
{
	PrintParameters(time); // Print parameters to sceen
}
