#ifndef AUTOPTR_HPP
#define AUTOPTR_HPP

#include <cassert>
#include <memory.h>
#include <stddef.h>


template<class Type> inline void DeleterEmpty(Type* ptr){}
template<class Type> inline void DeleterBasic(Type* ptr)
{
	delete ptr;
}
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

// This info is shared between object instance and all pointers to this object
template<class _Type> struct BaseObjectInfo
{
    typedef _Type target_type;
    typedef BaseObjectInfo<_Type> my_type;
    _Type * data;
    int refCounter;
    int ptrCounter;

#ifdef _SHARED_PROTOTYPE
    bool instance;
    int flags;
#endif
    BaseObjectInfo() : refCounter(0), ptrCounter(0), data( NULL )
    {
#ifdef _SHARED_PROTOTYPE
        flags = 0;
        instance = false;
#endif
    }

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
    static void addReference(target_type * data)
    {
        assert(data != NULL);
		my_type * info = data->objectInfo;
        assert(info != NULL);
		info->refCounter++;
    }
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
///////////////////////////////////////////////////////////////////////////////////
// Base for any object to be referenced by "shared" model
// 1. You can obtain shared pointer from raw pointer to object
// 2. Can handle "static" object allocation by overloading "new" operator
//	( need to decrease reference manually after allocation)
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
	typedef BaseObjectInfo<Referenced> ObjectInfo;

	ObjectInfo * objectInfo;

	Referenced() : objectInfo(NULL)
	{
		objectInfo = ObjectInfo::allocate();
		objectInfo->data = this;
		objectInfo->refCounter = 0;
		objectInfo->ptrCounter = 1;
	}

	virtual ~Referenced()
	{
		//_CrtCheckMemory();
		objectInfo->data = NULL;
		ObjectInfo::releaseInfo(objectInfo);
	}
public:
	template< class Type > BaseObjectInfo<Type> * getObjectInfo()
	{
		return (BaseObjectInfo<Type>*)this->objectInfo;
	}
};

template<class Type, class RefBase = Referenced>
class SharedPtr
{
public:
	typedef Type value_type;
	typedef SharedPtr<Type, RefBase> my_type;
	typedef value_type * ptr_type;
	typedef RefBase _RefBase;
	typedef BaseObjectInfo<Type> ObjectInfo;

	SharedPtr() : objectInfo(NULL) {}

	SharedPtr(ptr_type ptr) : objectInfo(NULL)
	{
		bind(ptr);
	}

	SharedPtr(const SharedPtr &ptr)	: objectInfo( NULL )
	{
		bind(ptr);
	}

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
	my_type & operator = (const value_type * ptr)
	{
		bind((value_type*)ptr);
		return (*this);
	}

	my_type & operator = (const my_type & ptr)
	{
		bind( ptr.get() );
		return *this;
	}

	ptr_type operator -> () const
	{
		ptr_type result = get();
		assert(result);
		return result;
	}

	operator ptr_type() const
	{
		return get();
	}

	ptr_type get() const
	{
		if(objectInfo && objectInfo->alive())
		{
			return objectInfo->data;
		}
		return NULL;
	}
protected:
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
	ObjectInfo * objectInfo;
};

template<class Type, class RefBase = Referenced>
class WeakPtr
{
public:
	typedef Type value_type;
	typedef WeakPtr<Type, RefBase> my_type;
	typedef value_type * ptr_type;
	typedef BaseObjectInfo<Type> ObjectInfo;

	WeakPtr() : objectInfo(NULL) {}

	WeakPtr(ptr_type  ptr) : objectInfo(NULL)
	{
		bind(ptr);
	}

	WeakPtr(const WeakPtr &ptr)	: objectInfo( NULL )
	{
		bind(ptr);
	}

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
	my_type & operator = (const value_type * ptr)
	{
		bind((value_type*)ptr);
		return (*this);
	}

	ptr_type operator -> () const
	{
		ptr_type result = get();
		assert(result);
		return result;
	}

	operator ptr_type() const
	{
		return get();
	}

	ptr_type get() const
	{
		if(objectInfo && objectInfo->alive())
		{
			return objectInfo->data;
		}
		return NULL;
	}
protected:
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
	ObjectInfo * objectInfo;
};

// When we do not want to use dynamic allocation, but going to use shared model
template<class Type>
class Instance
{
public:
	typedef Instance<Type> my_type;
	Type instance;
	//WeakPtr<Type> testPtr;
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
	template<class Arg0> Instance(Arg0 arg0) : instance(arg0)
	{
		bind();
	}
	template<class Arg0, class Arg1> Instance(Arg0 arg0, Arg1 arg1) : instance(arg0,arg1)
	{
		bind();
	}
	const Type * operator -> () const
	{
		return &instance;
	}
	Type * operator -> ()
	{
		return &instance;
	}
	operator const Type *() const
	{
		return &instance;
	}
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

#endif
