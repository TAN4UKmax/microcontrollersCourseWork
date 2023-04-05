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

#include <stdio.h>	// for 'sscanf'
#include <stdlib.h>	// for 'atof'
#include <string.h>	// for string operations like 'strlen' and 'strcpy'

//#define NDEBUG
#include <assert.h>	// for debug printing

#include ".\INC\VFD.H" // 7188EX uPAC

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
 * @return 1     - if motor have run according to the file
 * @return 0    - if some error occured
 */
char RunDiagramFromFile(VFD& motor);

/**
 * @brief Get the Motor Parameters requested by user and print them
 *
 * @param motor[in] - reference to VFD class instance
 * @return 1     - if all motor parameters have read successfully
 * @return 0    - if some error occured
 */
char GetMotorParameters(VFD& motor);

/**
 * @brief Print table header for parameters, specified in input arguments
 *
 * @param Time[in]			- Should be 1 if you want to print time later
 * @param printStream[in]	- File handle, if not specified, parameters will be printed into stdout stream
 */
//void PrintParametersHeader(char Time = 0, FILE_DATA far* printStream = 0);
void PrintParametersHeader(char Time = 0);

/**
 * @brief Print parameters, specified in input arguments
 *
 * @param Time[in]			- if you want to print time specify it here
 * @param printStream[in]	- File handle, if not specified, parameters will be printed into stdout stream
 */
//void PrintParameters(float Time = -1, FILE_DATA far* printStream = 0);
void PrintParameters(float Time = -1);

#endif // MAIN_H
