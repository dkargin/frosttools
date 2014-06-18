#ifndef FROSTTOOLS_DELEGATE
#define FROSTTOOLS_DELEGATE

#include <assert.h>

namespace frosttools
{

/// @cond DOX_DEEP_IMPLEMENTATION=1
/// Empty argument placeholder
struct NullArg
{
	/// Default constructor
	NullArg() {}
};

/// Empty argument test - positive variants
template<class Type> struct NotNullArg
{
	/// Dummt value
	enum Test
	{
		value=1
	};
};

/// Empty argument test - negative variant
template<> struct NotNullArg<NullArg>
{
	/// Dummy value
	enum Test
	{
		value=0
	};
};

template<	class ret,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5,class Arg6>
struct SignatureSingle
{
	typedef Arg1 _Arg1;
	typedef Arg2 _Arg2;
	typedef Arg3 _Arg3;
	typedef Arg4 _Arg4;
	typedef Arg5 _Arg5;
	typedef Arg6 _Arg6;
	static const int N=6;
	typedef ret (*single)(Arg1,Arg2,Arg3,Arg4,Arg5,Arg6);
};

template<	class ret>
struct SignatureSingle<ret,NullArg,NullArg,NullArg,NullArg,NullArg,NullArg>
{
	static const int N=0;
	typedef ret (*single)();
};

template<	class ret,class Arg1>
struct SignatureSingle<ret,Arg1,NullArg,NullArg,NullArg,NullArg,NullArg>
{
	typedef Arg1 _Arg1;
	static const int N=1;
	typedef ret (*single)(Arg1);
};

template<	class ret,class Arg1,class Arg2>
struct SignatureSingle<ret,Arg1,Arg2,NullArg,NullArg,NullArg,NullArg>
{
	typedef Arg1 _Arg1;
	typedef Arg2 _Arg2;
	static const int N=2;
	typedef ret (*single)(Arg1,Arg2);
};

template<	class ret,class Arg1,class Arg2,class Arg3>
struct SignatureSingle<ret,Arg1,Arg2,Arg3,NullArg,NullArg,NullArg>
{
	typedef Arg1 _Arg1;
	typedef Arg2 _Arg2;
	typedef Arg3 _Arg3;
	static const int N=3;
	typedef ret (*single)(Arg1,Arg2,Arg3);
};
template<class ret,class Arg1,class Arg2,class Arg3,class Arg4>
struct SignatureSingle<ret,Arg1,Arg2,Arg3,Arg4,NullArg,NullArg>
{
	struct X{};
	typedef Arg1 _Arg1;
	typedef Arg2 _Arg2;
	typedef Arg3 _Arg3;
	typedef Arg4 _Arg4;
	static const int N=4;
	typedef ret (*single)(Arg1,Arg2);
};

template<	class Owner,class ret,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5,class Arg6>
struct MemberFn: public SignatureSingle<ret,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6>
{
	typedef SignatureSingle<ret,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6> parent_type;
	typedef ret (Owner::*member)(Arg1,Arg2,Arg3,Arg4,Arg5,Arg6);
	static const int N=parent_type::N;

	Owner *owner;
	member *func;

	MemberFn(Owner *o,member *fn)
		:owner(o),func(fn)
	{}

};

template< class Owner,class ret>
struct MemberFn<Owner,ret,NullArg,NullArg,NullArg,NullArg,NullArg,NullArg>//: public SignatureSingle<ret,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6>
{
	typedef ret (Owner::*member)(void);
};

///\endcond DOX_DEEP_IMPLEMENTATION
//class SignatureMember:
template<	class ret,
			class Arg1=NullArg,
			class Arg2=NullArg,
			class Arg3=NullArg,
			class Arg4=NullArg,
			class Arg5=NullArg,
			class Arg6=NullArg>
class Delegate
{
private:

	struct X {};
	typedef ret (X::*MemberFn)(void);

	X *_ptr;
	MemberFn _func;
	typedef ret (*SingleFn)(void);
	int argc;	// argument count

public:
	/// Method signature
	typedef SignatureSingle<ret,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6> signature;
	typedef typename signature::single single;

