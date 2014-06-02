#pragma once

//////////////////////////////////////////////////////////////////////
/// FrostHand's ToolBox. Different utilitary functions
/// v0.5 WIP
//////////////////////////////////////////////////////////////////////
/*! \page page1 A documentation page
  \tableofcontents
  Leading text.
  \section sec An example section
  This page contains the subsections \ref subsection1 and \ref subsection2.
  For more info see page \ref page2.
  \subsection subsection1 The first subsection
  Text.
  \subsection subsection2 The second subsection
  More text.
*/

/*! \page page2 Another page
  Even more info.
*/

#include <fstream>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <math.h>
#include <assert.h>

#ifdef FrostTools_Use_All
#define FrostTools_Use_Asserts
#define FrostTools_Use_Multilayer
#define FrostTools_Use_Listeners
#define FrostTools_Use_Unique
#define FrostTools_Use_Scripter
#define FrostTools_Use_Files
#define FrostTools_Use_Types
#define FrostTools_Use_Raster
#define FrostTools_Use_Delegate
#define FrostTools_Use_System
#define FrostTools_NoWarnings
//#define FrostTools_Use_TaskProcessor only if need
#endif

#ifdef FrostTools_NoWarnings
#pragma warning(disable:4482) //nonstandard extension used
#pragma warning(disable:4244) //conversion from 'DWORD' to 'WORD', possible loss of data
#pragma warning(disable:4305)
#pragma warning(disable:4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable:4018) // signed/unsigned mismatch
#pragma warning(disable:4800) // 'int' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable:4355) // 'this' : used in base member initializer list
#pragma warning(disable:4005) // 'assert' : macro redefinition

#pragma warning(disable:4312) // 'type cast' : conversion from 'long' to 'void *' of greater size
#pragma warning(disable:4311) // 'type cast' : pointer truncation from 'void *const ' to 'long'
#pragma warning(disable:4996) // This function or variable may be unsafe.
#endif

/// Root namespace for all library classes
namespace frosttools
{
#ifdef FrostTools_Use_Types
typedef unsigned int uint;
typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef signed char sint8;
typedef signed short int sint16;
typedef signed int sint32;
typedef int int32;
typedef short int int16;
typedef char int8;
#ifdef TCHAR
typedef std::basic_string<TCHAR> tstring;
#endif
#endif
#define FrostTools_Impl inline

#ifndef FrostTools_Locals
#define FrostTools_Locals
#define rel_size(a,b) ((sizeof(a)+sizeof(b))/sizeof(b))

#include <stdarg.h>

template <typename Char> struct StrUtils {};
template<> struct StrUtils<char>
{
	char * sprintf(char *dst,char *format,...)
	{
		va_list v;
		va_start (v, format);
		::vsprintf(dst,format,v);
		va_end(v);
		return dst;
	}
	char * vsnprintf(char *dst,size_t max,char *format,...)
	{
		va_list v;
		va_start (v, format);
		::vsnprintf(dst,max,format,v);
		va_end(v);
		return dst;
	}
};

#ifdef FROSTTOOLS_WCHAR
template<> struct StrUtils<wchar_t>
{
	wchar_t *sprintf(wchar_t *dst,wchar_t*format,...)
	{
		va_list v;
		va_start (v, format);
		::vswprintf(dst,format,v);
		va_end(v);
		return dst;
	}
	/*wchar_t * vsnprintf(wchar_t *dst,size_t max,wchar_t *format,...)
	{
		va_list v;
		va_start (v, format);
		::vsnwprintf(dst,max,format,v);
		va_end(v);
		return dst;
	}*/
};
#endif

/// Static array for specified type.
/**
 * 	Provides some bounds checking
 */
template<class T, int MaxSize>
class StaticArray
{
public:
	typedef T value_type;
	typedef StaticArray<T,MaxSize> container_type;
	typedef T* ptr_type;
	// constant pointer holder
	struct holder_const
	{
		typedef const T value_type;
		typedef const T * pointer;
		typedef const T & reference;

		const T * ptr;
		operator const T() const
		{
			return *ptr;
		}
		const T operator *() const
		{
			return *ptr;
		}
		const T * operator->() const
		{
			return ptr;
		}
	};
	// free pointer holder
	struct holder
	{
		typedef T value_type;
		typedef T * pointer;
		typedef T & reference;

		T * ptr;
		operator value_type() 
		{
			return *ptr;
		}
		T operator* ()
		{
			return *ptr;
		}
		T * operator->()
		{
			return ptr;
		}
	};

	template<class Base> struct _iterator_base: public Base
	{
		typedef std::random_access_iterator_tag iterator_category;
		typedef int difference_type;		
		typedef _iterator_base<Base> my_type;

		_iterator_base(container_type * container, value_type * data)
		{
			this->ptr = data;
		}
		_iterator_base(const my_type &it)
		{
			this->ptr = it.ptr;
		}
		bool operator == (const my_type & t) const
		{
			return t.ptr == this->ptr;
		}
		bool operator != (const my_type &t) const
		{
			return t.ptr != this->ptr;
		}
		bool operator < (const my_type &t) const
		{
			return this->ptr < t.ptr;
		}
		bool operator > (const my_type &t) const
		{
			return this->ptr > t.ptr;
		}
		my_type & operator++()
		{
			this->ptr++;
			return *this;
		}
		my_type & operator--()
		{
			this->ptr--;
			return *this;
		}
	};

