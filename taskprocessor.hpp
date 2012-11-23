#ifndef FROSTTOOLS_TASKPROCESSOR
#define FROSTTOOLS_TASKPROCESSOR
#pragma once
#include "delegate.hpp"
#include <list>
//class TaskProcessor;
enum TaskState
{
	tsWait,
	tsReady,
	tsActive,
	tsDelete
};

template<class _Task>
class _TaskProcessor
{
public:
	typedef _TaskProcessor<_Task> my_type;
	typedef typename _Task::target_type target_type;

	Delegate<void,target_type*> onTaskFinish;
	Delegate<void,target_type*> onTaskUpdate;

	typedef std::list<_Task*> TaskOrder;
	typedef typename TaskOrder::iterator task_position;
protected:
	TaskOrder taskOrder;
public:
	_TaskProcessor()
	{}
	virtual ~_TaskProcessor()
	{
		for(typename std::list<_Task*>::iterator it=taskOrder.begin();it!=taskOrder.end();it++)
			delete *it;
	}
	virtual typename _Task::target_type * getTaskTarget()
	{
		return dynamic_cast<typename _Task::target_type*>(this);
	}
	virtual void processTasks(float ticks)
	{
		while(ticks && !taskOrder.empty())
		{
			_Task *active=taskOrder.front();
			ticks=active->process(getTaskTarget(),ticks);
			if(active->state==tsDelete)
			{
				if(onTaskFinish)
					onTaskFinish(getTaskTarget());
				delete active;
				taskOrder.pop_front();
			}
		}
	}
	void skipTask()
	{
		if( !taskOrder.empty() )
		{
			taskOrder.front()->state=tsDelete;
		}
	}
	task_position addTaskBefore(_Task *task, const task_position &position)
	{
		if(position!=taskOrder.begin())
		{
			task_position it=position;
			return taskOrder.insert(--it,task);
		}
		taskOrder.push_front(task);
		return taskOrder.begin();

	}
	task_position addTaskAfter(_Task *task,const task_position &position)
	{
		return taskOrder.insert(position,task);
	}
	// insert new task after to queue start
	task_position addTaskFront(_Task *task)
	{
		if(!taskOrder.empty())
			taskOrder.front()->state=tsWait;
		taskOrder.push_front(task);
		return taskOrder.begin();
	}
	// insert new task after to queue end
	task_position addTaskBack(_Task *task)
	{
		taskOrder.push_back(task);
		return --taskOrder.end();
	}
	task_position findTask(_Task *task)
	{
		typename TaskOrder::iterator it=taskOrder.begin();
		for(;it!=taskOrder.end();it++)
			if(*it==task)break;
		return it;
	}
	virtual void clearTasks(_Task *last=NULL)
	{
		if(!taskOrder.empty())
		{
			_Task *active=taskOrder.front();

			typename TaskOrder::iterator it=taskOrder.begin();
			while(it!=taskOrder.end())
				if(!last || *it==last)
				{
					delete *it;
					it=taskOrder.erase(it);
				}
				else
					it++;
		}
	}
	virtual void update(float dt)=0;
};

//template<class Target>
//class Task
//{
//public:
//	typedef Task<Target> my_type;
//	typedef Target target_type;
//	typedef _TaskProcessor<Task<Target>> TaskProcessor;
//	typedef typename TaskProcessor::Tasks::iterator task_position;
//	task_position position;	// Task position in TaskProcessor::taskOrder container
//	float ticksLeft;
//	float ticksMax;
//	int state;
//	bool init;
//public:
//
//	Task(float max):
//		ticksMax(max),
//		ticksLeft(max),
//		state(tsWait),
//		init(false)
//	{}
//	virtual ~Task(){};
//
//	void setTime(float time)
//	{
//		ticksMax=time;
//		ticksLeft=time;
//	}
//	float process(Target *unit,float ticks)
//	{
//		if(state==tsWait)
//			state=onInit(unit)?tsReady:tsDelete;
//		if(state==tsReady)
//			state=onBegin(unit)?tsActive:tsDelete;
//
//		if(state==tsActive)
//		{
//			if(ticksLeft<=ticks)
//			{
//				float t=ticksLeft;
//				int res=onProcess(unit,t);
//				assert(t>=0);
//				if(res)
//					t=0;
//				unit->update(ticksLeft);
//				state=onFinish(unit)?tsDelete:tsReady;
//				return t;
//			}
//			else
//			{
//				float t=ticks;
//				int res=onProcess(unit,t);
//				unit->update(ticks);
//				ticksLeft-=(ticks);
//				if(res)
//					t=0;
//				else
//					state=onFinish(unit)?tsDelete:tsReady;
//				return t;
//			}
//		}
//		return ticks;
//	}
//
//	Delegate<int,Target*> eventInit;
//	Delegate<int,Target*> eventFinish;
//	Delegate<int,Target*,float&> eventProcess;
//
//	virtual int onInit(Target *unit)=0;
//	virtual int onFinish(Target *unit)=0;
//	virtual int onBegin(Target *unit)=0;
//	virtual int onProcess(Target *unit,float &ticks)=0;	// return 0 in case of interrupt
//
//};


///////////////////////////////////////////////////////////////////////////
// Delegate based version
///////////////////////////////////////////////////////////////////////////
template<class Target>
class _Task
{
public:
	typedef _Task<Target> my_type;
	typedef Target target_type;
	typedef _TaskProcessor<my_type> parent_type;

	float ticksLeft;
	float ticksMax;
	int state;
	bool init;
	//typedef parent_type::Tasks Tasks;
	//typedef Tasks::iterator task_position;
	//typedef typename Target::task_position task_position;
	//typedef std::list<my_type*>::iterator task_position;
	//std::list<Task<Target>*>::iterator position;	// Task position in TaskProcessor::taskOrder container
public:

	_Task(float max):
		ticksMax(max),
		ticksLeft(max),
		state(tsWait),
		init(false)
	{}
	virtual ~_Task(){};

	void setTime(float time)
	{
		ticksMax=time;
		ticksLeft=time;
	}
	float process(Target *unit,float ticks)
	{
		if(state==tsWait)
			state=eventInit?(eventInit(unit)?tsReady:tsDelete):tsReady;
		if(state==tsReady)
			state=eventBegin?(eventBegin(unit)?tsActive:tsDelete):tsReady;

		if(state==tsActive)
		{
			if(ticksLeft<=ticks)
			{
				float t=ticksLeft;
				int res=eventProcess?eventProcess(unit,t):1;
				assert(t>=0);
				if(res)
					t=0;
				unit->update(ticksLeft);
				state=eventFinish?(eventFinish(unit)?tsDelete:tsReady):tsDelete;
				return t;
			}
			else
			{
				float t=ticks;
				int res=eventProcess?eventProcess(unit,t):1;
				unit->update(ticks);
				ticksLeft-=(ticks);
				if(res)
					t=0;
				else
					state=eventFinish?(eventFinish(unit)?tsDelete:tsReady):tsDelete;
				return t;
			}
		}
		return ticks;
	}

	typedef Delegate<int,Target*> EventInit;
	typedef Delegate<int,Target*> EventFinish;
	typedef Delegate<int,Target*> EventBegin;
	typedef Delegate<int,Target*,float&> EventProcess;

	EventInit eventInit;
	EventFinish eventFinish;
	EventBegin eventBegin;
	EventProcess eventProcess;
};
#endif