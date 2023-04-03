/**
 * @file main.cpp
 * @author TAN4UK (tan4ukmak7@gmail.com)
 * @brief VFDMotorControl main file
 *

 // TODO сохранение прошлого параметра порта

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

#include "main.h"

// Global variables ///////////////////////////////////////////////////////////

// CLI keys flags and values
struct {
	bool help;
	bool file;
	bool get;
	bool set;
	bool run;
	bool stop;
} CMD;
char portName[9] = "COM3";			// port name from command line
char* diagramFileName = nullptr;	// file name with diagram
// Get parameters flags
struct {
	bool FrequencyCommand;
	bool OutFrequency;
	bool OutCurrent;
	bool DCVoltage;
	bool OutVoltage;
	bool PowerFactor;
	bool OutTorque;
	bool MotorSpeed;
	bool OutPower;
	bool VFDTemperature;
	bool reg;
} getParam;
unsigned short getReg_a;			// stores register address that has to be read
unsigned short getReg_v;			// stores register value from VFD
VFD_status_t motorStatus = { 0 };	// Stores motor status
VFD_param_t motorParams = { 0 };	// Stores motor parameters
double OutPower;					// Stores power provided to motor
double VFDtemperature;				// Stores temperature of VFD heatsink
// Set parameters flags and values
struct {
	bool Frequency_f;				// Set Frequency flag
	double Frequency_v;				// Set Frequency value
	bool AccelerationTime_f;		// Set AccelerationTime flag
	double AccelerationTime_v;		// Set AccelerationTime value
	bool DecelerationTime_f;		// Set DecelerationTime flag
	double DecelerationTime_v;		// Set DecelerationTime value
	bool reg_f;						// Set register flag
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
void PrintHelp();

/**
 * @brief Set the Motor Parameters specified by user
 *
 * @param motor[in]	- reference to VFD class instance
 * @return true		- if all motor parameters have set successfully
 * @return false	- if some error occured
 */
bool SetMotorParameters(VFD& motor);

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
	//	printf("Demo version expired! Contact with tan4ukmak7@gmail.com to get full version.\n");
	//	return -1;
	//}

	// CLI arguments handle ///////////////////////////////////////////////////
	HandleCLIArguments(argc, argv);
	if (argc <= 1) // if run without arguments
	{
		printf("This program requires input arguments.");
		PrintHelp();
		return 0; // after help print program doesn't accept any commands
	}
	// Print help text ////////////////////////////////////////////////////////
	if (CMD.help) PrintHelp();

	VFD motor({ 1, { portName, 9600, 8, 'E', 1 } }); // VFD class instance

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
		clock_t start_time = clock();
#endif // NDEBUG
		if (!motor.Run(runMode))
		{
			assert(("main: Run error", 0));
			return -1;
		}
#ifndef NDEBUG
		printf("main: Motor started in %ldms\n", (clock() - start_time));
#endif // NDEBUG
	}
	// Stop handling //////////////////////////////////////////////////////////
	if (CMD.stop)
	{
#ifndef NDEBUG
		clock_t start_time = clock();
#endif // NDEBUG
		if (!motor.Stop())
		{
			assert(("main: Stop error", 0));
			return -1;
		}
#ifndef NDEBUG
		printf("main: Motor stopped in %ldms\n", (clock() - start_time));
#endif // NDEBUG
	}
	return 0;
}

void HandleCLIArguments(int argc, char* argv[])
{
	// Handle arguments and find matches
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == nullptr) continue;
#ifndef NDEBUG
		printf("main::HandleCLIArguments() Arg %d: %s\n", i, argv[i]);
