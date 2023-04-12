/**
 * @file main.cpp
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief VFDMotorControl main file
 *

Help
This program can control Delta VFD-B.
Input commandline arguments:
-h | --help         Display this help message
--port <COMx>       Specify serial port (COM3 default) (--port COM3)
--file <text_file>	Read a file with frequency and time parameters table. (--file coords.txt)
					And run motor according to the table.
Text file should contain table with times and frequencies and should look like this:
Time	Frequency
0	0
10	30
20	20
40	-10
50	0
(The first column specifies the time to be set.
The second column specifies the frequency that will be reached for the time,
specified in the first column.)
--get <parameter>   Read one of the following motor parameters: (--get MotorSpeed)
					<FrequencyCommand>
					<OutFrequency>
					<OutCurrent>
					<DCVoltage>
					<OutVoltage>
					<PowerFactor>
					<OutTorque>
					<MotorSpeed>
					<OutPower>
					<VFDTemperature>
					<0x(parameter address)> (--get 0x0109)
--set <parameter> <value>  Set one of the folloving motor parameters: (--set Frequency 45.5)
					<Frequency>
					<AccelerationTime>
					<DecelerationTime>
					<0x(parameter address)> (--set 0x2001 0x1388)
--run <n|f|r|c>     Run motor with direction set (no change, forvard, reverse, change) (--run r)
--stop              Stop motor

 *
 * @version 0.2
 * @date 2023-03-01
 *
 * @copyright Copyright (c) 2023 TAN4UK
 *
 */

 // Includes ///////////////////////////////////////////////////////////////////

#include "MAIN.H"

// Global variables ///////////////////////////////////////////////////////////

// CLI keys flags and values
struct {
	char help;
	char file;
	char get;
	char set;
	char run;
	char stop;
} CMD;
char portName[9] = "COM3";			// port name from command line
char* diagramFileName = 0;	// file name with diagram
// Get parameters flags
struct {
	char FrequencyCommand;
	char OutFrequency;
	char OutCurrent;
	char DCVoltage;
	char OutVoltage;
	char PowerFactor;
	char OutTorque;
	char MotorSpeed;
	char OutPower;
	char VFDTemperature;
	char reg;
} getParam;
unsigned short getReg_a;			// stores register address that has to be read
unsigned short getReg_v;			// stores register value from VFD
VFD_status_t motorStatus = { 0 };	// Stores motor status
VFD_param_t motorParams = { 0 };	// Stores motor parameters
float OutPower;						// Stores power provided to motor
float VFDtemperature;				// Stores temperature of VFD heatsink
// Set parameters flags and values
struct {
	char Frequency_f;				// Set Frequency flag
	float Frequency_v;				// Set Frequency value
	char AccelerationTime_f;		// Set AccelerationTime flag
	float AccelerationTime_v;		// Set AccelerationTime value
	char DecelerationTime_f;		// Set DecelerationTime flag
	float DecelerationTime_v;		// Set DecelerationTime value
	char reg_f;						// Set register flag
	unsigned short reg_a;			// Set register address
	unsigned short reg_v;			// Set register value
} setParam;
unsigned short runMode; // 0 - no change, 1 - forward, 2 - reverse, 3 - change

// Function prototypes ////////////////////////////////////////////////////////
/**
 * @brief Read CLI arguments and set flags according to input arguments.
 * Take input arguments for this function from main(int argc, char* argv[])
 *
 * @param argc[in]	- number of input arguments passed by user
 * @param argv[in]	- array of character pointers listing all the arguments
 */
void HandleCLIArguments(int argc, char* argv[]);

/**
 * @brief Print help message about program and
 * available command line arguments
 *
 */
void PrintHelp(void);

/**
 * @brief Set the Motor Parameters specified by user
 *
 * @param motor[in]	- reference to VFD class instance
 * @return 1		- if all motor parameters have set successfully
 * @return 0	- if some error occured
 */
char SetMotorParameters(VFD& motor);

/* Main function *************************************************************/
/**
 * @brief Program entry point. Accepts CLI arguments provided by user
 *
 * @param argc[in]	- number of input arguments passed by user
 * @param argv[in]	- array of character pointers listing all the arguments
 * @return int		- 0 if program have finished successfully, -1 otherwise
 */
