#ifdef _I7188E_
#include ".\INC\VFD.H"
#else
#include "..\INC\VFD.H"
#endif

#include <math.h> // for calculations

//#define NDEBUG
#include <assert.h>
#ifndef NDEBUG
#include <stdio.h>   // for debug printing
#include <time.h>    // for transfer time measure
#endif // NDEBUG

VFD::VFD(ModbusRTUClient mb /* = { 1, {portName, 9600, 8, 'E', 1} } */) :
	MB(mb),
	maxFrequency(50.0)
{
#ifndef NDEBUG
	printf("VFD::Constructor() Created instance 0x%p with params:\n", this);
	printf("- MB: 0x%p\n", &(this->MB));
#endif // NDEBUG
}

//VFD::VFD(VFD& other) : MB(other.MB), commStatus(other.commStatus) {}
//
//VFD& VFD::operator =(VFD& other)
//{
//	if (this != &other)
//	{
//		MB = other.MB;
//		commStatus = other.commStatus;
//	}
//	return (*this);
//}

//VFD::VFD(VFD&& other) noexcept : MB(other.MB),commStatus(other.commStatus) {}
//
//VFD& VFD::operator =(VFD&& other) noexcept
//{
//	if (this != &other)
//	{
//		MB = other.MB;
//		commStatus = other.commStatus;
//	}
//	return (*this);
//}

VFD::~VFD()
{
#ifndef NDEBUG
	printf("VFD::Destructor() Deleted instance 0x%p\n", this);
#endif // NDEBUG
}