	/// Return type
	typedef ret return_type;

	/// Default constructor
	Delegate()
		: _ptr(0), _func(0)
	{}
	/// Copy constructor
	Delegate(const Delegate &delegate)
		: _ptr(delegate._ptr), _func(delegate._func)
	{}

	/// Constructor
	/**
	 * Constructs delegate using class method
	 * @param owner class pointer
	 * @param func pointer to class method
	 */
	template <class Owner>
		Delegate(Owner *owner, return_type (Owner::*func)(void))
		: _ptr(0), _func(0)
	{
		assert(signature::N==0);
		_func = reinterpret_cast<MemberFn>(func);
		_ptr = reinterpret_cast<X *>(owner);
	}
	/// Constructor
	/**
	 * Constructs delegate using class method
	 * @param owner class pointer
	 * @param func pointer to class method
	 */
	template <class Owner>
	Delegate(Owner *owner, ret (Owner::*func)(Arg1))
	{
		assert(signature::N==1);
		_func = reinterpret_cast<MemberFn>(func);
		_ptr = reinterpret_cast<X *>(owner);
	}
	/// Constructor
	/**
	 * Constructs delegate using class method
	 * @param owner class pointer
	 * @param func pointer to class method
	 */
	template <class Owner>
	Delegate(Owner *owner, ret (Owner::*func)(Arg1, Arg2))
	{
		assert(signature::N==2);
		_func = reinterpret_cast<MemberFn>(func);
		_ptr = reinterpret_cast<X *>(owner);
	}
	/// Constructor
	/**
	 * Constructs delegate using class method
	 * @param owner class pointer
	 * @param func pointer to class method
	 */
	template <class Owner>//, class Arg1, class Arg2>
	Delegate(Owner *owner, ret (Owner::*func)(Arg1, Arg2, Arg3))
	{
		assert(signature::N==3);
		_func = reinterpret_cast<MemberFn>(func);
		_ptr = reinterpret_cast<X *>(owner);
	}

	/// Check if delegate is attached somewhere
	bool operator ! () const
	{
		return (!_ptr || !_func);
	}

	/// Check if two delegates are equal
	bool operator == (const Delegate &delegate)
	{
		return (_ptr == delegate._ptr && _func == delegate._func);
	}

	/// Check if two delegates are not equal
	bool operator != (const Delegate &delegate)
	{
		return !(*this == delegate);
	}
	/// Invoke a delegate
	return_type operator ()(Arg1 arg1=NullArg(),Arg2 arg2=NullArg(),Arg3 arg3=NullArg(),Arg4 arg4=NullArg(),Arg5 arg5=NullArg(),Arg6 arg6=NullArg())
	{
		//static parameter list check
		//typedef char argumentTest[(NotNullArg<Arg1>::value && NotNullArg<Arg2>::value)?1:-1];
		if (_ptr && _func)
		{
			if(!NotNullArg<Arg1>::value)
			{
				typedef return_type ( X::*Mfn)();
				return (_ptr->*reinterpret_cast<Mfn>(_func))();
			}
			if(!NotNullArg<Arg2>::value)
			{
				typedef return_type ( X::*Mfn)(Arg1);
				return (_ptr->*reinterpret_cast<Mfn>(_func))(arg1);
			}
			if(!NotNullArg<Arg3>::value)
			{
				typedef return_type ( X::*Mfn)(Arg1,Arg2);
				return (_ptr->*reinterpret_cast<Mfn>(_func))(arg1,arg2);
			}
			if(!NotNullArg<Arg4>::value)
			{
				typedef return_type ( X::*Mfn)(Arg1,Arg2,Arg3);
				return (_ptr->*reinterpret_cast<Mfn>(_func))(arg1,arg2,arg3);
			}
			if(!NotNullArg<Arg5>::value)
			{
				typedef return_type ( X::*Mfn)(Arg1,Arg2,Arg3,Arg4);
				return (_ptr->*reinterpret_cast<Mfn>(_func))(arg1,arg2,arg3,arg4);
			}
			if(!NotNullArg<Arg6>::value)
			{
				typedef return_type ( X::*Mfn)(Arg1,Arg2,Arg3,Arg4,Arg5);
				return (_ptr->*reinterpret_cast<Mfn>(_func))(arg1,arg2,arg3,arg4,arg5);
			}
			else
			{
				typedef return_type ( X::*Mfn)(Arg1,Arg2,Arg3,Arg4,Arg5,Arg6);
				return (_ptr->*reinterpret_cast<Mfn>(_func))(arg1,arg2,arg3,arg4,arg5,arg6);
			}
		}
		assert(false);
	}


