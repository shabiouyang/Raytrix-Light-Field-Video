#include "Timer.h"

//Constructor is Timer Start
Timer::Timer(timeUnit _unit, bool _isShow) : unit(_unit), isShow(_isShow)
{
	if (QueryPerformanceFrequency(&freq))
	{
		QueryPerformanceCounter(&startTime);
	}
	else
	{
		cout << "Timer::Timer() Error" << endl;
	}
}

//Destructor is Timer End 
Timer::~Timer()
{
	if (QueryPerformanceCounter(&endTime))
	{
		if (isShow)
		{
			printf("%.2f", (double)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * pow(1000, (int)unit));
			switch (unit)
			{
			case microsec:
				cout << " [us]" << endl;
				break;
			case millisec:
				cout << " [ms]" << endl;
				break;
			case sec:
				cout << " [s]" << endl;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		cout << "Timer::Timer() Error" << endl;
	}
}

double Timer::getTime()
{
	if (QueryPerformanceCounter(&endTime))
	{
		return (double)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart * pow(1000, (int)unit);
	}

	return 0.0;
}