int main(int argc, char* argv[])
{
	// Init 7188 library //////////////////////////////////////////////////////
	InitLib();
	// Check demo version /////////////////////////////////////////////////////
	//time_t currentTime = time(NULL);
	//struct tm localTime;
	//localtime_s(&localTime, &currentTime);
	//int year = localTime.tm_year + 1900;
	//int month = localTime.tm_mon + 1;
	//int day = localTime.tm_mday;
	//int hour = localTime.tm_hour;
	//int min = localTime.tm_min;
	//if ((year >= 2023) && (month >= 3) && (day >= 29) && (hour >= 0) && (min >= 0))
	//{
	//	Print("Demo version expired! Contact with tan4ukmak7@gmail.com to get full version.\n");
	//	return -1;
	//}

	// CLI arguments handle ///////////////////////////////////////////////////
	HandleCLIArguments(argc, argv);
	if (argc <= 1) // if run without arguments
	{
		Print("This program requires input arguments.");
		PrintHelp();
		return 0; // after help print program doesn't accept any commands
	}
	// Print help text ////////////////////////////////////////////////////////
	if (CMD.help) PrintHelp();

	VFD motor(ModbusRTUClient(1, COMPort(portName, 9600, 8, 'E', 1))); // VFD class instance

	// File handling //////////////////////////////////////////////////////////
	if (CMD.file)
	{
		if (!RunDiagramFromFile(motor)) return -1;
		// after diagram from file running program doesn't accept any commands
		return 0;
	}
	// Get param handling /////////////////////////////////////////////////////
	if (CMD.get)
	{
		if (!GetMotorParameters(motor)) return -1;
		PrintParametersHeader();
		PrintParameters();
	}
	// Set param handling /////////////////////////////////////////////////////
	if (CMD.set)
	{
		if (!SetMotorParameters(motor)) return -1;
	}
	// Run handling ///////////////////////////////////////////////////////////
	if (CMD.run)
	{
#ifndef NDEBUG
		unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
		if (!motor.Run(runMode))
		{
			assert(("main: Run error", 0));
			return -1;
		}
#ifndef NDEBUG
		Print("main: Motor started in %ldms\n", (GetTimeTicks() - start_time));
#endif // NDEBUG
	}
	// Stop handling //////////////////////////////////////////////////////////
	if (CMD.stop)
	{
#ifndef NDEBUG
		unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
		if (!motor.Stop())
		{
			assert(("main: Stop error", 0));
			return -1;
		}
#ifndef NDEBUG
		Print("main: Motor stopped in %ldms\n", (GetTimeTicks() - start_time));
#endif // NDEBUG
	}
	return 0;
}

void HandleCLIArguments(int argc, char* argv[])
{
	// Handle arguments and find matches
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == 0) continue;
#ifndef NDEBUG
		Print("main::HandleCLIArguments() Arg %d: %s\n", i, argv[i]);
#endif // NDEBUG
		// Handle --help argument
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h") || !strcmp(argv[i], "/?"))
		{
			CMD.help = 1;
		}
		// Handle --port argument
		else if (!strcmp(argv[i], "--port"))
		{
			if (argv[i + 1] != 0)
				strcpy(portName, argv[i + 1]);
		}
		// Handle --file argument
		else if (!strcmp(argv[i], "--file"))
		{
			if (argv[i + 1] != 0)
			{
				CMD.file = 1;
				diagramFileName = argv[i + 1];
			}
		}
		// Handle --get argument
		else if (!strcmp(argv[i], "--get"))
		{
			CMD.get = 1;
			if (argv[i + 1] != 0)
			{
				if (!strcmp(argv[i + 1], "FrequencyCommand"))
					getParam.FrequencyCommand = 1;
				else if (!strcmp(argv[i + 1], "OutFrequency"))
					getParam.OutFrequency = 1;
				else if (!strcmp(argv[i + 1], "OutCurrent"))
					getParam.OutCurrent = 1;
				else if (!strcmp(argv[i + 1], "DCVoltage"))
					getParam.DCVoltage = 1;
				else if (!strcmp(argv[i + 1], "OutVoltage"))
					getParam.OutVoltage = 1;
				else if (!strcmp(argv[i + 1], "PowerFactor"))
					getParam.PowerFactor = 1;
				else if (!strcmp(argv[i + 1], "OutTorque"))
					getParam.OutTorque = 1;
				else if (!strcmp(argv[i + 1], "MotorSpeed"))
					getParam.MotorSpeed = 1;
				else if (!strcmp(argv[i + 1], "OutPower"))
					getParam.OutPower = 1;
				else if (!strcmp(argv[i + 1], "VFDTemperature"))
					getParam.VFDTemperature = 1;
				else
				{
					getParam.reg = 1;
					if (argv[i + 1] != 0) // read register address that has to be read
						sscanf(argv[i + 1], "%hX", &getReg_a);
				}
			}
		}
		// Handle --set argument
		else if (!strcmp(argv[i], "--set"))
		{
			CMD.set = 1;
			if (argv[i + 1] != 0)
			{
				if (!strcmp(argv[i + 1], "Frequency"))
				{
					setParam.Frequency_f = 1;
					if (argv[i + 2] != 0) setParam.Frequency_v = atof(argv[i + 2]);
				}
				else if (!strcmp(argv[i + 1], "AccelerationTime"))
				{
					setParam.AccelerationTime_f = 1;
					if (argv[i + 2] != 0) setParam.AccelerationTime_v = atof(argv[i + 2]);
				}
				else if (!strcmp(argv[i + 1], "DecelerationTime"))
				{
					setParam.DecelerationTime_f = 1;
					if (argv[i + 2] != 0) setParam.DecelerationTime_v = atof(argv[i + 2]);
				}
				else
				{
					setParam.reg_f = 1;
					if (argv[i + 1] != 0) // read register address that has to be set
					{
						sscanf(argv[i + 1], "%hX", &(setParam.reg_a));
						if (argv[i + 2] != 0) // read register value that has to be set
							sscanf(argv[i + 2], "%hX", &(setParam.reg_v));
					}
				}
			}
		}
		// Handle --run argument
		else if (!strcmp(argv[i], "--run"))
		{
			CMD.run = 1;
			if (argv[i + 1] != 0)
			{
				switch (argv[i + 1][0])
				{
				case 'f':
					runMode = 1; // forward
					break;
				case 'r':
					runMode = 2; // reverse
					break;
				case 'c':
					runMode = 3; // change
					break;
				default:
					runMode = 0; // no change
					break;
				}
			}
		}
		// Handle --stop argument
		else if (!strcmp(argv[i], "--stop"))
		{
			CMD.stop = 1;
		}
	}
	return;
}

