#include <iostream>
#include <cassert>
#include <set>

struct resource 
{
  resource():counter(0) {}
  int counter;
};

template<typename T>
struct smart_ptr
{
  smart_ptr():owner(true), ptr(nullptr) {}
  explicit smart_ptr(bool owner, T *p) :owner(owner), ptr(p)
  {
    if (ptr && !owner)
    {
      ++ptr->counter;
    }
  }
  
  smart_ptr(const smart_ptr& rhs)
  {
    ptr = rhs.ptr;
    if (ptr && !owner)
    {
      ++ptr->counter;
    }
  }
  
  smart_ptr& operator=(const smart_ptr& rhs)
  {
    if (&rhs == this) {
      return *this;
    }

    assert ((owner && rhs.is_owner())==false);

    if (ptr && !owner) {
      --ptr->counter;
    }
    ptr = rhs.ptr;
    if (ptr && !owner) {
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
    if (ptr && !owner) {
      --ptr->counter;
    }
    if (owner) {
      ptr->dump();
      delete ptr;
    }
  }

  T* operator->() {
    return ptr;
  }

  operator bool() {
    return ptr != 0;
  }
    
  bool is_owner() const
  {
    return owner;
  }

  void set_owner()
  {
    owner = true;
  }
  void clear_owner()
  {
    owner = false;
  }
private:
  bool owner;
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
  std::set<node*> visited;
  smart_ptr<node> current = n;

  while (current) {
    current.clear_owner();
    if (visited.find(current.operator->()) != visited.end())
    {
      current = smart_ptr<node>(false,nullptr);
      continue;
    }
    std::cout << current->value << std::endl;
    visited.insert(current.operator->());
    current = current->next;
  }
}

void test1()
{
  smart_ptr<node> last (true,new node(2));
  smart_ptr<node> head (true,new node(1));

  assert(last.is_owner() == true);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);
  last.clear_owner();
  head->next = last;

  traverse(head);
}

void test2()
{
  smart_ptr<node> head (true,new node(1));

  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);
  head->next.clear_owner();
  head->next = head;
  
  traverse(head);
}

int main()
{
  test1();
  test2();
  return 0;
}
