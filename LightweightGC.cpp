#include <iostream>
#include <set>

#include "ptr.h"

struct node : public ucore::resource {
  int value;
  node() { value = 0; }
  node(int v) : value(v) {}
  ucore::gen_ptr<node> next;
  void dump() { std::cout << "node:" << value << std::endl; }
};

void traverse(const ucore::gen_ptr<node>& n, int expected_size) {
  std::set<ucore::gen_ptr<node>> visited;
  auto current = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  current.with_source_location(__FILE__, __LINE__);
  current = n;
  auto counter = 0;
  assert(current.is_owner() == false);
  while (current) {
    if (visited.find(current) != visited.end()) {
      break;
    }
    current.with_source_location(__FILE__, __LINE__);
    visited.insert(current);

    current.with_source_location(__FILE__, __LINE__);
    current = current->next;
    ++counter;
  }
  for (auto it = visited.begin(); it != visited.end(); it++) {
    it->release();
  }
  current.release();
  assert(counter == expected_size);
}

void test1() {
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  auto last = ucore::make_owning_ptr(new node(2), __FILE__, __LINE__);

  assert(last.is_owner() == true);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  head->next.move_ownership_from(last);

  traverse(head, 2);
  last = nullptr;
}

void test2() {
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  // head->next.move_ownership_from(head);
  head->next.convert_to_alias();
  head->next = head;

  traverse(head, 1);

  assert(head.alias_counter() == 1);

  head->next = nullptr;
}

void test3() {
  auto alias = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  auto alias2 = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  auto root = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  alias.with_source_location(__FILE__, __LINE__);
  alias = root;
  alias2.with_source_location(__FILE__, __LINE__);
  alias2 = root;

  assert(alias.alias_counter() == 2);
  alias.with_source_location(__FILE__, __LINE__);
  alias = nullptr;
  alias2.with_source_location(__FILE__, __LINE__);
  alias2 = nullptr;
}

void sink(ucore::gen_ptr<node> n) { n = nullptr; }

void test4() {
  auto alias = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  alias = head;
  sink(alias);
  alias = nullptr;
}

void test5() {
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  sink(std::move(head));
  head = nullptr;
}

int main() {
  test1();
  test2();
  test3();
  test4();
  test5();
  return 0;
}