#endif // NDEBUG
		// Handle --help argument
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h") || !strcmp(argv[i], "/?"))
		{
			CMD.help = true;
		}
		// Handle --port argument
		else if (!strcmp(argv[i], "--port"))
		{
			if (argv[i + 1] != nullptr)
			{
				size_t len;
				if (strlen(argv[i + 1]) < 9) len = strlen(argv[i + 1]);
				else len = 8;
				// Copy port name
				strcpy_s(portName, len + 1, argv[i + 1]);
			}
		}
		// Handle --file argument
		else if (!strcmp(argv[i], "--file"))
		{

			if (argv[i + 1] != nullptr)
			{
				CMD.file = true;
				diagramFileName = argv[i + 1];
			}
		}
		// Handle --get argument
		else if (!strcmp(argv[i], "--get"))
		{
			CMD.get = true;
			if (argv[i + 1] != nullptr)
			{
				if (!strcmp(argv[i + 1], "FrequencyCommand"))
					getParam.FrequencyCommand = true;
				else if (!strcmp(argv[i + 1], "OutFrequency"))
					getParam.OutFrequency = true;
				else if (!strcmp(argv[i + 1], "OutCurrent"))
					getParam.OutCurrent = true;
				else if (!strcmp(argv[i + 1], "DCVoltage"))
					getParam.DCVoltage = true;
				else if (!strcmp(argv[i + 1], "OutVoltage"))
					getParam.OutVoltage = true;
				else if (!strcmp(argv[i + 1], "PowerFactor"))
					getParam.PowerFactor = true;
				else if (!strcmp(argv[i + 1], "OutTorque"))
					getParam.OutTorque = true;
				else if (!strcmp(argv[i + 1], "MotorSpeed"))
					getParam.MotorSpeed = true;
				else if (!strcmp(argv[i + 1], "OutPower"))
					getParam.OutPower = true;
				else if (!strcmp(argv[i + 1], "VFDTemperature"))
					getParam.VFDTemperature = true;
				else
				{
					getParam.reg = true;
					if (argv[i + 1] != nullptr) // read register addres that has to be read
						sscanf_s(argv[i + 1], "%hX", &getReg_a);
				}
			}
		}
		// Handle --set argument
		else if (!strcmp(argv[i], "--set"))
		{
			CMD.set = true;
			if (argv[i + 1] != nullptr)
			{
				if (!strcmp(argv[i + 1], "Frequency"))
				{
					setParam.Frequency_f = true;
					if (argv[i + 2] != nullptr) setParam.Frequency_v = atof(argv[i + 2]);
				}
				if (!strcmp(argv[i + 1], "AccelerationTime"))
				{
					setParam.AccelerationTime_f = true;
					if (argv[i + 2] != nullptr) setParam.AccelerationTime_v = atof(argv[i + 2]);
				}
				if (!strcmp(argv[i + 1], "DecelerationTime"))
				{
					setParam.DecelerationTime_f = true;
					if (argv[i + 2] != nullptr) setParam.DecelerationTime_v = atof(argv[i + 2]);
				}
				else
				{
					setParam.reg_f = true;
					//if (argv[i + 2] != nullptr) setParam.reg_v = atof(argv[i + 2]);
					if (argv[i + 1] != nullptr) // read register address that has to be set
					{
						sscanf_s(argv[i + 1], "%hX", &(setParam.reg_a));
						if (argv[i + 2] != nullptr) // read register value that has to be set
							sscanf_s(argv[i + 2], "%hX", &(setParam.reg_v));
					}
				}
			}
		}
		// Handle --run argument
		else if (!strcmp(argv[i], "--run"))
		{
			CMD.run = true;
			if (argv[i + 1] != nullptr)
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
			CMD.stop = true;
		}
	}
	return;
}

void PrintHelp()
{
	printf("\nHelp.\n");
	printf("This program can control Delta VFD-B.\n");
	printf("Input commandline arguments:\n");
	printf("-h | --help\t\t\tDisplay this help message\n");
	printf("--port <COMx>\t\t\tSpecify serial port (COM3 default) (--port COM3)\n");
	printf("--file <text_file>\t\tRead a file with frequency and time parameters table. (--file coords.txt)\n");
	printf("\t\t\t\tAnd run motor according to the table.\n");
	printf("Text file should contain table with times and frequencies and should look like this:\n");
	printf("Time\tFrequency\n");
	printf("0\t0\n");
	printf("10\t30\n");
	printf("20\t20\n");
	printf("40\t-10\n");
	printf("50\t0\n");
	printf("(The first column specifies the time to be set.\n");
	printf("The second column specifies the frequency that will be reached for the time,\n");
	printf("specified in the first column.)\n");
	printf("--get <parameter>\t\tRead one of the following motor parameters: (--get MotorSpeed)\n");
	printf("\t\t\t\t<FrequencyCommand>\n");
	printf("\t\t\t\t<OutFrequency>\n");
	printf("\t\t\t\t<OutCurrent>\n");
	printf("\t\t\t\t<DCVoltage>\n");
	printf("\t\t\t\t<OutVoltage>\n");
	printf("\t\t\t\t<PowerFactor>\n");
	printf("\t\t\t\t<OutTorque>\n");
	printf("\t\t\t\t<MotorSpeed>\n");
	printf("\t\t\t\t<OutPower>\n");
	printf("\t\t\t\t<VFDTemperature>\n");
	printf("\t\t\t\t<0x(parameter address)> (--get 0x0109)\n");
	printf("--set <parameter> <value>\tSet one of the folloving motor parameters: (--set Frequency 45.5)\n");
	printf("\t\t\t\t<Frequency>\n");
	printf("\t\t\t\t<AccelerationTime>\n");
	printf("\t\t\t\t<DecelerationTime>\n");
	printf("\t\t\t\t<0x(parameter address)> (--set 0x2001 0x1388)\n");
	printf("--run <n|f|r|c>\t\t\tRun motor with direction set (no change, forvard, reverse, change) (--run r)\n");
	printf("--stop\t\t\t\tStop motor\n\n");
}

