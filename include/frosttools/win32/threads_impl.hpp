#ifndef _FT_THREADS_HPP_
#define _FT_THREADS_HPP_

#include <windows.h>

namespace Threading
{
	inline void sleep(int msec)
	{
		Sleep(msec);
	}

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
};
#endif
