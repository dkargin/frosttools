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

		typedef unsigned int (__stdcall* thread_fn)(void*);
		void join()
		{
			void * result = NULL;
			WaitForSingleObject(hThread, INFINITE);
		}
		/*
		template<class Function> void run(Function fn)
		{
		assert(id == 0);
		pthread_create(&id, NULL, (thread_fn)&functor_runner<Function>, &fn);
		}*/

		template<class Arg> void run(void (*func)(Arg ), Arg arg)
		{
			typedef _thread_helper::FnWrapper1<Arg> wrapper;
			wrapper * w = wrapper::create(func, arg);
			///pthread_create(&id, NULL, wrapper::run, w);
			assert(hThread == 0);
			hThread = (HANDLE)_beginthreadex( NULL, 0, (thread_fn)wrapper::run, w, 0, &threadID );
		}
		template<class Function> void run(Function fn)
		{
			assert(hThread == 0);
			hThread = (HANDLE)_beginthreadex( NULL, 0, (thread_fn)&functor_runner<Function>, &fn, 0, &threadID );
			//pthread_create(&id, NULL, (void*(*)(void*))&s_run<Function>, &fn);
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

	class mutex
	{
		HANDLE handle;
	public:
		mutex()
		{
			handle = CreateMutex(NULL, FALSE, NULL);
		}
		~mutex()
		{
			CloseHandle(handle);
		}

		HANDLE getHandle() const
		{
			return handle;
		}

		void lock()
		{
			WaitForSingleObject (handle, INFINITE);
		}
		void unlock()
		{
			ReleaseMutex(handle);
		}
		bool try_lock()
		{
			if(WaitForSingleObject (handle, 0) == WAIT_OBJECT_0)
			{
				//unlock();
				return false;
			}
			return true;
		}
	private:
		mutex(const mutex & mutex);
	};

	typedef struct
	{
		int waiters_count_;
		// Number of waiting threads.

		CRITICAL_SECTION waiters_count_lock_;
		// Serialize access to <waiters_count_>.

		HANDLE sema_;
		// Semaphore used to queue up threads waiting for the condition to
		// become signaled. 

		HANDLE waiters_done_;
		// An auto-reset event used by the broadcast/signal thread to wait
		// for all the waiting thread(s) to wake up and be released from the
		// semaphore. 

		size_t was_broadcast_;
		// Keeps track of whether we were broadcasting or signaling.  This
		// allows us to optimize the code if we're just signaling.
	} pthread_cond_t;

	typedef void pthread_condattr_t;
	typedef HANDLE pthread_mutex_t;

	int	pthread_cond_init (pthread_cond_t *cv, const pthread_condattr_t *)
	{
		cv->waiters_count_ = 0;
		cv->was_broadcast_ = 0;
		cv->sema_ = CreateSemaphore (NULL,       // no security
			0,          // initially 0
			0x7fffffff, // max count
			NULL);      // unnamed 
		InitializeCriticalSection (&cv->waiters_count_lock_);
		cv->waiters_done_ = CreateEvent (NULL,  // no security
			FALSE, // auto-reset
			FALSE, // non-signaled initially
			NULL); // unnamed
		return 0;
	}

	int pthread_cond_destroy(pthread_cond_t * cv)
	{
		EnterCriticalSection (&cv->waiters_count_lock_);
		CloseHandle(cv->sema_);
		cv->sema_ = NULL;
		CloseHandle(cv->waiters_done_);
		cv->waiters_done_ = NULL;
		cv->waiters_count_ = -1;
		LeaveCriticalSection (&cv->waiters_count_lock_);
		DeleteCriticalSection(&cv->waiters_count_lock_);
		return 0;
	}

	int	pthread_cond_wait (pthread_cond_t *cv, pthread_mutex_t *external_mutex)
	{
		// Avoid race conditions.
		EnterCriticalSection (&cv->waiters_count_lock_);
		cv->waiters_count_++;
		LeaveCriticalSection (&cv->waiters_count_lock_);

		// This call atomically releases the mutex and waits on the
		// semaphore until <pthread_cond_signal> or <pthread_cond_broadcast>
		// are called  by another thread.
		SignalObjectAndWait (*external_mutex, cv->sema_, INFINITE, FALSE);

		// Reacquire lock to avoid race conditions.
		EnterCriticalSection (&cv->waiters_count_lock_);

		// We're no longer waiting...
		cv->waiters_count_--;

		// Check to see if we're the last waiter after <pthread_cond_broadcast>.
		int last_waiter = cv->was_broadcast_ && cv->waiters_count_ == 0;

		LeaveCriticalSection (&cv->waiters_count_lock_);

		// If we're the last waiter thread during this particular broadcast
		// then let all the other threads proceed.
		if (last_waiter)
			// This call atomically signals the <waiters_done_> event and waits until
				// it can acquire the <external_mutex>.  This is required to ensure fairness. 
					SignalObjectAndWait (cv->waiters_done_, *external_mutex, INFINITE, FALSE);
		else
			// Always regain the external mutex since that's the guarantee we
			// give to our callers. 
			WaitForSingleObject (*external_mutex, INFINITE);
		return 0;
	}

	int	pthread_cond_signal (pthread_cond_t *cv)
	{
		EnterCriticalSection (&cv->waiters_count_lock_);
		int have_waiters = cv->waiters_count_ > 0;
		LeaveCriticalSection (&cv->waiters_count_lock_);

		// If there aren't any waiters, then this is a no-op.  
		if (have_waiters)
			ReleaseSemaphore (cv->sema_, 1, 0);
		return 0;
	}

	int	pthread_cond_broadcast (pthread_cond_t *cv)
	{
		// This is needed to ensure that <waiters_count_> and <was_broadcast_> are
		// consistent relative to each other.
		EnterCriticalSection (&cv->waiters_count_lock_);
		int have_waiters = 0;

		if (cv->waiters_count_ > 0) {
			// We are broadcasting, even if there is just one waiter...
			// Record that we are broadcasting, which helps optimize
			// <pthread_cond_wait> for the non-broadcast case.
			cv->was_broadcast_ = 1;
			have_waiters = 1;
		}

		if (have_waiters) {
			// Wake up all the waiters atomically.
			ReleaseSemaphore (cv->sema_, cv->waiters_count_, 0);

			LeaveCriticalSection (&cv->waiters_count_lock_);

			// Wait for all the awakened threads to acquire the counting
			// semaphore. 
			WaitForSingleObject (cv->waiters_done_, INFINITE);
			// This assignment is okay, even without the <waiters_count_lock_> held 
			// because no other waiter threads can wake up to access it.
			cv->was_broadcast_ = 0;
		}
		else
			LeaveCriticalSection (&cv->waiters_count_lock_);
		return 1;
	}

	class condition_variable
	{
		pthread_cond_t cv;
	public:
		condition_variable()
		{
			pthread_cond_init(&cv, NULL);
		}

		~condition_variable()
		{
			pthread_cond_destroy(&cv);
		}

		void wait(mutex &mutex)
		{
			HANDLE mHandle = mutex.getHandle();
			pthread_cond_wait(&cv, &mHandle);
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

	//typedef LockableCritSection Mutex;
};
#endif
