#ifndef FROSTTOOLS_TASKMASTER_HPP
#define FROSTTOOLS_TASKMASTER_HPP

#include <frosttools/threads.hpp>
#include <queue>

template<class Task>
class TaskMaster
{
public:	
	std::queue<Task> tasks;

	enum State
	{		
		Run,
		WaitDeep,		/// use condition to wait
		WaitActive,		/// actively check for new tasks
		Die
	};

	struct WorkerData
	{
		TaskMaster * master;
		size_t i;
		Threading::thread thread;

		// Get next task from queue and process it
		bool processTask()
		{
			
			Task task;
			TaskMaster::State state = master->getTask(task);

			if(state == TaskMaster::Die)
				return false;
			else if(master->getTask(task) == Run)
			{
				task();
				master->taskDone.notify_one();
			}
			return true;
		}

		static void workerBase(WorkerData * desc)
		{
			TaskMaster * master = desc->master;
			int workerId = desc - master->workerData;			

			while(1)
			{
				Task task;

				Threading::ScopedLock<Threading::mutex> lock(master->taskQueueGuard);
				
				TaskMaster::State state = master->getTask(task);

				if(state == TaskMaster::Die)
				{
					printf("[Worker%d] is dying\n",workerId);					
					break;
				}
				else if(state == Run)
				{
					master->workersActive++;
					master->taskQueueGuard.unlock();
					task();
					master->taskQueueGuard.lock();
					master->workersActive--;
					master->taskDone.notify_one();
				}
				else if(state == TaskMaster::WaitDeep)
				{
					printf("[Worker%d] no task - going to sleep\n",workerId);
					
					master->sleepBarrier.wait(master->taskQueueGuard);
					printf("[Worker%d] awakened\n",workerId);
				}	
			}
		}
	};

	WorkerData * workerData;
	size_t workerMax;
	bool killAll;

	Threading::mutex taskQueueGuard;
	Threading::ConditionVariable sleepBarrier, taskDone;

	size_t workersActive;

	/// Wrapper around process
	
	TaskMaster(size_t max)
	{			
		workerData = new WorkerData[max];
		workerMax = max;
		workersActive = 0;
		killAll = false;		
	
		for(size_t i = 0; i < workerMax; i++)
		{
			workerData[i].master = this;
			workerData[i].i = i;
			
			workerData[i].thread.run(&WorkerData::workerBase, workerData+i);
		}
	}

	~TaskMaster()
	{	
		taskQueueGuard.lock();
		killAll = true;		
		taskQueueGuard.unlock();

		// notify all workers
		sleepBarrier.notify_all();	

		for(unsigned int i = 0; i < workerMax; i++)
			workerData[i].thread.join();
		delete []workerData;
	}

	void waitUntilDone()
	{
		taskQueueGuard.lock();
		while(workersActive > 0)
		{
			taskDone.wait(taskQueueGuard);
		}
		taskQueueGuard.unlock();
	}

	void addTask(const Task &task)
	{
		taskQueueGuard.lock();		
		tasks.push(task);
		sleepBarrier.notify_one();
		taskQueueGuard.unlock();		
	}
private:	
	// Not threading safe call
	State getTask(Task &result)
	{
		//taskQueueGuard.lock();	
		
		if(killAll)
			return Die;
		if(tasks.size() > 0)
		{
			result = tasks.front();
			tasks.pop();
			return Run;
		}
		
		return WaitDeep;		
	}
};

#endif