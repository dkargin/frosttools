#ifndef AUTOPTR_HPP
#define AUTOPTR_HPP

#include <assert.h>
#include <memory.h>

namespace frosttools
{
/// Empty deleter
template<class Type> inline void DeleterEmpty(Type* ptr){}

/// Basic deleter
template<class Type> inline void DeleterBasic(Type* ptr)
{
	delete ptr;
}

/// Deleter using 'release' method
template<class Type> inline void Releaser(Type* ptr)
{
	ptr->release();
}
/*
template<class Type,class Deleter = DeleterEmpty<Type> >
class AutoPtr
{
	Type * pointer;
public:
	typedef Type my_type;

	AutoPtr():pointer((Type*)NULL){}

	AutoPtr(Type *ptr):pointer(ptr){}

	virtual ~AutoPtr()
	{
		if(valid())
			Deleter(pointer);
	}

	inline Type * attach(Type *ptr)
	{
		pointer=ptr;
		return pointer;
	}

	inline operator const Type * () const
	{
		return pointer;
	}

	inline operator Type * ()
	{
		return pointer;
	}

	inline const Type * operator->() const
	{
		return pointer;
	}

	inline Type * operator->()
	{
		return pointer;
	}

	inline const Type * getPtr() const
	{
		return pointer;
	}

	inline Type * getPtr()
	{
		return pointer;
	}

	inline bool valid()
	{
		return pointer!=NULL;
	}
};

*/
//template<class Type> class WeakPtr;
template<class Type, class RefBase> class SharedPtr;

/// This info is shared between object instance and all pointers to this object
template<class _Type> struct BaseObjectInfo
{
	/// Defines target type
    typedef _Type target_type;
    /// Defines object info for target type
    typedef BaseObjectInfo<_Type> my_type;
    /// Pointer to actual data
    _Type * data;
    /// Data reference counter
    int refCounter;
    /// ObjectInfo reference counter
    int ptrCounter;

#ifdef _SHARED_PROTOTYPE
    bool instance;
    int flags;
#endif
    /// Default constructor
    BaseObjectInfo() : data( NULL ), refCounter(0), ptrCounter(0)
    {
#ifdef _SHARED_PROTOTYPE
        flags = 0;
        instance = false;
#endif
    }

    /// Check if there are any data references
    bool alive() const
    {
        return data != NULL && refCounter != 0;
    }
#ifdef _SHARED_PROTOTYPE
    ~BaseObjectInfo()
    {
        assert(ptrCounter == 0);
    }
#endif
    /// add data reference
    static void addReference(target_type * data)
    {
        assert(data != NULL);
		my_type * info = data->objectInfo;
        assert(info != NULL);
		info->refCounter++;
    }
    /// Recrement data reference
    /// Releases object if reference counter is zero
    static bool removeReference(target_type * data)
    {
        assert(data);
        BaseObjectInfo<target_type> * info = data->objectInfo;
        assert(info != NULL);
        info->refCounter--;
        if(info->refCounter == 0)
        {
            releaseObject(data);
            info->data = NULL;
            return true;
        }
        return false;
    }
    /// releases object. Calls default deallocator
    static void releaseObject(target_type * data)
    {/*
        assert(data);
        assert(data->objectInfo->refCounter == 0);
#ifdef _SHARED_PROTOTYPE
        assert(data->objectInfo->flags >= 0);
        assert(data->objectInfo->instance != true);
        data->objectInfo->flags--;
#endif
*/
        delete data;
    }

    /// Allocates BaseObjectInfo
    static BaseObjectInfo<target_type> * allocate()
    {
#ifdef BASE_OBJECT_INFO_STATIC
        static int counter = 0;
        static ObjectInfo headers[8];
        assert(counter < 8);
        BaseObjectInfo<target_type> * result = headers + counter++;
        result->flags = counter;
        return result;
#else
        return new BaseObjectInfo<target_type>;
#endif
    }

    /// Deallocates ObjectInfo
    static bool releaseInfo(BaseObjectInfo<target_type> * info)
    {
        info->ptrCounter--;
        assert(info->ptrCounter >= 0);
        if( info->ptrCounter == 0 )
        {
#ifdef BASE_OBJECT_INFO_STATIC
#else
//				_CrtCheckMemory();
            delete info;
#endif
            return false;
        }
        return true;
    }
};

///\brief Implements intrusive reference counter
/* Base for any object to be referenced by "shared" model
 * 1. You can obtain shared pointer from raw pointer to object
 * 2. Can handle "static" object allocation by overloading "new" operator
 *	( need to decrease reference manually after allocation)
 *
 */
///////////////////////////////////////////////////////////////////////////////////
class Referenced
{
public:
	//template<class Stub> friend class WeakPtr;
	template<class Stub, class RefBase> friend class SharedPtr;
	template<class Stub, class RefBase> friend class WeakPtr;
	template<class Stub> friend class Instance;
	template<class Stub> friend struct BaseObjectInfo;
protected:
	/// Defines shared data type
	typedef BaseObjectInfo<Referenced> ObjectInfo;