void PrintHelp(void)
{
	Print("\nHelp.\n");
	Print("This program can control Delta VFD-B.\n");
	Print("Input commandline arguments:\n");
	Print("-h | --help\t\t\tDisplay this help message\n");
	Print("--port <COMx>\t\t\tSpecify serial port (COM3 default) (--port COM3)\n");
	Print("--file <text_file>\t\tRead a file with frequency and time parameters table. (--file coords.txt)\n");
	Print("\t\t\t\tAnd run motor according to the table.\n");
	Print("Text file should contain table with times and frequencies and should look like this:\n");
	Print("Time\tFrequency\n");
	Print("0\t0\n");
	Print("10\t30\n");
	Print("20\t20\n");
	Print("40\t-10\n");
	Print("50\t0\n");
	Print("(The first column specifies the time to be set.\n");
	Print("The second column specifies the frequency that will be reached for the time,\n");
	Print("specified in the first column.)\n");
	Print("--get <parameter>\t\tRead one of the following motor parameters: (--get MotorSpeed)\n");
	Print("\t\t\t\t<FrequencyCommand>\n");
	Print("\t\t\t\t<OutFrequency>\n");
	Print("\t\t\t\t<OutCurrent>\n");
	Print("\t\t\t\t<DCVoltage>\n");
	Print("\t\t\t\t<OutVoltage>\n");
	Print("\t\t\t\t<PowerFactor>\n");
	Print("\t\t\t\t<OutTorque>\n");
	Print("\t\t\t\t<MotorSpeed>\n");
	Print("\t\t\t\t<OutPower>\n");
	Print("\t\t\t\t<VFDTemperature>\n");
	Print("\t\t\t\t<0x(parameter address)> (--get 0x0109)\n");
	Print("--set <parameter> <value>\tSet one of the folloving motor parameters: (--set Frequency 45.5)\n");
	Print("\t\t\t\t<Frequency>\n");
	Print("\t\t\t\t<AccelerationTime>\n");
	Print("\t\t\t\t<DecelerationTime>\n");
	Print("\t\t\t\t<0x(parameter address)> (--set 0x2001 0x1388)\n");
	Print("--run <n|f|r|c>\t\t\tRun motor with direction set (no change, forvard, reverse, change) (--run r)\n");
	Print("--stop\t\t\t\tStop motor\n\n");
}

char GetMotorParameters(VFD& motor)
{
#ifndef NDEBUG
	unsigned long start_time = GetTimeTicks();
#endif // NDEBUG
	// Read all parameters and status
	if (!motor.ReadParameterRegisters(&motorStatus, &motorParams))
	{
		assert(("main::GetMotorParameters() Read parameters error", 0));
		return 0;
	}
	// Read OutPower parameter
	if (getParam.OutPower)
	{
		if (!motor.GetOutPower(&OutPower))
		{
			assert(("main::GetMotorParameters() Read power error", 0));
			return 0;
		}
	}
	// Read VFDTemperature parameter
	if (getParam.VFDTemperature)
	{
		if (!motor.GetVFDTemperature(&VFDtemperature))
		{
			assert(("main::GetMotorParameters() Read temperature error", 0));
			return 0;
		}
	}
	if (getParam.reg)
	{
		if (!motor.GetParam(getReg_a, &getReg_v))
		{
			assert(("main::GetMotorParameters() Get parameter error", 0));
			return 0;
		}
	}
#ifndef NDEBUG
	Print("main::GetMotorParameters() Read param time: %ld\n", GetTimeTicks() - start_time);
#endif // NDEBUG
	return 1;
}

