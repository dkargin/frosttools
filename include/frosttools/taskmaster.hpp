#ifndef FROSTTOOLS_TASKMASTER_HPP
#define FROSTTOOLS_TASKMASTER_HPP

#include <frosttools/threads.hpp>
#include <queue>

namespace Threading
{
	// Contains thread pool and task queue. Tasks are distributed among free threads.
	// The class itself is not thread safe. Prevent calling TaskMaster methods from tasks to prevent deadlocks
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

		// Does all threading work
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
						//printf("[Worker%d] is dying\n",workerId);					
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
						//printf("[Worker%d] no task - going to sleep\n",workerId);

						master->sleepBarrier.wait(master->taskQueueGuard);
						//printf("[Worker%d] awakened\n",workerId);
					}	
				}
			}
		};
				
		TaskMaster(size_t max)
		{		
			workerData = NULL;
			workerMax = 0;
			workersActive = 0;
			killAll = false;

			resize(max);	
		}

		~TaskMaster()
		{	
			clear();
		}

		void resize(int max)
		{
			if(max != workerMax)
				clear();
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

		/// Stop app tasks and exit
		void clear()
		{
			if(workerData == NULL)
				return;

			taskQueueGuard.lock();
			killAll = true;		
			taskQueueGuard.unlock();

			// notify all workers
			sleepBarrier.notify_all();	

			// wait for all threads to stop
			for(unsigned int i = 0; i < workerMax; i++)
				workerData[i].thread.join();

			delete []workerData;
			workerData = NULL;
		}

		void wait()
		{
			taskQueueGuard.lock();
			while(workersActive > 0 || !tasks.empty())
			{
				taskDone.wait(taskQueueGuard);
			}
			taskQueueGuard.unlock();
		}

		// Add new task to queue. Execution would be started immediately
		void schedule(const Task &task)
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

		WorkerData * workerData;
		size_t workerMax;	

		Threading::mutex taskQueueGuard;
		Threading::ConditionVariable sleepBarrier, taskDone;

		// Tasks active in this moment
		bool killAll;
		size_t workersActive;	// TODO: inc/dec using atomics?
	};
}
#endif