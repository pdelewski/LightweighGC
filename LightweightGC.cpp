#include <iostream>
#include <cassert>
#include <set>

enum {OWNER, ALIAS};

struct resource 
{
  resource():counter(0) {}
  int counter;
};

template<typename T>
struct smart_ptr
{
  smart_ptr():owner(true), ptr(nullptr) {}
  smart_ptr(T* p):owner(false), ptr(p) {}
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
      assert(ptr->counter == 0);
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

  void set_ownership()
  {
    owner = true;
    if (ptr) {
      --ptr->counter;
    }
  }
  void clear_ownership()
  {
    owner = false;
    if (ptr) {
      ++ptr->counter;
    }
  }

  void take_ownership(smart_ptr<T>& rhs)
  {
    clear_ownership();
    *this = rhs;
  }
  int alias_counter() const {
    return ptr->counter;
  }
private:
  bool owner;
  T* ptr;
};

template<typename T>
auto make_owning_ptr(T *ptr) -> smart_ptr<T>
{
  return smart_ptr<T>(true, ptr);
}

template<typename T>
auto make_alias(T *ptr) -> smart_ptr<T>
{
  return smart_ptr<T>(false, ptr);
}

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
  auto current = n;
  assert(current.is_owner() == false);
  while (current) {

    if (visited.find(current.operator->()) != visited.end())
    {
      current = nullptr;
      continue;
    }
    std::cout << current->value << std::endl;
    visited.insert(current.operator->());
    current = current->next;
  }
}

void test1()
{
  auto head = make_owning_ptr(new node(1));
  auto last = make_owning_ptr(new node(2));

  assert(last.is_owner() == true);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  last.clear_ownership();
  head->next = last;
  //head->next.take_ownership(last);

  traverse(head);
}

void test2()
{
  auto head = make_owning_ptr(new node(1));
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);
  head->next.clear_ownership();
  head->next = head;
  //head->next.take_ownership(head);
  
  traverse(head);

  head->next = nullptr;
}

void test3()
{
  smart_ptr<node> alias;
  auto root = make_owning_ptr(new node(1));
  alias.clear_ownership();
  alias = root;
  assert(alias.alias_counter() == 1);
  alias = nullptr;
}

int main()
{
  test1();
  test2();
  test3();
  return 0;
}
