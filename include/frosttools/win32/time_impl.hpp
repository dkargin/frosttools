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
	int t_start, t_current;
public:
	Timer()
	{
		t_start = 0;
		t_current = 0;
	}
	void start()
	{
		t_start = GetTickCount();
		check();
	}

	void check()
	{
		t_current = GetTickCount();
	}

	void stop()
	{
		check();
		t_start = t_current;
	}

	double lastTime() const
	{
		return (t_current - t_start)*0.001f;
	}

	double currentTime()
	{
		check();
		return lastTime();
	}
};
#endif
