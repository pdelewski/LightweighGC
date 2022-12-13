#include <iostream>
#include <set>

#include "primitives.h"
#include "ptr.h"

struct node : public ucore::resource {
  int value;
  node() { value = 0; }
  node(int v) : value(v) {}
  ucore::gen_ptr<node> next =
      ucore::gen_ptr<node>(ucore::OWNER, nullptr, __FILE__, __LINE__);
  void dump() { std::cout << "node:" << value << std::endl; }
};

void traverse(const ucore::gen_ptr<node>& n, int expected_size) {
  std::set<ucore::gen_ptr<node>> visited;
  auto current = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  current.with_source_location(__FILE__, __LINE__) = n;
  auto counter = 0;
  assert(current.is_owner() == false);
  while (current) {
    if (visited.find(current.with_source_location(__FILE__, __LINE__)) !=
        visited.end()) {
      break;
    }
    visited.insert(current.with_source_location(__FILE__, __LINE__));

    current.with_source_location(__FILE__, __LINE__) = current->next;
    ++counter;
  }
  for (auto it = visited.begin(); it != visited.end(); it++) {
    it->with_source_location(__FILE__, __LINE__).release();
  }
  current.with_source_location(__FILE__, __LINE__).release();
  assert(counter == expected_size);
}

void test1() {
  // singly linked list
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  auto last = ucore::make_owning_ptr(new node(2), __FILE__, __LINE__);

  assert(last.is_owner() == true);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  head->next.move_ownership_from(last, __FILE__, __LINE__);

  traverse(head, 2);
  last.with_source_location(__FILE__, __LINE__).release();
}

void test2() {
  // circular list
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == true);

  // head->next.move_ownership_from(head);
  head->next.convert_to_alias();
  head->next = head;

  traverse(head, 1);

  assert(head.alias_counter() == 1);

  head->next.release();
}

void test3() {
  // two aliases
  auto alias = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  auto alias2 = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  auto root = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  alias.with_source_location(__FILE__, __LINE__) = root;
  alias2.with_source_location(__FILE__, __LINE__) = root;

  assert(alias.alias_counter() == 2);
  alias.with_source_location(__FILE__, __LINE__).release();
  alias2.with_source_location(__FILE__, __LINE__).release();
}

void sink(ucore::gen_ptr<node> n) { n.release(); }
void sink2(ucore::gen_ptr<node> n) {}

void test4() {
  // function call with copy
  auto alias = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  alias.with_source_location(__FILE__, __LINE__) = head;
  sink(alias.with_source_location(__FILE__, __LINE__));
  alias.with_source_location(__FILE__, __LINE__).release();
}

void test5() {
  // move semantics
  auto alias = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  alias.with_source_location(__FILE__, __LINE__) = head;
  alias.with_source_location(__FILE__, __LINE__).release();
  sink2(std::move(head.with_source_location(__FILE__, __LINE__)));
}

void test6() {
  // alias to an alias
  auto head = ucore::make_owning_ptr(new node(1), __FILE__, __LINE__);
  auto alias1 = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  auto alias2 = ucore::make_alias<node>(nullptr, __FILE__, __LINE__);
  alias1.with_source_location(__FILE__, __LINE__) = head;
  alias2.with_source_location(__FILE__, __LINE__) = alias1;
  alias1.with_source_location(__FILE__, __LINE__).release();
  alias2.with_source_location(__FILE__, __LINE__).release();
}

void test7() {
  using namespace ucore;
  auto intptr = make_owning_ptr(new int_8(2), __FILE__, __LINE__);
  auto initialintptr = make_owning_ptr(new int_8(1), __FILE__, __LINE__);
  auto ptr = make_owning_ptr(new gen_ptr<int_8>(std::move(initialintptr)),
                             __FILE__, __LINE__);

  (*ptr).move_ownership_from(intptr);
  assert((**ptr) == 2);
  intptr.release();
}

void test8() {
  using namespace ucore;
  // distinguishing heap and stack address is needed
  // to prevent creating more than one owner
  gen_ptr<int_8> p = gen_ptr<int_8>(OWNER, new int_8(1));
  gen_ptr<gen_ptr<int_8>> pp = gen_ptr<gen_ptr<int_8>>(ALIAS, &p);
  pp.release();
}

int main() {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
  test8();
  return 0;
}
