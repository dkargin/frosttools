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

namespace frosttools
{
namespace threading
{
	/// Thread
	/**
	 * Wraps pthread thread class
	 */
	class Thread
	{
		pthread_t id;
		pthread_attr_t attr;
		
	public:
		Thread()
		{
			id = 0;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
		}

		template<class Function> Thread(Function fn)
		{
			id = 0;
			run(fn);
		}

		/// wait until thread is complete
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

	/// sleep for millisecond time
	inline void sleep(int msec)
	{
		usleep(msec*1000);
	}

	/// pthread mutex wrapper	
	class Mutex : public BaseLockable
	{
	public:
		pthread_mutex_t mutex;
		pthread_mutexattr_t attr;

		Mutex()
		{
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&mutex, &attr);
			//pthread_mutex_init(&mutex, NULL);
		}

		~Mutex()
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

		bool lock()
		{
			int result = pthread_mutex_lock (&mutex);
			if(result < 0)
			{
				reportError("Mutex::lock()");
				return false;
			}
			return true;
		}

		bool unlock()
		{
			int result = pthread_mutex_unlock (&mutex);
			if(result < 0)
			{
				reportError("Mutex::lock()");
				return false;
			}
			return true;
		}
	private:
		void reportError(const char * where)
		{
			int err = errno;
			fprintf(stderr, "%s: error errno=%d, %s", where, err, strerror(err));
		}
	};

	/// ConditionVariable
	/**
	 * Wrapper around pthreads condition variable construct. Mimics std::condition_variable
	 */
	class ConditionVariable
	{
		pthread_cond_t cv;
	public:
		//! Constructor
		ConditionVariable()
		{
			pthread_cond_init(&cv, NULL);
		}
		//! Destructor
		~ConditionVariable()
		{
			pthread_cond_destroy(&cv);
		}

		//! Wait until notification
		void wait(Mutex &mutex)
		{
			int result = pthread_cond_wait(&cv, &mutex.mutex);
			if(result != 0)
				reportError(result);
		}

		//! Wait until notification or timeout
		/*! Wait for specified time, in milliseconds.
		 * Returns cv status after completion
		*/
		CvStatus waitFor(Mutex & mutex, int timeMS)
		{
			if(timeMS <= 0)
				return cvError;

			const int nsecInSec = 1000000000;
			const int msecInSec = 1000;
			const int nsecInMsec = 1000000;
			timespec current;
			clock_gettime(CLOCK_REALTIME, &current);
			timespec time;
			long nsec = current.tv_nsec + (timeMS % msecInSec) * nsecInMsec;
			time.tv_sec = current.tv_sec + timeMS / msecInSec + nsec / nsecInSec;
			time.tv_nsec = nsec % nsecInSec;
			int result = pthread_cond_timedwait(&cv, &mutex.mutex, &time);
			if( result != 0 && result != ETIMEDOUT)
			{
				reportError(result);
				return cvError;
			}
			if(result == ETIMEDOUT)
				return cvTimeout;
			return cvNoTimeout;
		}

		//! Signal one thread waiting for this CV
		void notifyOne()
		{
			int result = pthread_cond_signal(&cv);
			if( result != 0)
				reportError(result);
		}
		//! Signal every thread waiting for this CV
		void notifyAll()
		{
			int result = pthread_cond_broadcast(&cv);
			if( result != 0)
				reportError(result);
		}
private:
		//! Used as error reporting
		static void reportError(int errcode)
		{
			printf("Error %d:%d in CV: %s\n", errcode, errno, strerror(errno));
		}
	};
};

} // namespace frosttools
#endif