	/// pointer to shared data
	ObjectInfo * objectInfo;

	/// Default constructor
	Referenced() : objectInfo(NULL)
	{
		objectInfo = ObjectInfo::allocate();
		objectInfo->data = this;
		objectInfo->refCounter = 0;
		objectInfo->ptrCounter = 1;
	}

	/// Destructor
	virtual ~Referenced()
	{
		//_CrtCheckMemory();
		objectInfo->data = NULL;
		ObjectInfo::releaseInfo(objectInfo);
	}
public:
	/// Access shared data
	template< class Type > BaseObjectInfo<Type> * getObjectInfo()
	{
		return (BaseObjectInfo<Type>*)this->objectInfo;
	}
};

/// Shared pointer
template<class Type, class RefBase = Referenced>
class SharedPtr
{
public:
	/// Defines target value type
	typedef Type value_type;
	/// Defines shared pointer type
	typedef SharedPtr<Type, RefBase> my_type;
	/// Defines target value pointer type
	typedef value_type * ptr_type;
	/// Defines base class for intrusive referencing (shared_from_this construct)
	typedef RefBase _RefBase;
	/// Defines type for shared info
	typedef BaseObjectInfo<Type> ObjectInfo;

	/// Default constructor
	SharedPtr() : objectInfo(NULL) {}

	/// Construct using raw pointer
	SharedPtr(ptr_type ptr) : objectInfo(NULL)
	{
		bind(ptr);
	}

	/// Copy constructor
	SharedPtr(const SharedPtr &ptr)	: objectInfo( NULL )
	{
		bind(ptr);
	}

	/// Destructor
	~SharedPtr()
	{
		bind(NULL);
	}
	/*
	bool operator != (const value_type * ptr)const
	{
		return get() != ptr;
	}

	bool operator != (const my_type & ptr)const
	{
		return get() != ptr.get();
	}

	bool operator == (const value_type * ptr)const
	{
		return get() == ptr;
	}

	bool operator == (const my_type & ptr)const
	{
		return get() == ptr.get();
	}
	*/
	/// Assignment operator
	my_type & operator = (const value_type * ptr)
	{
		bind((value_type*)ptr);
		return (*this);
	}

	/// Assignment operator
	my_type & operator = (const my_type & ptr)
	{
		bind( ptr.get() );
		return *this;
	}

	/// Dereferencing operator
	ptr_type operator -> () const
	{
		ptr_type result = get();
		assert(result);
		return result;
	}

	/// Cast to pointer type. Might be dangerous
	operator ptr_type() const
	{
		return get();
	}

	/// Get raw pointer. Dangerous one
	ptr_type get() const
	{
		if(objectInfo && objectInfo->alive())
		{
			return objectInfo->data;
		}
		return NULL;
	}
protected:
	/// Binds to raw object
	void bind(ptr_type ptr)
	{
		if( objectInfo != NULL )
		{
			objectInfo->refCounter--;
			if( objectInfo->refCounter == 0 && objectInfo->data != NULL )
			{
				ptr_type ptr = objectInfo->data;
				objectInfo->data = NULL;
				ObjectInfo::releaseObject(ptr);
			}
			ObjectInfo::releaseInfo(objectInfo);
			objectInfo = NULL;
		}
		if(ptr != NULL)
		{
			objectInfo = reinterpret_cast<ObjectInfo*>(((RefBase*)ptr)->objectInfo);
			objectInfo->ptrCounter++;
			objectInfo->refCounter++;
		}
	}
	/// pointer to shared data
	ObjectInfo * objectInfo;
};

/// Weak pointer
template<class Type, class RefBase = Referenced>
class WeakPtr
{
public:
	/// Defines value type
	typedef Type value_type;
	/// Defines pointer own type
	typedef WeakPtr<Type, RefBase> my_type;
	/// Defines raw pointer type
	typedef value_type * ptr_type;
	/// Defines shared data type
	typedef BaseObjectInfo<Type> ObjectInfo;

