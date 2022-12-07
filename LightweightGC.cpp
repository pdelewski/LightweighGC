#include <iostream>

struct resource 
{
  resource():counter(0) {}
  int counter;
};

template<typename T>
struct smart_ptr
{
  explicit smart_ptr(T *p = 0) :ptr(p) 
  {
    if (ptr) 
    {
      ++ptr->counter;
    }
  }
  
  smart_ptr(const smart_ptr& rhs)
  {
    ptr = rhs.ptr;
    if (ptr)
    {
      ++ptr->counter;
    }
  }
  
  smart_ptr& operator=(const smart_ptr& rhs)
  {
    if (&rhs == this) {
      return *this;
    }
    if (ptr) {
      --ptr->counter;
    }
    ptr = rhs.ptr;
    if (ptr) {
      ++ptr->counter;
    }
    return *this;
  }
  
  ~smart_ptr()
  {
    if (!ptr) 
    {
      return;
    }
    if (--ptr->counter == 0) {
      delete ptr;
    }
  }

  T* operator->() {
    return ptr;
  }

  operator bool() {
    return ptr != 0;
  }
    
private:
  T* ptr;
};


struct node : public resource
{
  int value;
  node() { value = 0; }
  node(int v):value(v) {}
  smart_ptr<node> next;
  void dump() { std::cout << "node:" << value << std::endl;}
};

void traverse(const smart_ptr<node>& n)
{
  smart_ptr<node> current = n;
  while (current) {
    std::cout << current->value << std::endl;
    current = current->next;
  }
}

int main() {
  smart_ptr<node> last (new node(2));
  smart_ptr<node> head (new node(1));
 
  head->next = last;
  
  traverse(head);
  return 0;
}
