#ifndef _FROSTTOOLS_TIMER_HPP_
#define _FROSTTOOLS_TIMER_HPP_

#ifdef TIMER_ACCURATE
class Timer
{
	LARGE_INTEGER freq, start, end;
public:
	Timer()
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
#else
class Timer
{
public:
	double interval;	
	int ticks;
	void Start()
	{
		interval = 0;
		ticks = 0;//GetTickCount();
	}
	void Stop()
	{
		interval = 0.1;//(GetTickCount() - ticks)*0.001f;
	}	
	double Interval() const
	{
		return interval;
	}
};
#endif

#endif
