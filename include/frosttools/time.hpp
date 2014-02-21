#ifndef _FROSTTOOLS_TIMER_HPP_
#define _FROSTTOOLS_TIMER_HPP_

#ifdef _MSC_VER
#include "win32/time_impl.hpp"
#else
#include "linux/time_impl.hpp"
#endif

#include <vector>	// for timeline

namespace frosttools
{
//! Manager for timed events
//! Not sure if anyone needs it
class TimeManager
{
public:
	typedef size_t Time;
	typedef long unsigned int TimeAccumulator;

	class Action
	{
	public:
		virtual ~Action() {}
		virtual bool IsDestructible() const {return true;}
		virtual void Execute() = 0;
	};

	struct Timer
	{
		Action * action;
		Time time;
	};

	void add(Action * action, Time delay);
	virtual void update(Time dt);

	TimeManager();
	~TimeManager();

	void clear();					// clear timeline
	void resetTime();				// reset accumulated time
protected:
	TimeAccumulator totalTime;
	std::vector<Timer> timeLine;

	void destroy(Timer & timer);	// destroy specific timer
};

#ifndef FrostTools_Impl
#define FrostTools_Impl inline
#endif

#define TimeManager_impl FrostTools_Impl

TimeManager_impl TimeManager::TimeManager()
	:totalTime(0)
{}

TimeManager_impl TimeManager::~TimeManager()
{
	clear();
}
// destroy specific timer
TimeManager_impl void TimeManager::destroy(Timer & timer)
{
	if(timer.action != NULL && timer.action->IsDestructible())
	{
		delete timer.action;
	}
	timer.action = NULL;
	timer.time = 0;
}
// reset accumulated time
TimeManager_impl void TimeManager::resetTime()
{
	for( size_t i = 0; i < timeLine.size(); ++i)
	{
		Timer & timer = timeLine[i];
		timer.time -= totalTime;
	}
	totalTime = 0;
}
// clear timeline
TimeManager_impl void TimeManager::clear()
{
	for( size_t i = 0; i < timeLine.size(); ++i)
	{
		Timer & timer = timeLine[i];
		destroy(timer);
	}
	timeLine.clear();
}

// add new events
TimeManager_impl void TimeManager::add(TimeManager::Action * action, TimeManager::Time delay)
{
	Timer timer = {action, delay + totalTime};
	timeLine.push_back(timer);

	struct Helper
	{
		static int Comparator(const void * pa, const void *pb)
		{
			return ((Timer*)pa)->time < ((Timer*)pb)->time;
		}
	};
	::qsort(&timeLine.front(), timeLine.size(), sizeof(Timer), &Helper::Comparator);
}
// update timeline
TimeManager_impl void TimeManager::update(TimeManager::Time dt)
{
	// do not accumulate time if timeline is empty
	if( timeLine.empty() )
		return;

	totalTime += dt;
	size_t newSize = timeLine.size();
	Timer * start = &timeLine.front();
	// check timeline events
	for(Timer * current = &timeLine.back();	; --current)
	{
		// if event is occured
		if( current->time <= totalTime )
		{
			current->action->Execute();
			newSize--;
			destroy(*current);
		}
		else
			break;
		// have iterated through all events
		if( current == start )
			break;
	}
	timeLine.resize(newSize);
}
}
#endif
