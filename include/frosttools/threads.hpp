#ifndef _FT_THREADS_HPP_
#define _FT_THREADS_HPP_

#include <stdlib.h>
#include <assert.h>

namespace frosttools
{

/// \addtogroup Threading
/// Contains cross-platform tools for working with thread synchronization
/// @{
namespace threading
{
	//! Base class for any lockable construct
	class BaseLockable
	{
	public:
		virtual ~BaseLockable() {}
		/// Lock object
		virtual bool lock() = 0;
		/// Unlock object
		virtual bool unlock() = 0;
	};

	//! Local helper to wrap thread functions.
	/**
	 * Not so safe
	 */
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

			/// actual thread function, that calls stored function pointer
			static void * run(void *data)
			{
				wrap_type * f = (wrap_type*)data;
				f->func(f->arg0);
				delete f;
				return 0;
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
				return 0;
			}
		protected:
			~FnWrapper2()
			{
			}
		};
	};

	//! helper to call functor
	template<class Function> inline void functor_runner(Function * own)
	{
		Function tmpfn = *own;
		tmpfn();
	}

	template<class Function> struct FunctionWrapper
	{
		/// Function pointer
		Function * func;

		/// Defines wrapper type
		typedef FunctionWrapper<Function> wrap_type;

		/// Create wrapper
		static wrap_type * create(Function &fn)
		{
			wrap_type * result = new wrap_type;
			result->func = &fn;
			return result;
		}

		/// Runs wrapped functor
		static void * run(void * data)
		{
			wrap_type * f = (wrap_type*)data;
			(*f->func)();
			delete f;
			return 0;
		}
	};

	//! Return status for condition variable. Returned by CV::wait_for
	enum CvStatus
	{
		cvError,
		cvNoTimeout,
		cvTimeout,
	};
}	// namespace threading

} // namespace frosttools

#ifdef _MSC_VER
#include "win32/threads_impl.hpp"
#else
#include "linux/threads_impl.hpp"
#endif

namespace frosttools
{
namespace threading
{
	/// Scoped locker
	template<class Lockable = BaseLockable>
	class ScopedLock
	{
	public:
		/// reference to locking object
		Lockable & mutex;

		/// Constructor
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
	class SimpleMutex : public BaseLockable
	{
	public:
		int counter;	///< Lock counter

		SimpleMutex()
		{
			counter = 0;
		}

		/// Check if mutex is locked
		bool locked()const
		{
			return counter > 0;
		}

		/// Lock object
		bool lock()
		{
			counter++;
			return true;
		}

		/// Unlock object
		bool unlock()
		{
			counter--;
			return false;
		}
	};
};
/// @}
} // namespace frosttools
#endif
