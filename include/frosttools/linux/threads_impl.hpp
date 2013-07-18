#ifndef _FT_THREADS_IMPL_HPP_
#define _FT_THREADS_IMPL_HPP_

#include <unistd.h>
#include <pthread.h>
#include <assert.h>

// for error reporting
#include <string.h>
#include <stdio.h>
#include <errno.h>
/*
 * PThreads based implementation
 */

namespace Threading
{
	class thread
	{
		pthread_t id;
		pthread_attr_t attr;
		
	public:
		thread()
		{
			id = 0;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
		}

		template<class Function> thread(Function fn)
		{
			id = 0;
			run(fn);
		}

		void join()
		{
			void * result = NULL;
			pthread_join(id, &result);
		}

		typedef void *(*thread_fn) (void *);

		template<class Function> void run(Function & fn)
		{
			assert(id == 0);
			typedef FunctionWrapper<Function> wrapper;
			wrapper * w = wrapper::create(fn);
			pthread_create(&id, &attr, wrapper::run, w);
		}

		template<class Arg> void run(void (*func)(Arg ), Arg arg)
		{
			typedef _thread_helper::FnWrapper1<Arg> wrapper;
			wrapper * w = wrapper::create(func, arg);
			pthread_create(&id, &attr, wrapper::run, w);
		}
	protected:

	};

	inline void sleep(int msec)
	{
		usleep(msec*1000);
	}
	/// pthread mutex wrapper	
	class MutexPT : public BaseLockable
	{
	public:
		pthread_mutex_t mutex;

		MutexPT()
		{
			pthread_mutex_init(&mutex, NULL);
		}

		~MutexPT()
		{
			pthread_mutex_destroy(&mutex);
		}

		bool trylock()
		{
			int result = pthread_mutex_trylock(&mutex);
			return result == 0;
		}

		bool locked() const
		{
			pthread_mutex_t* m = (pthread_mutex_t*)&mutex;
			if(pthread_mutex_trylock((pthread_mutex_t*)&m))
			{
				pthread_mutex_unlock((pthread_mutex_t*)&m);
				return true;
			}
			return false;
		}

		void reportError(const char * where)
		{
			int err = errno;
			fprintf(stderr, "%s: error errno=%d, %s", where, err, strerror(err));
		}

		void lock()
		{
			int result = pthread_mutex_lock (&mutex);
			if(result < 0)
				reportError("Mutex::lock()");
		}

		void unlock()
		{
			int result = pthread_mutex_unlock (&mutex);
			if(result < 0)
				reportError("Mutex::lock()");
		}
	};


	typedef MutexPT Mutex;

	class ConditionVariable
	{
		pthread_cond_t cv;
	public:
		ConditionVariable()
		{
			pthread_cond_init(&cv, NULL);
		}

		~ConditionVariable()
		{
			pthread_cond_destroy(&cv);
		}

		void wait(Mutex &mutex)
		{
			pthread_cond_wait(&cv, &mutex.mutex);
		}

		int wait_for(Mutex & mutex, int timeMS)
		{
			timespec time;
			time.tv_sec = timeMS / 1000;
			time.tv_nsec = timeMS / 1000000;
			return pthread_cond_timedwait(&cv, &mutex.mutex, &time);
		}

		void notify_one()
		{
			pthread_cond_signal(&cv);
		}

		void notify_all()
		{
			pthread_cond_broadcast(&cv);
		}
	};
};
#endif
