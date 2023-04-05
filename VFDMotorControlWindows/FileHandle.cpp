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

#include "main.h"

 /**
  * @brief Get the Next Time and Frequency pair from file with diagram coordinates
  *
  * @param diagramFile[in]	- pointer to FILE handle with diagram
  * @param curTime[in]		- current time
  * @param curFreq[in]		- current frequency
  * @param nextTime[out]	- pointer to variable when the next time will be stored
  * @param nextFreq[out]	- pointer to variable when the next frequency will be stored
  * @return true			- if new coordinates have read
  * @return false			- if no new coordinates (end of file reached)
  */
bool GetNextTimeAndFrequency(FILE* diagramFile,
	double curTime, double curFreq,
	double* nextTime, double* nextFreq);

/**
 * @brief Prints measured parameters into screen and into file
 *
 * @param time[in]  - time when parameters measured
 */
void OutParameters(double time);

bool RunDiagramFromFile(VFD& motor)
{
	// 1) Update max frequency parameter from VFD (and check connection by doing this)
	if (!motor.ReadMaxFrequency())
	{
		assert(("main::RunDiagramFromFile(): Read max frequency error", 0));
		return false;
	}
	// 2) Open file with required diagram and check open error
	FILE* diagram_FILE;
	int openStatus = fopen_s(&diagram_FILE, diagramFileName, "r");
#ifndef NDEBUG
	printf("main::RunDiagramFromFile() File %p opened\n", diagram_FILE);
#endif // NDEBUG
	if ((diagram_FILE == nullptr) || openStatus)
	{
		assert(("main::RunDiagramFromFile(): Read coords file error", 0));
		return false;
	}
	// 3) Print output parameters table header to screen
	// 3.1) Read and print initial parameters
	PrintParametersHeader(true);
	// 4) Update max frequency and read parameters to determine current frequency
	// (This will allow to start motor not only from zero frequency)
	if (!GetMotorParameters(motor)) return false;
	OutParameters(0); // Out parameters at 0 time
	// 5) Set watchdog 
	if (!motor.SetWatchdog(1))
	{
		assert(("main::RunDiagramFromFile(): Set watchdog timer error", 0));
		return false;
	}
	// 6) Create initial variables
	// parameters from file
	double	fileFreqNext = 0;	// next frequency from file
	double	fileTimeNext = 0;	// next time from file
	double	fileFreqCur = 0;	// current frequency from file
	double	fileTimeCur = 0;	// current time from file
	// timers
	double	timeStart = clock() / 1000.0;	// time of start following diagram in seconds
	double	timeLastOperation = 0;			// last time of parameters measure
	const double readInterval = 0.1;		// time interval between read parameters
#ifndef NDEBUG
	printf("main::RunDiagramFromFile() Diagram started in %g\n", timeStart);
#endif // NDEBUG
	// Start following diagram ////////////////////////////////////////////////
	while (true)
	{
		double timeNow = (clock() / 1000.0) - timeStart; // current moment time (seconds)
		// Set new motor parameters
		if (timeNow >= fileTimeNext) // if current time is greater than assigned time from file
		{
#ifndef NDEBUG
			printf("main::RunDiagramFromFile() Write new parameter in %g\n", timeNow);
#endif // NDEBUG
			fileTimeCur = timeNow;		// timeNow here to calculate parameters more precise
			fileFreqCur = motorParams.OutFrequency; // update frequency
			// Read new time and frequency parameters from file
			if (!GetNextTimeAndFrequency(diagram_FILE, fileTimeCur, fileFreqCur, &fileTimeNext, &fileFreqNext))
			{
				// Reached end of file
				fclose(diagram_FILE);
				// 1) Print last coordinate parameters
				if (!GetMotorParameters(motor)) return false;
				timeNow = (clock() / 1000.0) - timeStart; // get new fresh time
				OutParameters(timeNow);

				// 2) Stop motor at the minimal deceleration (0 deceleration time is dangerous)
				if (!motor.SetDecelerationTime(0) || !motor.Stop())
				{
					assert(("main::RunDiagramFromFile(): Stop motor error", 0));
					return false;
				}
				return true;
			}
#ifndef NDEBUG
			printf("main::RunDiagramFromFile() Change frequency to %g in %g s\n", fileFreqNext, fileTimeNext);
#endif // NDEBUG
			// Set new parameters
			if (!motor.ChangeFrequency(fileFreqCur, fileFreqNext, fileTimeNext - fileTimeCur))
			{
				assert(("main::RunDiagramFromFile(): Change frequency error", 0));
				return false;
			}
		}
		// Read current motor parameters (every 100 ms)
		// read only when no write operations in the next 100 ms
		if (((timeNow - timeLastOperation) > readInterval) && ((timeNow + readInterval) < fileTimeNext))
		{
			timeLastOperation = timeNow;
			if (!GetMotorParameters(motor)) return false;
			timeNow = (clock() / 1000.0) - timeStart; // get new fresh time
			OutParameters(timeNow);
		}
		// Small delay between iterations for stability
		Sleep(1);
	}
	return true;
}

bool GetNextTimeAndFrequency(FILE* diagramFile,
	double curTime, double curFreq,
	double* nextTime, double* nextFreq)
{
	// temporary variables (necessary for part of diagram when direction changes)
	static bool		dirChange = false;	// true if direction is going to change
	static double	tempTime = 0;		// stores temporary time
	static double	tempFreq = 0;		// stores temporary frequency
#ifndef NDEBUG
	printf("main::GetNextTimeAndFrequency() Read new data from file %p\n", diagramFile);
#endif // NDEBUG
	if (!dirChange) // no direction change
	{
		while (true)
		{
			int read_result = fscanf_s(diagramFile, "%lf%lf",
				nextTime, nextFreq);
			// success read check
#ifndef NDEBUG
			printf("main::GetNextTimeAndFrequency() New point %g, %g\n", *nextTime, *nextFreq);
#endif // NDEBUG
			// 0.25 is necessary to prevent updating frequency to fast
			if ((read_result == 2) && (*nextTime >= (curTime + 0.25)))
			{
				// check direction change
				if ((curFreq * (*nextFreq)) < 0)
				{
					dirChange = true;
					tempTime = *nextTime;
					tempFreq = *nextFreq;
					*nextFreq = 0;
					// Calculate time when frequency should be 0 with interpolation
					*nextTime = curTime + (*nextFreq - curFreq) *
						((tempTime - curTime) / (tempFreq - curFreq));
				}
				return true;
			}
			// clean current line
			while ((fgetc(diagramFile) != '\n') && (!feof(diagramFile)));
			if (feof(diagramFile)) return false;// reached end of file
		}
	}
	else // direction is going to change
	{
		*nextTime = tempTime;
		*nextFreq = tempFreq;
		dirChange = false;
	}
	return true;
}

void OutParameters(double time)
{
	PrintParameters(time); // Print parameters to sceen
	// Print parameters to file
	FILE* param_FILE;
	int openStatus = fopen_s(&param_FILE, "paramTable.txt", "w");
	// Print params to file only if it is correctly opened
	if ((param_FILE != nullptr) || !openStatus)
	{
		PrintParametersHeader(true, param_FILE);
		PrintParameters(time, param_FILE);
		fclose(param_FILE);
	}
}