bool VFD::Run(unsigned short direction /* = 0 */)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	if (direction > 3) direction = 0; // Prevent invalid direction parameter set
	unsigned short command = 0;
	// Set start bit
	command |= 1 << 1;
	// Set direction bits
	command |= direction << 4;
	if (!MB.WriteSingleRegister(0x2000, command))
	{
		assert(("VFD::Run() Run error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("VFD::Run() Success in %ldms\n",
		(clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::Stop()
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	unsigned short command = 0;
	// Set stop bit
	command |= 1 << 0;
	if (!MB.WriteSingleRegister(0x2000, command))
	{
		assert(("VFD::Run() Stop error", 0));
		return false;
	}
	if (!SetWatchdog(0)) return false;
#ifndef NDEBUG
	printf("VFD::Stop() Success in %ldms\n",
		(clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::ChangeFrequency(
	double curFreq /* = 0 */,
	double newFreq /* = 50 */,
	double changeTime /* = 1 */)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	if (fabs(newFreq - curFreq) < 0.1) return true; // new frequency remains the same
	// Acceleration or deceleration time
	double accDecTime = maxFrequency * changeTime / fabs(newFreq - curFreq);
	if ((curFreq * newFreq) < 0) // Direction changes
	{
		if (!SetDecelerationTime(accDecTime) || !SetAccelerationTime(accDecTime))
			return false;
		// Run motor in the different direction
		if (!Run(3)) return false;
		// Set new frequency
		if (!SetFrequency(fabs(newFreq))) return false;
	}
	else // Direction remains the same
	{
		// Set new acceleration/deceleration time
		if (fabs(newFreq) > fabs(curFreq)) // Motor frequency increases
		{
			if (!SetAccelerationTime(accDecTime)) return false;
		}
		else // Motor frequency decreases
		{
			if (!SetDecelerationTime(accDecTime)) return false;
		}
		// Set new frequency 
		if (!SetFrequency(fabs(newFreq))) return false;

		if (fabs(curFreq) < 0.1) // If start from zero
		{
			if (newFreq > 0) // Run forward
			{
				if (!Run(1)) return false;
			}
			else			// Run reverse
			{
				if (!Run(2)) return false;
			}
		}
	}
#ifndef NDEBUG
	printf("VFD::ChangeFrequency() Frequency changed in %ldms\n",
		(clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::ReadParameterRegisters(VFD_status_t* status, VFD_param_t* param)
{
	const unsigned short firstReg = 0x2101; // fisrt register to start reading
	const unsigned char nReg = 12; // number of registers to read
	unsigned short regArray[nReg]; // array to store registers values
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	// Read registers
		if (!MB.ReadHoldingRegisters(firstReg, nReg, regArray))
	{
		assert(("VFD::ReadParameterRegisters() Read registers error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("VFD::ReadParameterRegisters() Read %u parameters in %ldms\n",
		nReg, (clock() - start_time));
#endif // NDEBUG
	// Fill status structure
	status->LED.RUN = (regArray[0] >> 0) & 0x1;
	status->LED.STOP = (regArray[0] >> 1) & 0x1;
	status->LED.JOG = (regArray[0] >> 2) & 0x1;
	status->LED.FWD = (regArray[0] >> 3) & 0x1;
	status->LED.REW = (regArray[0] >> 4) & 0x1;
	status->F = (regArray[0] >> 5) & 0x1;
	status->H = (regArray[0] >> 6) & 0x1;
	status->u = (regArray[0] >> 7) & 0x1;
	status->controlFrequencyBySerialInterface = (regArray[0] >> 8) & 0x1;
	status->controlFrequencyByAnalogSignal = (regArray[0] >> 9) & 0x1;
	status->controlVFDBySerialInterface = (regArray[0] >> 10) & 0x1;
	status->parametersBlocked = (regArray[0] >> 11) & 0x1;
	status->VFDCurrentState = (regArray[0] >> 12) & 0x1;
	status->JOGcommand = (regArray[0] >> 13) & 0x1;
#ifndef NDEBUG
	printf("VFD::ReadParameterRegisters() Status:\n");
	printf("- LED: {RUN: %u, STOP: %u, JOG: %u, FWD: %u, REW: %u}\n",
		status->LED.RUN, status->LED.STOP, status->LED.JOG, status->LED.FWD, status->LED.REW);
	printf("- F: %u, H: %u, u: %u\n",
		status->F, status->H, status->u);
	printf("- controlFrequencyBySerialInterface: %u\n", status->controlFrequencyBySerialInterface);
	printf("- controlFrequencyByAnalogSignal: %u\n", status->controlFrequencyByAnalogSignal);
	printf("- controlVFDBySerialInterface: %u\n", status->controlVFDBySerialInterface);
	printf("- parametersBlocked: %u\n", status->parametersBlocked);
	printf("- VFDCurrentState: %u\n", status->VFDCurrentState);
	printf("- JOGcommand: %u\n", status->JOGcommand);
#endif // NDEBUG
	// Fill parameters structure
	param->FrequencyCommand = regArray[0x2102 - firstReg] / 100.0;
	param->OutFrequency = regArray[0x2103 - firstReg] / 100.0;
	param->OutCurrent = regArray[0x2104 - firstReg] / 10.0;
	param->DCVoltage = regArray[0x2105 - firstReg] / 10.0;
	param->OutVoltage = regArray[0x2106 - firstReg] / 10.0;
	param->PowerFactor = regArray[0x210A - firstReg] / 100.0;
	param->OutTorque = regArray[0x210B - firstReg] / 10.0;
	param->MotorSpeed = regArray[0x210C - firstReg] / 1.0;
	if (status->LED.REW)
	{
		param->FrequencyCommand *= -1;
		param->OutFrequency *= -1;
		param->OutTorque *= -1;
		param->MotorSpeed *= -1;
	}
#ifndef NDEBUG
	printf("VFD::ReadParameterRegisters() Parameters:\n");
	printf("- FrequencyCommand: %gHz\n", param->FrequencyCommand);
	printf("- OutFrequency: %gHz\n", param->OutFrequency);
	printf("- OutCurrent: %gA\n", param->OutCurrent);
	printf("- DCVoltage: %gV\n", param->DCVoltage);
	printf("- OutVoltage: %gV\n", param->OutVoltage);
	printf("- PowerFactor: %g\n", param->PowerFactor);
	printf("- OutTorque: %gNm\n", param->OutTorque);
	printf("- MotorSpeed: %grpm\n", param->MotorSpeed);
	//printf("- OutPower: %gkW\n", param->OutPower);
#endif // NDEBUG
	return true;
}

bool VFD::GetOutPower(double* power)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	unsigned short regValue;
	if (!MB.ReadHoldingRegisters(0x210F, 1, &regValue))
	{
		assert(("VFD::GetOutPower() Read power error", 0));
		return false;
	}
	*power = regValue / 10.0;
#ifndef NDEBUG
	printf("VFD::GetOutPower() Read power: %gdegC in %ldms\n",
		*power, (clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::GetVFDTemperature(double* temp)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	unsigned short regValue;
	if (!MB.ReadHoldingRegisters(0x2206, 1, &regValue))
	{
		assert(("VFD::GetVFDTemperature() Read temperature error", 0));
		return false;
	}
	*temp = regValue / 1.0;
#ifndef NDEBUG
	printf("VFD::GetVFDTemperature() Read temperature: %gdegC in %ldms\n",
		*temp, (clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::ReadMaxFrequency(double* maxFreq /* = nullptr */)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	unsigned short regValue;
	if (!MB.ReadHoldingRegisters(0x0100, 1, &regValue)) // (read 01-00 parameter)
	{
		assert(("VFD::ReadMaxFrequency() Read max frequency error", 0));
		return false;
	}
	maxFrequency = regValue / 100.0;
	if (maxFreq != nullptr) *maxFreq = maxFrequency;
#ifndef NDEBUG
	printf("VFD::ReadMaxFrequency() Read max frequency: %gHz in %ldms\n",
		maxFrequency, (clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::SetFrequency(double freq)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	// restrict values according to VFD-B_manual_rus.pdf
	if (freq < 0) freq = 0;
	if (freq > maxFrequency) freq = maxFrequency;
	unsigned short regVal = (unsigned short)round(freq * 100.0);
	if (!MB.WriteSingleRegister(0x2001, regVal))
	{
		assert(("VFD::SetFrequency() Set frequency error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("VFD::SetFrequency() Frequency %gHz set in %ldms\n",
		freq, (clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::SetAccelerationTime(double time)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	// restrict values according to VFD-B_manual_rus.pdf
	if (time < 0.1) time = 0.1;
	if (time > 3600) time = 3600;
	unsigned short regVal = (unsigned short)round(time * 10.0);
	if (!MB.WriteSingleRegister(0x0109, regVal)) // (write 01-09 parameter)
	{
		assert(("VFD::SetAccelerationTime() Set acceleration time error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("VFD::SetAccelerationTime() Acceleration time %gs set in %ldms\n",
		time, (clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::SetDecelerationTime(double time)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	// restrict values according to VFD-B_manual_rus.pdf
	if (time < 0.1) time = 0.1;
	if (time > 3600) time = 3600;
	unsigned short regVal = (unsigned short)round(time * 10.0);
	if (!MB.WriteSingleRegister(0x010A, regVal)) // (write 01-10 parameter)
	{
		assert(("VFD::SetDecelerationTime() Set deceleration time error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("VFD::SetDecelerationTime() Deceleration time %gs set in %ldms\n",
		time, (clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::SetWatchdog(double time  /* = 0 */)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	// restrict values according to VFD-B_manual_rus.pdf
	if (time < 0.0) time = 0.0;
	if (time > 60.0) time = 60.0;
	unsigned short regVal = (unsigned short)round(time * 10.0);
	if (!MB.WriteSingleRegister(0x0903, regVal)) // (write 09-03 parameter)
	{
		assert(("VFD::SetWatchdog() Set watchdog time error", 0));
		return false;
	}
	if (time > 0) regVal = 02;	// Warn and COAST to stop
	else regVal = 03;			// No warning and keep operating
	if (!MB.WriteSingleRegister(0x0902, regVal)) // (write 09-02 parameter)
	{
		assert(("VFD::SetWatchdog() Switch watchdog error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("VFD::SetWatchdog() Watchdog time %gs set in %ldms\n",
		time, (clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::GetParam(unsigned short addr, unsigned short* val)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	if (!MB.ReadHoldingRegisters(addr, 1, val))
	{
		assert(("VFD::GetParam() Get parameter error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("VFD::GetParam() Parameter 0x%04X: 0x%04X read in %ldms\n",
		addr, *val,  (clock() - start_time));
#endif // NDEBUG
	return true;
}

bool VFD::SetParam(unsigned short addr, unsigned short val)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	if (!MB.WriteSingleRegister(addr, val))
	{
		assert(("VFD::SetParam() Set parameter error", 0));
		return false;
	}
#ifndef NDEBUG
	printf("VFD::SetParam() Parameter 0x%04X: 0x%04X set in %ldms\n",
		addr, val, (clock() - start_time));
#endif // NDEBUG
	return true;
}
