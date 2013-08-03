#ifndef FROSTTOOLS_LOCKFREE
#define FROSTTOOLS_LOCKFREE
#include <memory>

#include <frosttools/threads.hpp>
#include "frosttools/gperftools/atomicops.h"


namespace LockFree
{
#ifdef _MSC_VER
	inline int32_t InterlockedEX(volatile int32_t * ptr, int32_t val)
	{	
		base::subtle::FastInterlockedExchange(ptr, val);
	}

	//inline int32_t InterlockedCAS
#else
	inline int32_t InterlockedEX(volatile int32_t * ptr, int32_t val)
	{		
		// gcc atomic, cast to void to prevent "value computed is not used"
		
		asm volatile("" ::: "memory");              // prevend compiler reordering
		return __sync_lock_test_and_set(ptr, val);
	}
#endif

	//! Double-lock FIFO queue implementation
	//! Should be empty before being destroyed, or you've get an assert
	//! Element should have a default constructor and 'next' pointer
	template <typename Node>
	class DLockQueue
	{
	  private:
		typedef Node * NodePtr;
		Node dummyNode;				//!< Dummy node for the head
		Node* head;
		Node* tail;
		Threading::mutex headLock, tailLock;
	  public:

		//! Constructor
		DLockQueue()
		{
			dummyNode.next = NULL;
			head = &dummyNode;
			tail = &dummyNode;
		}

		//! Destructor. Queue must be empty here to prevent memory leaks
		~DLockQueue()
		{
			assert(empty() == true);
		}

		bool empty() const
		{
			return head == tail;
		}

		//! Push new node to the list
		void push(Node * node)
		{
			node->next = NULL;
			tailLock.lock();
			tail->next = node;
			tail = node;
			tailLock.unlock();
		}

		//! Try to pop value from list
		//! Returns true if successful, false otherwise
		bool pop(NodePtr& result)
		{
			Node * ptr = NULL;
			headLock.lock();
			ptr = head->next;
			Node * new_next = ptr->next;
			if(ptr == NULL)	// list is already empty
			{
				headLock.unlock();
				return false;
			}
			head->next = new_next;
			headLock.unlock();
			ptr->next = NULL;
			result = ptr;
			return true;
		}
	};

	// This queue uses extarnal type with pointer "next"
	template <typename Node/*, typename _Alloc = std::allocator<Node>*/ >
	class BaseQueue
	{
	  private:
		typedef Node * NodePtr;
		Node* first;			                                     // for producer only
		Node* divider;                                               // shared
		Node* last;                                                  // shared
	  public:
		BaseQueue()
		{
		  first = new Node();//_Alloc::construct();
		  divider = first;
		  last = first;
		}

		~BaseQueue()
		{
		  while(first != NULL)                                   // release the list
		  {
			Node* tmp = first;
			first = tmp->next;
			delete tmp;
		  }
		}

		bool empty() const
		{
			return first == divider;
		}

		void produce(Node * node)
		{
		  last->next = node;                              // add the new item
		  InterlockedEX(&last, last->next);
		  //asm volatile("" ::: "memory");              // prevend compiler reordering
			// gcc atomic, cast to void to prevent "value computed is not used"
		  
		  //(void)__sync_lock_test_and_set(&last, last->next);
		  while(first != divider)                               // trim unused nodes
		  {
			Node* tmp = first;
			first = first->next;
		  }
		}

		bool consume(NodePtr& result)
		{
		  if(divider != last)                                // if queue is nonempty
		  {
			result = divider->next; 	                      // C: copy it back
			InterlockedEX(&divider, divider->next);
			/*
			asm volatile("" ::: "memory");            // prevend compiler reordering
				 // gcc atomic, cast to void to prevent "value computed is not used"
			
			(void)__sync_lock_test_and_set(&divider, divider->next);
			*/
			return true;          	                           // and report success
		  }
		  return false;           	                            // else report empty
		}
	};

	template <typename T>
	class lockfreequeue
	{
	  private:
		struct Node
		{
		  Node(T val) : value(val), next(NULL) { }
		  T value;
		  Node* next;
		};
		Node* first;			                                      // for producer only
		Node* divider;                                                     // shared
		Node* last;                                                        // shared

	  public:
		lockfreequeue()
		{
		  first = new Node(T());
		  divider = first;
		  last = first;
		}

		~lockfreequeue()
		{
		  while(first != NULL)                                   // release the list
		  {
			Node* tmp = first;
			first = tmp->next;
			delete tmp;
		  }
		}

		void produce(const T& t)
		{
		  last->next = new Node(t);                              // add the new item
		  asm volatile("" ::: "memory");              // prevend compiler reordering
				 // gcc atomic, cast to void to prevent "value computed is not used"
		  (void)__sync_lock_test_and_set(&last, last->next);
		  while(first != divider)                               // trim unused nodes
		  {
			Node* tmp = first;
			first = first->next;
			delete tmp;
		  }
		}

		bool consume(T& result)
		{
		  if(divider != last)                                // if queue is nonempty
		  {
			result = divider->next->value; 	                      // C: copy it back
			asm volatile("" ::: "memory");            // prevend compiler reordering
				 // gcc atomic, cast to void to prevent "value computed is not used"
			(void)__sync_lock_test_and_set(&divider, divider->next);
			return true;          	                           // and report success
		  }
		  return false;           	                            // else report empty
		}
	};
}
#endif