	/// Attach delegate
	/**
	 * Attaches delegate to class method
	 * @param owner class pointer
	 * @param func pointer to class method
	 */
	template <class Owner>
	void attach(Owner *owner, return_type (Owner::*func)(void))
	{
		assert(signature::N==0);
		_func = reinterpret_cast<MemberFn>(func);
		_ptr = reinterpret_cast<X *>(owner);
	}
	/// Attach delegate
	/**
	 * Attaches delegate to class method
	 * @param owner class pointer
	 * @param func pointer to class method
	 */
	template <class Owner>
	void attach(Owner *owner, ret (Owner::*func)(Arg1))
	{
		assert(signature::N==1);
		_func = reinterpret_cast<MemberFn>(func);
		_ptr = reinterpret_cast<X *>(owner);
	}
	/// Attach delegate
	/**
	 * Attaches delegate to class method
	 * @param owner class pointer
	 * @param func pointer to class method
	 */
	template <class Owner>
	void attach(Owner *owner, ret (Owner::*func)(Arg1, Arg2))
	{
		assert(signature::N==2);
		_func = reinterpret_cast<MemberFn>(func);
		_ptr = reinterpret_cast<X *>(owner);
	}
	/// Attach delegate
	/**
	 * Attaches delegate to class method
	 * @param owner class pointer
	 * @param func pointer to class method
	 */
	template <class Owner>//, class Arg1, class Arg2>
	void attach(Owner *owner, ret (Owner::*func)(Arg1, Arg2, Arg3))
	{
		assert(signature::N==3);
		_func = reinterpret_cast<MemberFn>(func);
		_ptr = reinterpret_cast<X *>(owner);
	}
	/// Detach delegate
	void detach()
	{
		_func=NULL;
		_ptr=NULL;
	}
	/// Check if delegate is attached
	inline bool attached()
	{
		return _ptr && _func;
	}
	/// Cast to bool - checks if delegate is valid
	inline operator bool()
	{
		return attached();
	}
	/*
	ret_type operator()(Arg1 arg)
	{
		//static parameter list check
		typedef char argumentTest[(NotNullArg<Arg1>::value && NotNullArg<Arg2>::value)?1:-1];
		typedef ret_type ( X::*Mfn)(Arg);
		Mfn func = reinterpret_cast<Mfn>(_func);

		if (_ptr && func)
			return (_ptr->*func)(arg);
	}*/

	/*template <class Arg1, class Arg2>
		ret_type operator() (Arg1 arg1, Arg2 arg2)
	{
		typedef ret_type(X::*Mfn)(Arg1, Arg2);
		Mfn func = reinterpret_cast<Mfn>(_func);

		if (_ptr && func)
			return (_ptr->*func)(arg1, arg2);
	}*/
	/// Get function signature
	template<class Owner>
	::MemberFn<Owner,ret,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6> getSignatureSingle(Owner *ptr)
	{
		::MemberFn<Owner,ret,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6> sig;
		return sig;
	}
};
/*
template<	class ret,
			class Arg1=NullArg,
			class Arg2=NullArg,
			class Arg3=NullArg,
			class Arg4=NullArg,
			class Arg5=NullArg,
			class Arg6=NullArg>
class Delegate2
{
};*/


/*template<class Owner, class Signature>
class MemberFn
{
	typedef ret (X::*MemberFn)(void);
	Owner *owner;
public:
};*/

}
#endif
