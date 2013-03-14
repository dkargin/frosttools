#ifndef _FT_THREADS_HPP_
#define _FT_THREADS_HPP_

namespace Threading
{
	struct _thread_helper
	{
		/// Wraps function with 1 argument
		template<class Arg0> struct FnWrapper1
		{
			void (*func)(Arg0 *);
			Arg0 * arg0;
			void * __cdecl operator()()
			{
				func(arg0);
				return NULL;
			}
		};
		/// Wraps function with 2 arguments
		template<class Arg0, class Arg1>	struct FnWrapper2
		{
			void (*func)(Arg0 *, Arg1 *);
			Arg0 * arg0;
			Arg1 * arg1;
			void * __cdecl operator()()
			{
				func(arg0, arg1);
				return NULL;
			}
		};
	};
}

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