	typedef _iterator_base<holder> iterator;
	typedef _iterator_base<holder_const> const_iterator;	
protected:	
	// Dummy object with the same size, as value_type
	struct Dummy
	{
		char data[sizeof(value_type)];
	};
	Dummy data[MaxSize];
	size_t currentSize;

	void remove(Dummy * dummy)
	{
		//value_type * ptr = static_cast<value_type*>(dummy);
		//ptr->~value_type();
	}
public:
	
	StaticArray():currentSize(0)
	{}
	~StaticArray()
	{
		clear();
	}
	size_t size() const
	{
		return currentSize;
	}
	size_t max_size() const
	{
		return MaxSize;
	}
	iterator begin()
	{
		return iterator(this,(value_type*)(data));
	}
	iterator end()
	{
		return iterator(this,(value_type*)(data + currentSize + 1));
	}
	void push_back(const value_type &t)
	{
		if(currentSize < max_size())
		{			
			void * raw = &data[currentSize++];
			T * data = new (raw) T(t);			
		}
		else
		{
			std::exception("Array: maximum size exceeded");
		}
	}
	bool empty() const
	{
		return currentSize == 0;
	}
	void clear()
	{
		if(!empty())
			while(currentSize--)
			{
				remove(data+currentSize);
			}
	}

	value_type * getData()
	{
		return (value_type*) data;
	}
};
#endif
/// @}
} // namespace frosttools

#ifdef FrostTools_Use_AutoPtr
#include "autoptr.hpp"
#endif
#ifdef FrostTools_Use_RingBuffer
#include "ringbuffer.hpp"
#endif
#ifdef FrostTools_Use_Raster
#include "raster.hpp"
#endif
#ifdef FrostTools_Use_Multilayer
#include "multilayer.hpp"
#endif
#ifdef FrostTools_Use_Listeners
#include "listeners.hpp"
#endif
#ifdef FrostTools_Use_Unique
#include "unique.hpp"
#endif
#ifdef FrostTools_Use_Scripter
#include "scripter.hpp"
#endif
#ifdef FrostTools_Use_Files
#include "sysFiles.hpp"
#endif
#ifdef FrostTools_Use_TaskProcessor
#include "taskProcessor.hpp"
#endif

#ifdef FrostTools_Use_Delegate
#include <delegate.hpp>
#endif
// various system stuff, like DLL loading, object manager, message processing.
#ifdef FrostTools_Use_System
#include "sysManager.hpp"
#endif
#ifdef FrostTools_Use_3DMath
#include "3dmath.h"
#endif
#ifdef FrostTools_Use_Serialiser
#include "serialiser.hpp"
#endif

#include "logger.hpp"

