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

#ifdef _I7188E_
#include ".\INC\MBRTUCL.H"
#else
#include "..\INC\MBRTUCL.H"
#endif

// Stores VFD status from 0x2101 register
typedef struct VFD_status {
	struct {
		bool RUN;	// RUN LED (1 - on, 0 - off)
		bool STOP;	// STOP LED (1 - on, 0 - off)
		bool JOG;	// JOG LED (1 - on, 0 - off)
		bool FWD;	// FWD LED (1 - on, 0 - off)
		bool REW;	// REW LED (1 - on, 0 - off)
	} LED;
	bool F;	// F letter on display (1 - on, 0 - off)
	bool H;	// H letter on display (1 - on, 0 - off)
	bool u;	// u letter on display (1 - on, 0 - off)
	bool controlFrequencyBySerialInterface; // 1 if frequency controlled by serial interface
	bool controlFrequencyByAnalogSignal;	// 1 if frequency controlled by analog signal
	bool controlVFDBySerialInterface;		// 1 if controlled by serial interface
	bool parametersBlocked;					// 1 if parameters blocked
	bool VFDCurrentState;					// (0 - VFD stopped, 1 - VFD works)
	bool JOGcommand;						// 1 if JOG command
} VFD_status_t;

// Stores VFD parameters from registers 0x2103-0x210F
typedef struct VFD_param {
	double FrequencyCommand;// 0x2102
	double OutFrequency;	// 0x2103
	double OutCurrent;		// 0x2104
	double DCVoltage;		// 0x2105
	double OutVoltage;		// 0x2106
	double PowerFactor;		// 0x210A
	double OutTorque;		// 0x210B
	double MotorSpeed;		// 0x210C
} VFD_param_t;

class VFD
{
private:
	ModbusRTUClient MB;             // Instance of ModbusRTUClient class
    double          maxFrequency;   // Maximum output motor frequency (01-00 value, default 50Hz)
public:
	/**
	 * @brief Construct a new VFD object. Set device communication parameters
	 *
	 * @param mb[in] - ModbusRTUClient class instance with its parameters
	 */
	VFD(ModbusRTUClient mb = { 1, {"COM3", 9600, 8, 'E', 1} });

	///**
	// * @brief Copy constructor for VFD.
	// * Is necessary for correct handle transfers and share resourse.
	// *
	// * @param other[in] - VFD object that will be copied into current instance
	// */
	//VFD(VFD& other);

	///**
	// * @brief Copy operaror for VFD.
	// * Is necessary for correct handle transfers and share resourse.
	// *
	// * @param other[in] - VFD object that will be copied into current instance
	// * @return VFD& - reference to current instance of class
	// */
	//VFD& operator =(VFD& other);

	///**
	// * @brief Move constructor for VFD.
	// * Is necessary for correct handle transfers and share resourse.
	// *
	// * @param other[in] - VFD object that will be moved into current instance
	// */
	//VFD(VFD&& other) noexcept;

	///**
	// * @brief Move operator for VFD.
	// * Is necessary for correct handle transfers and share resourse.
	// *
	// * @param other[in] - VFD object that will be moved into current instance
	// * @return VFD& - reference to current instance of class
	// */
	//VFD& operator =(VFD&& other) noexcept;

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
     * @return true         - Run success
     * @return false        - Run fail
     */
	bool Run(unsigned short direction = 0);

    /**
     * @brief Stop motor with specified deceleration
     * 
     * @return true     - Stop success
     * @return false    - Stop fail
     */
	bool Stop();

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
     * @return true				- Set new frequency and time success
     * @return false			- Set new frequency and time fail
     */
	bool ChangeFrequency(
		double curFreq = 0,
		double newFreq = 50,
		double changeTime = 1);

    /**
     * @brief Read motor current status and parameters from registers 0x2101-0x210C
	 * and store them into structures
     * 
     * @param status[out]   - pointer to structure where status will be stored
     * @param param[out]    - pointer to structure where parameters will be stored
     * @return true         - Read success
     * @return false        - Read fail
     */
	bool ReadParameterRegisters(VFD_status_t* status, VFD_param_t* param);

	/**
	 * @brief Get the power provided to motor (0x210F VFD parameter)
	 *
	 * @param power[out]	- Pointer to variable where power will be stored
	 * @return true			- Read power success
	 * @return false		- Read power fail
	 */
	bool GetOutPower(double* power);

    /**
     * @brief Get the temperature of VFD heatsink (0x2206 VFD parameter)
     * 
     * @param temp[out] - Pointer to variable where temperature will be stored
     * @return true     - Read temperature success
     * @return false    - Read temperature fail
     */
	bool GetVFDTemperature(double* temp);

    /**
     * @brief Get the maximum output frequency of motor (0x0100 VFD parameter)
     * Maximum frequency value vill be stored in internal field of class for
     * using it in ChangeFrequency() method. Also you can get this value by providing
     * pointer to variable as an argument
     * 
     * @param maxFreq[out]  - [optional] Pointer to variable where max frequency will be stored
     * @return true         - Read maximum output frequency success
     * @return false        - Read maximum output frequency fail
     */
	bool ReadMaxFrequency(double* maxFreq = nullptr);

    /**
     * @brief Set the Frequency of motor
     * 
     * @param freq[in]	- Frequency command to set
     * @return true     - Set frequency success
     * @return false    - Set frequency fail
     */
	bool SetFrequency(double freq);

    /**
     * @brief Set the Acceleration Time of motor
     * 
     * @param time[in]	- Time to set
     * @return true     - Set time success
     * @return false    - Set time fail
     */
	bool SetAccelerationTime(double time);

    /**
     * @brief Set the Deceleration Time of motor
     * 
     * @param time[in]	- Time to set
     * @return true     - Set time success
     * @return false    - Set time fail
     */
	bool SetDecelerationTime(double time);

    /**
     * @brief Set the Watchdog timer for Modbus communication.
     * After set this timer and start motor the client should send new
     * Modbus request in peirods less than specified time. Otherwise the
     * motor will stop.
     * Set 0 to switch off watchdog timer
     * 
     * @param time[in]	- Watchdog time to set
     * @return true     - Set watchdog success
     * @return false    - Set time fail
     */
	bool SetWatchdog(double time = 0);

    /**
     * @brief Allows to Get and Set VFD parameters directly
     * 
     * @param addr      - parameter address
     * @param val       - pointer to value where parameter will be stored or value to write into VFD
     * @return true     - if success
     * @return false    - if fail
     */
	bool GetParam(unsigned short addr, unsigned short* val);
	bool SetParam(unsigned short addr, unsigned short val);
};

#endif // VFD_H
