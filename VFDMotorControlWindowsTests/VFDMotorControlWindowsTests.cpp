#include "pch.h"
#include "CppUnitTest.h"

#include "../VFDMotorControlWindows/VFD.h"
#include "../VFDMotorControlWindows/ModbusRTUclient.h"
#include "../VFDMotorControlWindows/COMPort.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace std;

namespace VFDMotorControlWindowsTests
{

	TEST_CLASS(COMPortTests)
	{
	public:

		TEST_METHOD(DefaultConstructorTests)
		{
			//COMPort p1(5);
			//string s = "ѕроверка работы тестов и получени€ данных: " + to_string(p1.get_x()) + "\n";
			//Logger::WriteMessage(s.c_str());

			//Assert::IsTrue(p1.get_x() == 0, L"Assert bad x");
		}
	};

	TEST_CLASS(ModbusRTUclientTests)
	{
	public:

		TEST_METHOD(DefaultConstructorTests)
		{

		}
	};

	TEST_CLASS(VFDTests)
	{
	public:

		TEST_METHOD(DefaultConstructorTests)
		{

		}
	};
}
