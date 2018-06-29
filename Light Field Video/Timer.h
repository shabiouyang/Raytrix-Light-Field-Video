
#pragma once
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <time.h>

using namespace std;

enum timeUnit { sec, millisec, microsec };

class Timer
{
private:
	timeUnit unit;
	bool isShow;
	LARGE_INTEGER freq;
	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;

public:
	Timer(timeUnit _unit = sec, bool _isShow = true);
	~Timer();

	double getTime();
};