void PrintParametersHeader(char Time /* = 0 */)
{
	// printStream is used for print parameters to file, but it unused on 7188
	// Print parameters header for parameters given in --get argument
	char firstTime = 1; // this flag is for avoid print unnecessary tabs
	if (Time)
	{
		firstTime = 0;
		Print("Time");
	}
	if (getParam.FrequencyCommand)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("FrequencyCommand");
	}
	if (getParam.OutFrequency)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("OutFrequency");
	}
	if (getParam.OutCurrent)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("OutCurrent");
	}
	if (getParam.DCVoltage)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("DCVoltage");
	}
	if (getParam.OutVoltage)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("OutVoltage");
	}
	if (getParam.PowerFactor)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("PowerFactor");
	}
	if (getParam.OutTorque)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("OutTorque");
	}
	if (getParam.MotorSpeed)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("MotorSpeed");
	}
	if (getParam.OutPower)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("OutPower");
	}
	if (getParam.VFDTemperature)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("VFDTemperature");
	}
	if (getParam.reg)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("Param0x%04X", getReg_a);
	}
	Print("\n");
}

void PrintParameters(float Time /* = -1 */)
{
	// printStream is used for print parameters to file, but it unused on 7188
	// Print parameters given in --get argument
	char firstTime = 1; // this flag is for avoid print unnecessary tabs
	if (Time > -0.5)
	{
		firstTime = 0;
		Print("%.2f", Time);
	}
	if (getParam.FrequencyCommand)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", motorParams.FrequencyCommand);
	}
	if (getParam.OutFrequency)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", motorParams.OutFrequency);
	}
	if (getParam.OutCurrent)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", motorParams.OutCurrent);
	}
	if (getParam.DCVoltage)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", motorParams.DCVoltage);
	}
	if (getParam.OutVoltage)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", motorParams.OutVoltage);
	}
	if (getParam.PowerFactor)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", motorParams.PowerFactor);
	}
	if (getParam.OutTorque)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", motorParams.OutTorque);
	}
	if (getParam.MotorSpeed)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", motorParams.MotorSpeed);
	}
	if (getParam.OutPower)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", OutPower);
	}
	if (getParam.VFDTemperature)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("%g", VFDtemperature);
	}
	if (getParam.reg)
	{
		if (firstTime) firstTime = 0;
		else Print("\t");
		Print("0x%04X", getReg_v);
	}
	Print("\n");
}

char SetMotorParameters(VFD& motor)
{
	// Set frequency
	if (setParam.Frequency_f)
	{
		if (!motor.SetFrequency(setParam.Frequency_v))
		{
			assert(("main::SetMotorParameters() Set frequency error", 0));
			return 0;
		}
#ifndef NDEBUG
		Print("main::SetMotorParameters() Frequency %gHz set\n", setParam.Frequency_v);
#endif // NDEBUG
	}
	// Set acceleration time
	if (setParam.AccelerationTime_f)
	{
		if (!motor.SetAccelerationTime(setParam.AccelerationTime_v))
		{
			assert(("main::SetMotorParameters() Set acceleration time error", 0));
			return 0;
		}
#ifndef NDEBUG
		Print("main::SetMotorParameters() Acceleration time %gs set\n", setParam.AccelerationTime_v);
#endif // NDEBUG
	}
	// Set deceleration time
	if (setParam.DecelerationTime_f)
	{
		if (!motor.SetDecelerationTime(setParam.DecelerationTime_v))
		{
			assert(("main::SetMotorParameters() Set deceleration time error", 0));
			return 0;
		}
#ifndef NDEBUG
		Print("main::SetMotorParameters() Deceleration time %gs set\n", setParam.DecelerationTime_v);
#endif // NDEBUG
	}
	// Set any param
	if (setParam.reg_f)
	{
		if (!motor.SetParam(setParam.reg_a, setParam.reg_v))
		{
			assert(("main::SetMotorParameters() Set parameter error", 0));
			return 0;
		}
#ifndef NDEBUG
		Print("main::SetMotorParameters() Parameter 0x%04X is set to 0x%04X\n", setParam.reg_a, setParam.reg_v);
#endif // NDEBUG
	}
	return 1;
}
