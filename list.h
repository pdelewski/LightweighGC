#pragma once

#include <iostream>

#include "ptr.h"

namespace ucore {

template <typename T>
struct list {
 private:
  template <typename U>
  struct node : public ucore::resource {
    U value;
    node() { value = 0; }
    node(U v) : value(v) {}
    ucore::gen_ptr<node<U>> next;
  };

 public:
  void push_front(const T& v) {
    if (!head) {
      head = ucore::make_owning_ptr<node<T>>(v);
    } else {
      auto p = ucore::make_owning_ptr<node<T>>(v);
      p->next = std::move(head);
      head = std::move(p);
    }
  }

  void traverse() {
    auto current = ucore::make_alias<ucore::gen_ptr<node<T>>>();

    current = head;

    while (current) {
      std::cout << current->value << std::endl;
      current = current->next;
    }
  }

 private:
  ucore::gen_ptr<node<T>> head;
};

}  // namespace ucore
