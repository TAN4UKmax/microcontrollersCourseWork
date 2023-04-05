/**
 * @file VFD.h
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief Class for create and decode commands for VFD control
 * This class is written according to VFD-B_manual_rus.pdf
 * @version 0.1
 * @date 2023-02-23
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */

#ifndef VFD_H
#define VFD_H

#include ".\INC\MBRTUCL.H" // 7188EX uPAC

// Stores VFD status from 0x2101 register
typedef struct VFD_status {
	struct {
		char RUN;	// RUN LED (1 - on, 0 - off)
		char STOP;	// STOP LED (1 - on, 0 - off)
		char JOG;	// JOG LED (1 - on, 0 - off)
		char FWD;	// FWD LED (1 - on, 0 - off)
		char REW;	// REW LED (1 - on, 0 - off)
	} LED;
	char F;	// F letter on display (1 - on, 0 - off)
	char H;	// H letter on display (1 - on, 0 - off)
	char u;	// u letter on display (1 - on, 0 - off)
	char controlFrequencyBySerialInterface; // 1 if frequency controlled by serial interface
	char controlFrequencyByAnalogSignal;	// 1 if frequency controlled by analog signal
	char controlVFDBySerialInterface;		// 1 if controlled by serial interface
	char parametersBlocked;					// 1 if parameters blocked
	char VFDCurrentState;					// (0 - VFD stopped, 1 - VFD works)
	char JOGcommand;						// 1 if JOG command
} VFD_status_t;

// Stores VFD parameters from registers 0x2103-0x210F
typedef struct VFD_param {
	float FrequencyCommand;	// 0x2102
	float OutFrequency;		// 0x2103
	float OutCurrent;		// 0x2104
	float DCVoltage;		// 0x2105
	float OutVoltage;		// 0x2106
	float PowerFactor;		// 0x210A
	float OutTorque;		// 0x210B
	float MotorSpeed;		// 0x210C
} VFD_param_t;

class VFD
{
private:
	ModbusRTUClient	MB;				// Instance of ModbusRTUClient class
    float			maxFrequency;   // Maximum output motor frequency (01-00 value, default 50Hz)
public:
	/**
	 * @brief Construct a new VFD object. Set device communication parameters
	 *
	 * @param mb[in] - ModbusRTUClient class instance with its parameters
	 */
	VFD(ModbusRTUClient mb);

	/**
	 * @brief Destroy the VFD object
	 *
	 */
	~VFD();

    /**
     * @brief Run motor with specified direction and specified acceleration.
	 * The direction attribute accepts several parameters:
	 * 0 - no change, 1 - forward, 2 - reverse, 3 - change
     * 
     * @param direction[in]	- Set rotation direction (default: 0 - no change)
     * @return 1            - Run success
     * @return 0            - Run fail
     */
	char Run(unsigned short direction = 0);

    /**
     * @brief Stop motor with specified deceleration
     * 
     * @return 1    - Stop success
     * @return 0    - Stop fail
     */
	char Stop(void);

    /**
     * @brief Run motor to specified frequency with specified
	 * acceleration of deceleration time. This method uses maxFrequency (01-00)
	 * parameter which is 50Hz by default. If you set another value in your VFD,
	 * read that value by calling ReadMaxFrequency() one time
	 * before using this method
     * 
     * @param curFreq[in]       - Current motor frequency
     * @param newFreq[in]       - New motor frequency
     * @param changeTime[in]	- Acceleration or deceleraiton time for which the
	 * specified frequency will be reached
     * @return 1				- Set new frequency and time success
     * @return 0                - Set new frequency and time fail
     */
	char ChangeFrequency(
		float curFreq = 0,
		float newFreq = 50,
		float changeTime = 1);

    /**
     * @brief Read motor current status and parameters from registers 0x2101-0x210C
	 * and store them into structures
     * 
     * @param status[out]   - pointer to structure where status will be stored
     * @param param[out]    - pointer to structure where parameters will be stored
     * @return 1            - Read success
     * @return 0            - Read fail
     */
	char ReadParameterRegisters(VFD_status_t* status, VFD_param_t* param);

	/**
	 * @brief Get the power provided to motor (0x210F VFD parameter)
	 *
	 * @param power[out]	- Pointer to variable where power will be stored
	 * @return 1			- Read power success
	 * @return 0            - Read power fail
	 */
	char GetOutPower(float* power);

    /**
     * @brief Get the temperature of VFD heatsink (0x2206 VFD parameter)
     * 
     * @param temp[out] - Pointer to variable where temperature will be stored
     * @return 1        - Read temperature success
     * @return 0        - Read temperature fail
     */
	char GetVFDTemperature(float* temp);

    /**
     * @brief Get the maximum output frequency of motor (0x0100 VFD parameter)
     * Maximum frequency value vill be stored in internal field of class for
     * using it in ChangeFrequency() method. Also you can get this value by providing
     * pointer to variable as an argument
     * 
     * @param maxFreq[out]  - [optional] Pointer to variable where max frequency will be stored
     * @return 1            - Read maximum output frequency success
     * @return 0            - Read maximum output frequency fail
     */
	char ReadMaxFrequency(float* maxFreq = 0);

    /**
     * @brief Set the Frequency of motor
     * 
     * @param freq[in]	- Frequency command to set
     * @return 1        - Set frequency success
     * @return 0        - Set frequency fail
     */
	char SetFrequency(float freq);

    /**
     * @brief Set the Acceleration Time of motor
     * 
     * @param time[in]	- Time to set
     * @return 1        - Set time success
     * @return 0        - Set time fail
     */
	char SetAccelerationTime(float time);

    /**
     * @brief Set the Deceleration Time of motor
     * 
     * @param time[in]	- Time to set
     * @return 1        - Set time success
     * @return 0        - Set time fail
     */
	char SetDecelerationTime(float time);

    /**
     * @brief Set the Watchdog timer for Modbus communication.
     * After set this timer and start motor the client should send new
     * Modbus request in peirods less than specified time. Otherwise the
     * motor will stop.
     * Set 0 to switch off watchdog timer
     * 
     * @param time[in]	- Watchdog time to set
     * @return 1        - Set watchdog success
     * @return 0        - Set time fail
     */
	char SetWatchdog(float time = 0);

    /**
     * @brief Allows to Get and Set VFD parameters directly
     * 
     * @param addr      - parameter address
     * @param val       - pointer to value where parameter will be stored or value to write into VFD
     * @return 1        - if success
     * @return 0        - if fail
     */
	char GetParam(unsigned short addr, unsigned short* val);
	char SetParam(unsigned short addr, unsigned short val);
};

#endif // VFD_H
