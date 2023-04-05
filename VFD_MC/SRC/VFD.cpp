#include ".\INC\VFD.H" // 7188EX uPAC

#include <math.h> // for calculations

//#define NDEBUG
#include <assert.h>

VFD::VFD(ModbusRTUClient mb) :
	MB(mb),
	maxFrequency(50.0)
{
#ifndef NDEBUG
	Print("VFD::Constructor() Created instance 0x%p with params:\n", this);
	Print("- MB: 0x%p\n", &(this->MB));
#endif // NDEBUG
}

VFD::~VFD()
{
#ifndef NDEBUG
	Print("VFD::Destructor() Deleted instance 0x%p\n", this);
#endif // NDEBUG
}

char VFD::Run(unsigned short direction /* = 0 */)
{
#ifndef NDEBUG
	Print("VFD::Run() Run started\n");
	unsigned long start_time = GetTimeTicks();
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
		return 0;
	}
#ifndef NDEBUG
	Print("VFD::Run() Success in %ldms\n",
		(GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::Stop(void)
{
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	unsigned short command = 0;
	// Set stop bit
	command |= 1 << 0;
	if (!MB.WriteSingleRegister(0x2000, command))
	{
		assert(("VFD::Run() Stop error", 0));
		return 0;
	}
	if (!SetWatchdog(0)) return 0;
#ifndef NDEBUG
	Print("VFD::Stop() Success in %ldms\n",
		(GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::ChangeFrequency(
	float curFreq /* = 0 */,
	float newFreq /* = 50 */,
	float changeTime /* = 1 */)
{
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	if (fabs(newFreq - curFreq) < 0.1) return 1; // new frequency remains the same
	// Acceleration or deceleration time
	float accDecTime = maxFrequency * changeTime / fabs(newFreq - curFreq);
	if ((curFreq * newFreq) < 0) // Direction changes
	{
		if (!SetDecelerationTime(accDecTime) || !SetAccelerationTime(accDecTime))
			return 0;
		// Run motor in the different direction
		if (!Run(3)) return 0;
		// Set new frequency
		if (!SetFrequency(fabs(newFreq))) return 0;
	}
	else // Direction remains the same
	{
		// Set new acceleration/deceleration time
		if (fabs(newFreq) > fabs(curFreq)) // Motor frequency increases
		{
			if (!SetAccelerationTime(accDecTime)) return 0;
		}
		else // Motor frequency decreases
		{
			if (!SetDecelerationTime(accDecTime)) return 0;
		}
		// Set new frequency 
		if (!SetFrequency(fabs(newFreq))) return 0;

		if (fabs(curFreq) < 0.1) // If start from zero
		{
			if (newFreq > 0) // Run forward
			{
				if (!Run(1)) return 0;
			}
			else			// Run reverse
			{
				if (!Run(2)) return 0;
			}
		}
	}
#ifndef NDEBUG
	Print("VFD::ChangeFrequency() Frequency changed in %ldms\n",
		(GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::ReadParameterRegisters(VFD_status_t* status, VFD_param_t* param)
{
	const unsigned short firstReg = 0x2101; // fisrt register to start reading
	const unsigned char nReg = 12; // number of registers to read
	unsigned short regArray[nReg]; // array to store registers values
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	// Read registers
		if (!MB.ReadHoldingRegisters(firstReg, nReg, regArray))
	{
		assert(("VFD::ReadParameterRegisters() Read registers error", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("VFD::ReadParameterRegisters() Read %u parameters in %ldms\n",
		nReg, (GetTimeTicks() - start_time));
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
	Print("VFD::ReadParameterRegisters() Status:\n");
	Print("- LED: {RUN: %u, STOP: %u, JOG: %u, FWD: %u, REW: %u}\n",
		status->LED.RUN, status->LED.STOP, status->LED.JOG, status->LED.FWD, status->LED.REW);
	Print("- F: %u, H: %u, u: %u\n",
		status->F, status->H, status->u);
	Print("- controlFrequencyBySerialInterface: %u\n", status->controlFrequencyBySerialInterface);
	Print("- controlFrequencyByAnalogSignal: %u\n", status->controlFrequencyByAnalogSignal);
	Print("- controlVFDBySerialInterface: %u\n", status->controlVFDBySerialInterface);
	Print("- parametersBlocked: %u\n", status->parametersBlocked);
	Print("- VFDCurrentState: %u\n", status->VFDCurrentState);
	Print("- JOGcommand: %u\n", status->JOGcommand);
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
	Print("VFD::ReadParameterRegisters() Parameters:\n");
	Print("- FrequencyCommand: %gHz\n", param->FrequencyCommand);
	Print("- OutFrequency: %gHz\n", param->OutFrequency);
	Print("- OutCurrent: %gA\n", param->OutCurrent);
	Print("- DCVoltage: %gV\n", param->DCVoltage);
	Print("- OutVoltage: %gV\n", param->OutVoltage);
	Print("- PowerFactor: %g\n", param->PowerFactor);
	Print("- OutTorque: %gNm\n", param->OutTorque);
	Print("- MotorSpeed: %grpm\n", param->MotorSpeed);
	//Print("- OutPower: %gkW\n", param->OutPower);
#endif // NDEBUG
	return 1;
}

char VFD::GetOutPower(float* power)
{
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	unsigned short regValue;
	if (!MB.ReadHoldingRegisters(0x210F, 1, &regValue))
	{
		assert(("VFD::GetOutPower() Read power error", 0));
		return 0;
	}
	*power = regValue / 10.0;
#ifndef NDEBUG
	Print("VFD::GetOutPower() Read power: %gdegC in %ldms\n",
		*power, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::GetVFDTemperature(float* temp)
{
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	unsigned short regValue;
	if (!MB.ReadHoldingRegisters(0x2206, 1, &regValue))
	{
		assert(("VFD::GetVFDTemperature() Read temperature error", 0));
		return 0;
	}
	*temp = regValue / 1.0;
#ifndef NDEBUG
	Print("VFD::GetVFDTemperature() Read temperature: %gdegC in %ldms\n",
		*temp, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::ReadMaxFrequency(float* maxFreq /* = 0 */)
{
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	unsigned short regValue;
	if (!MB.ReadHoldingRegisters(0x0100, 1, &regValue)) // (read 01-00 parameter)
	{
		assert(("VFD::ReadMaxFrequency() Read max frequency error", 0));
		return 0;
	}
	maxFrequency = regValue / 100.0;
	if (maxFreq != 0) *maxFreq = maxFrequency;
#ifndef NDEBUG
	Print("VFD::ReadMaxFrequency() Read max frequency: %gHz in %ldms\n",
		maxFrequency, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::SetFrequency(float freq)
{
#ifndef NDEBUG
	Print("VFD::SetFrequency() Start set\n");
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	// restrict values according to VFD-B_manual_rus.pdf
	if (freq < 0) freq = 0;
	if (freq > maxFrequency) freq = maxFrequency;
	unsigned short regVal = (unsigned short)(freq * 100.0);
	//unsigned short regVal = (unsigned short)round(freq * 100.0);
	if (!MB.WriteSingleRegister(0x2001, regVal))
	{
		assert(("VFD::SetFrequency() Set frequency error", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("VFD::SetFrequency() Frequency %gHz set in %ldms\n",
		freq, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::SetAccelerationTime(float time)
{
#ifndef NDEBUG
	Print("VFD::SetAccelerationTime() Start set\n");
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	// restrict values according to VFD-B_manual_rus.pdf
	if (time < 0.1) time = 0.1;
	if (time > 3600) time = 3600;
	unsigned short regVal = (unsigned short)(time * 10.0);
	//unsigned short regVal = (unsigned short)round(time * 10.0);
	if (!MB.WriteSingleRegister(0x0109, regVal)) // (write 01-09 parameter)
	{
		assert(("VFD::SetAccelerationTime() Set acceleration time error", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("VFD::SetAccelerationTime() Acceleration time %gs set in %ldms\n",
		time, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::SetDecelerationTime(float time)
{
#ifndef NDEBUG
	Print("VFD::SetDecelerationTime() Start set\n");
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	// restrict values according to VFD-B_manual_rus.pdf
	if (time < 0.1) time = 0.1;
	if (time > 3600) time = 3600;
	unsigned short regVal = (unsigned short)(time * 10.0);
	//unsigned short regVal = (unsigned short)round(time * 10.0);
	if (!MB.WriteSingleRegister(0x010A, regVal)) // (write 01-10 parameter)
	{
		assert(("VFD::SetDecelerationTime() Set deceleration time error", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("VFD::SetDecelerationTime() Deceleration time %gs set in %ldms\n",
		time, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::SetWatchdog(float time  /* = 0 */)
{
#ifndef NDEBUG
	Print("VFD::SetWatchdog() Start set\n");
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	// restrict values according to VFD-B_manual_rus.pdf
	if (time < 0.0) time = 0.0;
	if (time > 60.0) time = 60.0;
	unsigned short regVal = (unsigned short)(time * 10.0);
	//unsigned short regVal = (unsigned short)round(time * 10.0);
	if (!MB.WriteSingleRegister(0x0903, regVal)) // (write 09-03 parameter)
	{
		assert(("VFD::SetWatchdog() Set watchdog time error", 0));
		return 0;
	}
	if (time > 0) regVal = 02;	// Warn and COAST to stop
	else regVal = 03;			// No warning and keep operating
	if (!MB.WriteSingleRegister(0x0902, regVal)) // (write 09-02 parameter)
	{
		assert(("VFD::SetWatchdog() Switch watchdog error", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("VFD::SetWatchdog() Watchdog time %gs set in %ldms\n",
		time, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::GetParam(unsigned short addr, unsigned short* val)
{
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	if (!MB.ReadHoldingRegisters(addr, 1, val))
	{
		assert(("VFD::GetParam() Get parameter error", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("VFD::GetParam() Parameter 0x%04X: 0x%04X read in %ldms\n",
		addr, *val,  (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}

char VFD::SetParam(unsigned short addr, unsigned short val)
{
#ifndef NDEBUG
	Print("VFD::SetParam() Start set\n");
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	if (!MB.WriteSingleRegister(addr, val))
	{
		assert(("VFD::SetParam() Set parameter error", 0));
		return 0;
	}
#ifndef NDEBUG
	Print("VFD::SetParam() Parameter 0x%04X: 0x%04X set in %ldms\n",
		addr, val, (GetTimeTicks() - start_time));
#endif // NDEBUG
	return 1;
}
