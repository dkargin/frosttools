#ifndef _FT_THREADS_HPP_
#define _FT_THREADS_HPP_

namespace Threading
{
	struct _thread_helper
	{
		/// Wraps function with 1 argument
		template<class Arg0> struct FnWrapper1
		{
			typedef void (*fnptr)(Arg0 );
			fnptr func;
			Arg0 arg0;
			typedef FnWrapper1<Arg0> wrap_type;

			/// create wrapper
			static wrap_type * create(fnptr fn, Arg0 arg0)
			{
				wrap_type * result = new wrap_type;
				result->arg0 = arg0;
				result->func = fn;
				return result;
			}

			static void * run(void *data)
			{
				wrap_type * f = (wrap_type*)data;
				f->func(f->arg0);
				delete f;
				return NULL;
			}
		protected:
			~FnWrapper1()
			{
			}
		};
		/// Wraps function with 2 arguments
		template<class Arg0, class Arg1>	struct FnWrapper2
		{
			typedef void (*fnptr)(Arg0 , Arg1 );
			fnptr func;
			Arg0 arg0;
			Arg1 arg1;

			typedef FnWrapper2<Arg0, Arg1> wrap_type;

			static wrap_type * create(fnptr fn, Arg0 arg0, Arg1 arg1)
			{
				wrap_type * result = new wrap_type;
				result->arg0 = arg0;
				result->arg1 = arg1;
				result->func = fn;
				return result;
			}

			static void * run(void *data)
			{
				wrap_type * f = (wrap_type*)data;
				f->func(f->arg0, f->arg1);
				delete f;
				return NULL;
			}
		protected:
			~FnWrapper2()
			{
			}
		};


	};

	/// helper to call functor
	template<class Function> void functor_runner(Function * own)
	{
		Function tmpfn = *own;
		tmpfn();
	}
}

#ifdef WIN32
#include "win32/threads_impl.hpp"
#else
#include "linux/threads_impl.hpp"
#endif

namespace Threading
{
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
