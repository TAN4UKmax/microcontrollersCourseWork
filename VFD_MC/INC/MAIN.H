/**
 * @file main.h
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief This file declares common resourses (variables and function prototypes)
 * for main.cpp and FileHandle.cpp 
 * @version 0.1
 * @date 2023-03-04
 * 
 * @copyright Copyright (c) 2023 TAN4UK
 * 
 */

#ifndef MAIN_H
#define MAIN_H

// Global includes ////////////////////////////////////////////////////////////

#include <stdio.h>	// for 'printf'
#include <stdlib.h>	// for 'atof'
#include <string.h>	// for string operations like 'strlen' and 'strcpy'
#include <time.h>	// for time intervals in milliseconds measure and date check

//#define NDEBUG
#include <assert.h>	// for debug printing

#ifdef _I7188E_
#include ".\INC\VFD.H"
#else
#include "..\INC\VFD.H"
#endif

//using namespace std;

// Global variables ///////////////////////////////////////////////////////////

extern char*		diagramFileName;	// file name with diagram
extern VFD_status_t	motorStatus;		// Stores motor status
extern VFD_param_t	motorParams;		// Stores motor parameters

// Global function prototypes /////////////////////////////////////////////////
/**
 * @brief Run motor according to the file with input coordinats.
 * Also measure motor parameters specified by -- get CLI argument.
 *
 * @param motor[in] - reference to VFD class instance
 * @return true     - if motor have run according to the file
 * @return false    - if some error occured
 */
bool RunDiagramFromFile(VFD& motor);

/**
 * @brief Get the Motor Parameters requested by user and print them
 *
 * @param motor[in] - reference to VFD class instance
 * @return true     - if all motor parameters have read successfully
 * @return false    - if some error occured
 */
bool GetMotorParameters(VFD& motor);

/**
 * @brief Print table header for parameters, specified in input arguments
 *
 * @param Time[in]			- Should be true if you want to print time later
 * @param printStream[in]	- File handle, if not specified, parameters will be printed into stdout stream
 */
void PrintParametersHeader(bool Time = false, FILE* printStream = stdout);

/**
 * @brief Print parameters, specified in input arguments
 *
 * @param Time[in]			- if you want to print time specify it here
 * @param printStream[in]	- File handle, if not specified, parameters will be printed into stdout stream
 */
void PrintParameters(double Time = -1, FILE* printStream = stdout);

#endif // MAIN_H