bool GetMotorParameters(VFD& motor)
{
#ifndef NDEBUG
	clock_t start_time = clock();
#endif // NDEBUG
	// Read all parameters and status
	//if (getParam.FrequencyCommand || getParam.OutFrequency ||
	//	getParam.OutCurrent || getParam.DCVoltage ||
	//	getParam.OutVoltage || getParam.PowerFactor ||
	//	getParam.OutTorque || getParam.MotorSpeed)
	//{
	if (!motor.ReadParameterRegisters(&motorStatus, &motorParams))
	{
		assert(("main::GetMotorParameters() Read parameters error", 0));
		return false;
	}
	//}
	// Read OutPower parameter
	if (getParam.OutPower)
	{
		if (!motor.GetOutPower(&OutPower))
		{
			assert(("main::GetMotorParameters() Read power error", 0));
			return false;
		}
	}
	// Read VFDTemperature parameter
	if (getParam.VFDTemperature)
	{
		if (!motor.GetVFDTemperature(&VFDtemperature))
		{
			assert(("main::GetMotorParameters() Read temperature error", 0));
			return false;
		}
	}
	if (getParam.reg)
	{
		if (!motor.GetParam(getReg_a, &getReg_v))
		{
			assert(("main::GetMotorParameters() Get parameter error", 0));
			return false;
		}
	}
#ifndef NDEBUG
	printf("main::GetMotorParameters() Read param time: %ld\n", clock() - start_time);
#endif // NDEBUG
	return true;
}

void PrintParametersHeader(bool Time /* = false */, FILE* printStream /* = stdout */)
{
	// Print parameters header for parameters given in --get argument
	bool firstTime = true; // this flag is for avoid print unnecessary tabs
	if (Time)
	{
		firstTime = false;
		fprintf(printStream, "Time");
	}
	if (getParam.FrequencyCommand)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "FrequencyCommand");
	}
	if (getParam.OutFrequency)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "OutFrequency");
	}
	if (getParam.OutCurrent)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "OutCurrent");
	}
	if (getParam.DCVoltage)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "DCVoltage");
	}
	if (getParam.OutVoltage)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "OutVoltage");
	}
	if (getParam.PowerFactor)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "PowerFactor");
	}
	if (getParam.OutTorque)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "OutTorque");
	}
	if (getParam.MotorSpeed)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "MotorSpeed");
	}
	if (getParam.OutPower)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "OutPower");
	}
	if (getParam.VFDTemperature)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "VFDTemperature");
	}
	if (getParam.reg)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "Param0x%04X", getReg_a);
	}
	fprintf(printStream, "\n");
}

void PrintParameters(double Time /* = -1 */, FILE* printStream /* = stdout */)
{
	// Print parameters given in --get argument
	bool firstTime = true; // this flag is for avoid print unnecessary tabs
	if (Time > -0.5)
	{
		firstTime = false;
		fprintf(printStream, "%.2f", Time);
	}
	if (getParam.FrequencyCommand)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", motorParams.FrequencyCommand);
	}
	if (getParam.OutFrequency)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", motorParams.OutFrequency);
	}
	if (getParam.OutCurrent)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", motorParams.OutCurrent);
	}
	if (getParam.DCVoltage)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", motorParams.DCVoltage);
	}
	if (getParam.OutVoltage)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", motorParams.OutVoltage);
	}
	if (getParam.PowerFactor)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", motorParams.PowerFactor);
	}
	if (getParam.OutTorque)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", motorParams.OutTorque);
	}
	if (getParam.MotorSpeed)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", motorParams.MotorSpeed);
	}
	if (getParam.OutPower)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", OutPower);
	}
	if (getParam.VFDTemperature)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "%g", VFDtemperature);
	}

	if (getParam.reg)
	{
		if (firstTime) firstTime = false;
		else fprintf(printStream, "\t");
		fprintf(printStream, "0x%04X", getReg_v);
	}
	fprintf(printStream, "\n");
}

bool SetMotorParameters(VFD& motor)
{
	// Set frequency
	if (setParam.Frequency_f)
	{
		if (!motor.SetFrequency(setParam.Frequency_v))
		{
			assert(("main::SetMotorParameters() Set frequency error", 0));
			return false;
		}
#ifndef NDEBUG
		printf("main::SetMotorParameters() Frequency %gHz set\n", setParam.Frequency_v);
#endif // NDEBUG
	}
	// Set acceleration time
	if (setParam.AccelerationTime_f)
	{
		if (!motor.SetAccelerationTime(setParam.AccelerationTime_v))
		{
			assert(("main::SetMotorParameters() Set acceleration time error", 0));
			return false;
		}
#ifndef NDEBUG
		printf("main::SetMotorParameters() Acceleration time %gs set\n", setParam.AccelerationTime_v);
#endif // NDEBUG
	}
	// Set deceleration time
	if (setParam.DecelerationTime_f)
	{
		if (!motor.SetDecelerationTime(setParam.DecelerationTime_v))
		{
			assert(("main::SetMotorParameters() Set deceleration time error", 0));
			return false;
		}
#ifndef NDEBUG
		printf("main::SetMotorParameters() Deceleration time %gs set\n", setParam.DecelerationTime_v);
#endif // NDEBUG
	}
	// Set any param
	if (setParam.reg_f)
	{
		if (!motor.SetParam(setParam.reg_a, setParam.reg_v))
		{
			assert(("main::SetMotorParameters() Set parameter error", 0));
			return false;
		}
#ifndef NDEBUG
		printf("main::SetMotorParameters() Parameter 0x%04X is set to 0x%04X\n", setParam.reg_a, setParam.reg_v);
#endif // NDEBUG
	}
	return true;
}
