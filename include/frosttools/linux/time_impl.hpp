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
