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

void traverse(const ucore::gen_ptr<node>& n, int expected_size)
{
  std::set<node*> visited;
  auto current = ucore::make_alias<node>(); current.with_source_location(__FILE__,__LINE__);
  current.with_source_location(__FILE__,__LINE__); current = n;
  auto counter = 0;
  assert(current.is_owner() == false);
  while (current) {
    if (visited.find(current.operator->()) != visited.end())
    {
      current = nullptr;
      continue;
    }
    visited.insert(current.operator->());
    current.with_source_location(__FILE__,__LINE__); current = current->next;
    ++counter;
  }
  assert(counter == expected_size);
}

void test1()
{
  auto head = ucore::make_owning_ptr(new node(1)); head.with_source_location(__FILE__, __LINE__);
  auto last = ucore::make_owning_ptr(new node(2)); last.with_source_location(__FILE__, __LINE__);

  assert(last.is_owner() == true);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  head->next.move_ownership_from(last);

  traverse(head,2);
}

void test2()
{
  auto head = ucore::make_owning_ptr(new node(1));  head.with_source_location(__FILE__, __LINE__);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  head->next.move_ownership_from(head);
  
  traverse(head,1);

  head->next = nullptr;
}

void test3()
{
  auto alias = ucore::make_alias<node>(); alias.with_source_location(__FILE__, __LINE__);
  auto root = ucore::make_owning_ptr(new node(1)); root.with_source_location(__FILE__, __LINE__);
  alias = root; alias.with_source_location(__FILE__, __LINE__);
  assert(alias.alias_counter() == 1);
  alias = nullptr;
}

void sink(ucore::gen_ptr<node> n) {}

void test4()
{
  auto head = ucore::make_owning_ptr(new node(1)); head.with_source_location(__FILE__, __LINE__);
  auto alias = ucore::make_alias<node>(); alias.with_source_location(__FILE__, __LINE__);

  alias.with_source_location(__FILE__, __LINE__); sink(alias);
}


void test5()
{
  auto head = ucore::make_owning_ptr(new node(1)); head.with_source_location(__FILE__, __LINE__);
  auto alias = ucore::make_alias<node>(); alias.with_source_location(__FILE__, __LINE__);

  alias.with_source_location(__FILE__, __LINE__); sink(std::move(head));
  head = nullptr;
}

int main()
{
  test1();
  test2();
  test3();
  test4();
  test5();
  return 0;
}
