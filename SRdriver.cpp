#include <iostream>
#include <set>

#include "primitives.h"
#include "ptr.h"

struct node : public ucore::resource {
  int value;
  node() { value = 0; }
  node(int v) : value(v) {}
  ucore::gen_ptr<node> next = ucore::gen_ptr<node>(__FILE__, __LINE__);
  void dump() { std::cout << "node:" << value << std::endl; }
};

void traverse(const ucore::gen_ptr<node>& n, int expected_size) {
  std::set<ucore::gen_ptr<node>> visited;
  auto current = ucore::make_alias<ucore::gen_ptr<node>>(__FILE__, __LINE__);
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
  assert(counter == expected_size);
}

void test1() {
  // singly linked list
  auto head = ucore::make_owning_ptr<node>(1, __FILE__, __LINE__);
  auto last = ucore::make_owning_ptr<node>(2, __FILE__, __LINE__);
  assert(last.is_owner() == true);
  assert(head.is_owner() == true);
  head->next = std::move(last);
  traverse(head, 2);
}

void test2() {
  // circular list
  auto head = ucore::make_owning_ptr<node>(1, __FILE__, __LINE__);
  assert(head.is_owner() == true);
  assert(head->next.is_owner() == false);
  head->next = head;

  traverse(head, 1);

  assert(head.alias_counter() == 1);

  head->next.release();
}

void test3() {
  // two aliases
  auto alias = ucore::make_alias<ucore::gen_ptr<node>>(__FILE__, __LINE__);
  auto alias2 = ucore::make_alias<ucore::gen_ptr<node>>(__FILE__, __LINE__);
  auto root = ucore::make_owning_ptr<node>(1, __FILE__, __LINE__);
  alias.with_source_location(__FILE__, __LINE__) = root;
  alias2.with_source_location(__FILE__, __LINE__) = root;

  assert(alias.alias_counter() == 2);
  alias.with_source_location(__FILE__, __LINE__).release();
  alias2.with_source_location(__FILE__, __LINE__).release();
}

void sink(ucore::gen_ptr<node> n) {}

void test4() {
  // function call with copy
  auto alias = ucore::make_alias<ucore::gen_ptr<node>>(__FILE__, __LINE__);
  auto head = ucore::make_owning_ptr<node>(1, __FILE__, __LINE__);
  alias.with_source_location(__FILE__, __LINE__) = head;
  sink(alias.with_source_location(__FILE__, __LINE__));
  alias.with_source_location(__FILE__, __LINE__).release();
}

void test5() {
  // move semantics
  auto alias = ucore::make_alias<ucore::gen_ptr<node>>(__FILE__, __LINE__);
  auto head = ucore::make_owning_ptr<node>(1, __FILE__, __LINE__);
  alias.with_source_location(__FILE__, __LINE__) = head;
  alias.with_source_location(__FILE__, __LINE__).release();
  sink(std::move(head.with_source_location(__FILE__, __LINE__)));
}

void test6() {
  // alias to an alias
  auto head = ucore::make_owning_ptr<node>(1, __FILE__, __LINE__);
  auto alias1 = ucore::make_alias<ucore::gen_ptr<node>>(__FILE__, __LINE__);
  auto alias2 = ucore::make_alias<ucore::gen_ptr<node>>(__FILE__, __LINE__);
  alias1.with_source_location(__FILE__, __LINE__) = head;
  alias2.with_source_location(__FILE__, __LINE__) = alias1;
}

void test7() {
  using namespace ucore;
  auto initialintptr = make_owning_ptr<int_8>(1, __FILE__, __LINE__);
  auto ptr = make_owning_ptr<gen_ptr<int_8>>(
      make_owning_ptr<int_8>(std::move(initialintptr)), __FILE__, __LINE__);

  auto intptr = make_owning_ptr<int_8>(2, __FILE__, __LINE__);
  (*ptr).move_ownership_from(intptr);
  assert((**ptr) == 2);
}

void test8() {
  using namespace ucore;
  // distinguishing heap and stack address is needed
  // to prevent creating more than one owner
  {
    gen_ptr<int_8> p = gen_ptr<int_8>(OWNER, new int_8(1));
    gen_ptr<gen_ptr<int_8>> pp = gen_ptr<gen_ptr<int_8>>(ALIAS, &p);
  }
  {
    auto alias = make_alias<gen_ptr<gen_ptr<int_8>>>();
    gen_ptr<int_8> p =
        make_owning_ptr<int_8>(1);  // gen_ptr<int_8>(OWNER, new int_8(1));
    gen_ptr<gen_ptr<int_8>> pp = make_owning_ptr<gen_ptr<int_8>>(
        make_owning_ptr<int_8>(1) /*std::move(p)*/);
    alias = pp;
    alias.release();
    // auto alias = make_alias(pp);
    //     gen_ptr<gen_ptr<int_8>>(OWNER, new gen_ptr<int_8>(std::move(p)));
  }
}

void test9() {
  using namespace ucore;
  using p_int8 = gen_ptr<int_8>;
  using pp_int8 = gen_ptr<gen_ptr<int_8>>;
  p_int8 p(OWNER, new int_8(1));
  pp_int8 p2(ALIAS, &p);
  assert(** p2 = 1);
}

void test10() {
  using namespace ucore;
  auto p = make_owning_ptr<int_8>(10);
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
  test9();
  test10();
  return 0;
}
