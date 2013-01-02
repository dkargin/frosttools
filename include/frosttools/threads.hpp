#ifndef _FT_THREADS_HPP_
#define _FT_THREADS_HPP_


#ifdef WIN32
#include "win32/threads_impl.hpp"
#else
#include "linux/threads_impl.hpp"
#endif

namespace Threading
{/*
	class Thread
	{
	public:
		static void sleep(int msec)
		{
#ifdef _MSC_VER
			Sleep(msec);
#else
			usleep(msec*1000);
#endif
		}
	};*/
	
	template<class Lockable>
	class ScopedLock
	{
	public:
		Lockable & mutex;
		ScopedLock(Lockable &mutex)
			:mutex(mutex)
		{
			mutex.lock();
		}
		~ScopedLock()
		{
			mutex.unlock();
		}
	};

#ifdef WIN32
	class LockableCritSection
	{
		CRITICAL_SECTION cs;
	public:

		friend class ScopedLock<LockableCritSection>;

		void lock()
		{
			EnterCriticalSection(&cs);
		}

		void unlock()
		{
			LeaveCriticalSection(&cs);
		}

		LockableCritSection()
		{
			InitializeCriticalSection(&cs);
		}

		virtual ~LockableCritSection()
		{
			DeleteCriticalSection(&cs);
		}

		bool locked() const
		{
			if(TryEnterCriticalSection(const_cast<CRITICAL_SECTION*>(&cs)))
			{
				LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&cs));
				return false;
			}
			return true;
		}
	};
	typedef LockableCritSection Mutex;
#else

	/// pthreads mutex wrapper
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
#endif
	/// Mutex without actual locking. Just a placeholder
	class SimpleMutex
	{
	public:
		int counter;
		SimpleMutex()
		{
			counter = 0;
		}
		bool locked()const
		{
			return counter > 0;
		}
		void lock()
		{
			counter++;
		}
		void unlock()
		{
			counter--;
		}
	};
};
#endif
