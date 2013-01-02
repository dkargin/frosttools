#ifndef _FT_TIME_IMPL_HPP_
#define _FT_TIME_IMPL_HPP_

#include <sys/time.h>

class Timer
{
	static unsigned long GetTickCount()
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return (tv.tv_sec*1000+tv.tv_usec/1000);
	}
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
