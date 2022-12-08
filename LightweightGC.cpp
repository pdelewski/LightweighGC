#include <iostream>
#include "ptr.h"

struct node : public ucore::resource
{
  int value;
  node() { value = 0; }
  node(int v):value(v) {}
  ucore::gen_ptr<node> next;
  void dump() { std::cout << "node:" << value << std::endl;}
};

void traverse(const ucore::gen_ptr<node>& n)
{
  std::set<node*> visited;
  auto current = ucore::make_alias<node>();
  current = n;
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
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  auto last = ucore::make_owning_ptr(new node(2), __FILE__, __LINE__);

  assert(last.is_owner() == true);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  head->next.move_ownership_from(last);

  traverse(head);
}

void test2()
{
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  head->next.move_ownership_from(head);
  
  traverse(head);

  head->next = nullptr;
}

void test3()
{
  auto alias = ucore::make_alias<node>(nullptr,  __FILE__, __LINE__);
  auto root = ucore::make_owning_ptr(new node(1),  __FILE__, __LINE__);
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