	/// Default constructor
	WeakPtr() : objectInfo(NULL) {}

	/// Constructor
	/// Construct using raw pointer
	WeakPtr(ptr_type  ptr) : objectInfo(NULL)
	{
		bind(ptr);
	}

	/// Copy constructor
	WeakPtr(const WeakPtr &ptr)	: objectInfo( NULL )
	{
		bind(ptr);
	}

	/// Destructor
	~WeakPtr()
	{
		bind(NULL);
	}
	/*
	bool operator != (const value_type * ptr)const
	{
		return get() != ptr;
	}

	bool operator != (const my_type & ptr)const
	{
		return get() != ptr.get();
	}

	bool operator == (const value_type * ptr)const
	{
		return get() == ptr;
	}

	bool operator == (const my_type & ptr)const
	{
		return get() == ptr.get();
	}
	*/
	/// Assignment operator
	my_type & operator = (const value_type * ptr)
	{
		bind((value_type*)ptr);
		return (*this);
	}

	/// Dereferencing operator
	ptr_type operator -> () const
	{
		ptr_type result = get();
		assert(result);
		return result;
	}

	/// Cast to pointer type
	/// Returns raw pointer. Dangerous
	operator ptr_type() const
	{
		return get();
	}

	/// Get raw pointer
	ptr_type get() const
	{
		if(objectInfo && objectInfo->alive())
		{
			return objectInfo->data;
		}
		return NULL;
	}
protected:
	/// Bind to raw pointer
	void bind(ptr_type ptr)
	{
		if( objectInfo != NULL )
		{
			ObjectInfo::releaseInfo(objectInfo);
			objectInfo = NULL;
		}
		if(ptr != NULL)
		{		    
			objectInfo = reinterpret_cast<ObjectInfo*>(ptr->objectInfo);
			objectInfo->ptrCounter++;
		}
	}
	/// pointer to shared data
	ObjectInfo * objectInfo;
};

/// When we do not want to use dynamic allocation, but going to use shared model
template<class Type>
class Instance
{
public:
	/// Defines instance type
	typedef Instance<Type> my_type;
	/// Instance
	Type instance;
	/// Default constructor
	Instance()
	{
		bind();
	}
private:
	Instance(const Type & data) : instance(data)
	{
		bind();
	}
	const my_type & operator = (const my_type & ) const;
public:
	~Instance()	{}
	/// Constructor
	template<class Arg0> Instance(Arg0 arg0) : instance(arg0)
	{
		bind();
	}
	/// Constructor
	template<class Arg0, class Arg1> Instance(Arg0 arg0, Arg1 arg1) : instance(arg0,arg1)
	{
		bind();
	}

	/// Const dereferencing operator
	const Type * operator -> () const
	{
		return &instance;
	}
	/// Dereferencing
	Type * operator -> ()
	{
		return &instance;
	}
	/// Cast to const raw pointer
	operator const Type *() const
	{
		return &instance;
	}
	/// Cast to raw pointer
	operator Type *()
	{
		return &instance;
	}
private:
	void bind()
	{
		instance.objectInfo->refCounter++;
#ifdef _SHARED_PROTOTYPE
		instance.objectInfo->instance = true;
#endif
	}
};
#ifdef AUTOPTR_TESTS
struct AutoPtrTest
{
	class Dummy : public Referenced
	{
	public:
		std::string name;
		int value;
		Dummy(const char * name, int value)
			:name(name), value(value)
		{
			printf("Dummy %s created\n", this->name.c_str());
		}
		~Dummy()
		{
			printf("Dummy %s is destroyed\n", name.c_str());
		}
	};

	typedef SharedPtr<Dummy> DummyPtr;

	static void testSinglePtr()
	{
		printf("testSinglePtr started\n");
		{
			DummyPtr dummy(new Dummy("d1", 1));
		}
		printf("testSinglePtr complete\n");
	}

	static void addDummy(std::list<DummyPtr> &data, const char * name, int val)
	{
		data.push_back(new Dummy(name, val));
	}

	static void testLists()
	{
		printf("testList started\n");
		{
			std::list<DummyPtr> data;
			addDummy(data, "d1",1);
			addDummy(data, "d2",2);

			{
				DummyPtr d = data.front();
				data.pop_front();
			}

			printf("d1 should be dead\n");
			DummyPtr d = data.front();
		}
		printf("testLists complete\n");
	}

	void testPtr()
	{
		testSinglePtr();
		testLists();
	}
};
#endif

}
#endif
