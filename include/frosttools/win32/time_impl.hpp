#ifndef _FT_TIME_IMPL_HPP_
#define _FT_TIME_IMPL_HPP_

#include <windows.h>

class TimerAccurate
{
	LARGE_INTEGER freq, start, end;
public:
	TimerAccurate()
	{
		QueryPerformanceFrequency(&freq);
	}
	void Start()
	{
		QueryPerformanceCounter(&start);
	}
	void Stop()
	{
		QueryPerformanceCounter(&end);
	}

	double Interval() const
	{
		return double(end.QuadPart - start.QuadPart)/freq.QuadPart;
	}
};

class Timer
{
public:
	double interval;
	int ticks;
	void Start()
	{
		interval = 0;
		ticks = GetTickCount();
	}
	void Stop()
	{
		interval = (GetTickCount() - ticks)*0.001f;
	}
	double Interval() const
	{
		return interval;
	}
};
#endif
