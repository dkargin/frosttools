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
