#ifndef _FROSTTOOLS_TIMER_HPP_
#define _FROSTTOOLS_TIMER_HPP_

#ifdef _MSC_VER
#include "win32/time_impl.hpp"
#else
#include "linux/time_impl.hpp"
#endif

#include <cstdlib>
#include <vector>	// for timeline

namespace frosttools
{
//! Manager for timed events
//! Not sure if anyone needs it
class TimeManager
{
public:
	typedef size_t Time;	///< Type to store actual time
	typedef long unsigned int TimeAccumulator;	///< Type to accumulate time

	/// Base class for stored action
	class Action
	{
	public:
		/// Destructor
		virtual ~Action() {}
		/// Check if TimeManager should delete it by itself
		virtual bool IsDestructible() const {return true;}
		/// Execute pending action
		virtual void Execute() = 0;
	};

	/// Ticket for timer
	struct Timer
	{
		Action * action;	//!< Action to run
		Time time;			//!< Time to run
	};

	/// Add action to execute after specified delay
	void add(Action * action, Time delay);
	/// Update timer
	/// It increments time counter and calls pending actions
	virtual void update(Time dt);

	/// Constructor
	TimeManager();
	/// Destructor
	virtual ~TimeManager();

	/// remove all pending actions
	void clear();
	/// reset accumulated time
	void resetTime();
protected:
	/// Accumulated time
	TimeAccumulator totalTime;
	/// Stored actions
	std::vector<Timer> timeLine;

	/// remove all actions
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
