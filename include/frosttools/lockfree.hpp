#ifndef FROSTTOOLS_LOCKFREE
#define FROSTTOOLS_LOCKFREE
#include <memory>

#include "frosttools/gperftools/atomicops.h"


namespace LockFree
{
	// This queue uses extarnal type with pointer "next"
	template <typename Node, typename _Alloc = std::allocator<Node> >
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
		  first = _Alloc::construct();
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

		void produce(Node * node)
		{
		  last->next = node;                              // add the new item
		  asm volatile("" ::: "memory");              // prevend compiler reordering
				 // gcc atomic, cast to void to prevent "value computed is not used"
		  (void)__sync_lock_test_and_set(&last, last->next);
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
			asm volatile("" ::: "memory");            // prevend compiler reordering
				 // gcc atomic, cast to void to prevent "value computed is not used"
			(void)__sync_lock_test_and_set(&divider, divider->next);
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