namespace frosttools
{

//\addtogroup Containers
//@{

/// Intrusive tree node
/**
 * 	Each node can have several children.
 *	Children are stored in double-linked list
 */
template<class Type> class TreeNode
{
protected:
	typedef TreeNode<Type> node_type;
	Type *parent;						///< Pointer to parent node
	Type *next, *prev;					///< Pointer to adjacent nodes
	Type *head, *tail;					///< Pointer to head/tail child nodes

	/// Called when node is attached
	virtual void onAttach( Type * object ) = 0;
	/// Called when node is detached
	virtual void onDetach( Type * object ) = 0;
public:

	/// Constructor
	TreeNode()
	{
		parent = NULL;
		next = NULL;
		prev = NULL;
		head = NULL;
		tail = NULL;
	}

	/// Get pointer to target type
	virtual Type * getTargetType() = 0;

	/// Disconnect self from tree
	void orphan_me()
	{
		if( parent != NULL )
			parent->removeChild(this);
	}

	/// Check if there are any chilren
	bool hasChild(const node_type * child) const
	{
		return child && child->parent == this;
	}

	/// Attach child node
	/** newChild is detached first
	 */
	void attach(Type * newChild)
	{
		newChild->orphan_me();
		if( head == NULL)
		{
			head = newChild;
			tail = newChild;
			newChild->prev = NULL;
			newChild->next = NULL;
		}
		else
		{
			tail->next = newChild;
			newChild->prev = tail;
			newChild->next = NULL;
			tail = newChild;
		}
		newChild->parent = static_cast<Type*>(this);
		onAttach(static_cast<Type*>(newChild));
	}

	/// Removes specified child.
	void removeChild(node_type * child)
	{
		assert( hasChild(child) );
		if( child->prev != head )
			child->prev->next = child->next;
		else
			head = child->next;

		if( child->next != tail )
			child->next->prev = child->prev;
		else
			tail = child->prev;
		child->next = NULL;
		child->prev = NULL;
		child->parent = NULL;
		onDetach(static_cast<Type*>(child));
	}

	/// Detaches from tree
	/** Detaches node from existing tree
	 */
	virtual void detach(node_type * child)
	{
		removeChild(child);
	}

	/// Destructor
	virtual ~TreeNode()
	{
		orphan_me();
	}

	/// Const iterator to iterate through immediate children nodes
	class const_iterator
	{
		protected:
		const Type * container;	///< Pointer to container
		const Type * current;		///< Pointer to current node
	public:
		typedef const_iterator iterator_type;	///< own iterator type
		/// Constructor
		const_iterator(const Type * container_, const Type * current_)
		{
			container = container_;
			current = current_;
		}
		/// Copy constructor
		const_iterator(const const_iterator &it)
		{
			container = it.container;
			current = it.current;
		}
		/// Check if iterators are equal
		bool operator == ( const iterator_type & it) const
		{
			return container == it.container && current == it.current;
		}
		/// Check if iterators are not equal
		bool operator != ( const iterator_type & it) const
		{
			return container != it.container || current != it.current;
		}
		///
		const Type * operator->()
		{
			return current;
		}
		/// Prefix increment. Moves to the next element
		iterator_type & operator++()
		{
			assert(current != NULL);
			current = current->next;
			return *this;
		}
		/// Postfix increment
		iterator_type operator++(int)	// postfix
		{
			assert(current != NULL);
			const_iterator result(*this);
			current = current->next;
			return result;
		}
	};

	/// Iterator to iterate through immediate children nodes
	class iterator
	{
	protected:
		Type * container;	///< pointer to the container
		Type * current;		///< pointer to current element
	public:
		typedef iterator iterator_type;		///< own iterator type
		/// Constructor
		iterator(Type * container_, Type * current_)
		{
			container = container_;
			current = current_;
		}
		/// Copy constructor
		iterator(const const_iterator &it)
		{
			container = it.container;
			current = it.current;
		}
		/// Check if iterators are equal
		bool operator == ( const iterator_type & it) const
		{
			return container == it.container && current == it.current;
		}
		/// Check if iterators are different
		bool operator != ( const iterator_type & it) const
		{
			return container != it.container || current != it.current;
		}
		Type * operator->()
		{
			return current;
		}
		/// Prefix increment
		iterator_type & operator++()	// prefix
		{
			assert(current != NULL);
			current = current->next;
			return *this;
		}
		/// Postfix increment
		iterator_type operator++(int)	// postfix
		{
			assert(current != NULL);
			const_iterator result(*this);
			current = current->next;
			return result;
		}
	};

	/// Get const iterator to the first child
	const_iterator begin() const
	{
		return const_iterator(static_cast<const Type*>(this), head);
	}

	/// Get const iterator to the last child
	const_iterator end() const
	{
		return const_iterator(static_cast<const Type*>(this), NULL);
	}

	/// Get iterator to the first child
	iterator begin()
	{
		return iterator(static_cast<Type*>(this),head);
	}
	/// Get iterator to the last child
	iterator end()
	{
		return iterator(static_cast<Type*>(this), NULL);
	}
};

/// Helper to manage list operations
/**
 * 	Type should have accessible prev and next pointers
 */
template<class Type> struct ListHelper
{
	/// typedef for node pointer
	typedef Type * Nodeptr;
	/// Remove node from the list
	static void remove(Nodeptr &head, Nodeptr &tail, Type * node)
	{
		if(node->next != 0)
			node->next->prev = node->prev;
		if(node->prev != 0)
			node->prev->next = node->next;
		if(node == head)
			head = head->next;
		if(node == tail)
			tail = node->prev;
		node->prev = NULL;
		node->next = NULL;
	}

	/// Add node to the front of the list
	static void push_front(Nodeptr &head, Nodeptr &tail, Type * node)
	{
		if( head == NULL )
		{
			head = node;
			tail = node;
		}
		else
		{
			head->prev = node;
			node->next = head;
			node->prev = NULL;
		}
		head = node;
	}

	/// Add node to the back of the list
	static void push_back(Nodeptr &head, Nodeptr &tail, Type * node)
	{
		if( tail == NULL )
		{
			head = node;
			tail = node;
		}
		else
		{
			tail->next = node;
			node->next = NULL;
			node->prev = tail;
		}
		tail = node;
	}
};
//@}
}	// namespace frosttools

#ifdef FrostTools_Use_Asserts
#ifndef FROSTTOOLS_ASSERT
#define FROSTTOOLS_ASSERT
#ifndef _CrtDbgBreak
void _CrtDbgBreak()
{
    assert(0);
}
#endif
inline void frostAssert(const char *text,const char *file,int line)
{
	std::ofstream("asserts.log")<<file<<"("<<line<<")"<<" : "<<text<<std::endl;
	_CrtDbgBreak();
}
#ifdef assert
    #undef assert
    #define assert(expr) (!(expr))?frostAssert(#expr, __FILE__, __LINE__):(void)0;
#endif //assert
#endif //FROSTTOOLS_ASSERT
#endif //FrostTools_Use_Asserts
