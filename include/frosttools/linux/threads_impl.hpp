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

		template<class Holder> thread(Holder * holder, void (Holder::*method)(void))
		{
			id = 0;
			MethodHolder<Holder> h;
			h.holder = holder;
			h.method = method;
			assert(id == 0);
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
			pthread_create(&id, &attr, &MethodHolder<Holder>::run, &h);
		}

		template<class Function> thread(Function fn)
		{
			id = 0;
			run_fn(fn);
		}


		void join()
		{
			void * result = NULL;
			pthread_join(id, &result);
		}
	protected:
		template<class Holder> struct MethodHolder
		{
			Holder * holder;
			void (Holder::*method)(void);
			MethodHolder()
			{
				holder = NULL;
				method = NULL;
			}
			MethodHolder(const MethodHolder &mh)
			:holder(mh.holder), method(mh.method)
			{
			}

			void operator()()
			{
				(holder->*method)();
			}

			static void * run(void*data)
			{
				MethodHolder * mh = (MethodHolder*)data;
				(*mh)();
				pthread_exit(NULL);
				return NULL;
			}
		};
		template<class Function> void run_fn(Function fn)
		{
			assert(id == 0);
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
			pthread_create(&id, &attr, (void*(*)(void*))&(s_run_fn<Function>), &fn);
		}
		template<class Function> static void * s_run_fn(Function * fn)
		{
			(*fn)();
			pthread_exit(NULL);
			return NULL;
		}
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
