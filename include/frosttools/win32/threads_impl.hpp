#ifndef _FT_THREADS_IMPL_HPP_
#define _FT_THREADS_IMPL_HPP_

#include <windows.h>

#include <process.h>

namespace Threading
{
	class thread
	{
		HANDLE hThread;
    	unsigned threadID;
	public:
		thread()
		{
			hThread = 0;
			threadID = 0;
		}

		template<class Function> thread(Function fn)
		{
			id = 0;
			run(fn);
		}

		void join()
		{
			void * result = NULL;
			WaitForSingleObject(hThread, INFINITE);
		}

		template<class Function> void run(Function fn)
		{
			assert(hThread == 0);
			hThread = (HANDLE)_beginthreadex( NULL, 0, (unsigned int (__stdcall*)(void*))&s_run<Function>, &fn, 0, &threadID );
			//pthread_create(&id, NULL, (void*(*)(void*))&s_run<Function>, &fn);
		}
		
		template<class Arg> void run(void (*func)(Arg *), Arg * arg)
		{
			typename _thread_helper::FnWrapper1<Arg> wrapper = {func, arg};
			run(wrapper);
		}
	protected:
		template<class Function> static void s_run(Function * own)
		{
			Function tmpfn = *own;
			tmpfn();
		}		
	};

	inline void sleep(int msec)
	{
		Sleep(msec);
	}

	class LockableCritSection
	{
		CRITICAL_SECTION cs;
	public:

		//friend class ScopedLock<LockableCritSection>;

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
};
#endif
