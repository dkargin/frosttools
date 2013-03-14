#ifndef _FT_THREADS_IMPL_HPP_
#define _FT_THREADS_IMPL_HPP_

#include <unistd.h>
#include <pthread.h>
#include <assert.h>
/*
 * PThreads based implementation
 */

namespace Threading
{
	class thread
	{
		pthread_t id;
	public:
		thread()
		{
			id = 0;
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

		template<class Function> void run(Function fn)
		{
			assert(id == 0);
			pthread_create(&id, NULL, (thread_fn)&functor_runner<Function>, &fn);
		}

		template<class Arg> void run(void (*func)(Arg ), Arg arg)
		{
			typedef _thread_helper::FnWrapper1<Arg> wrapper;
			wrapper * w = wrapper::create(func, arg);
			pthread_create(&id, NULL, wrapper::run, w);
		}
	protected:

	};

	inline void sleep(int msec)
	{
		usleep(msec*1000);
	}
	/// pthread mutex wrapper	
	class MutexPT
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

		void lock()
		{
			pthread_mutex_lock (&mutex);
		}

		void unlock()
		{
			pthread_mutex_unlock (&mutex);
		}
	};

	typedef MutexPT Mutex;
};
#endif
