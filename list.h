#pragma once

#include <iostream>

#include "ptr.h"

namespace ucore {

template <typename T>
struct list : public resource {
 private:
  template <typename U>
  struct node;

  template <typename U>
  using ptr = ucore::gen_ptr<node<U>>;

  template <typename U>
  struct node : public ucore::resource {
    U value;
    node() { value = 0; }
    node(U v) : value(v) {}
    ptr<U> next;
  };

 public:
  struct iterator {
    iterator() {}
    iterator(const ptr<T>& p_) { p = p_; }
    iterator& operator++() {
      if (p) {
        p = p->next;
      }
      return *this;
    }
    bool operator==(const iterator& rhs) { return p == rhs.p; }
    bool operator!=(const iterator& rhs) { return p != rhs.p; }
    T& operator*() { return p->value; }

   private:
    ptr<T> p;
  };

  void push_front(const T& v) {
    if (!head) {
      head = ucore::make_owning_ptr<node<T>>(v);
    } else {
      auto p = ucore::make_owning_ptr<node<T>>(v);
      p->next = std::move(head);
      head = std::move(p);
    }
  }

  iterator begin() {
    iterator i(head);
    return i;
  }

  iterator end() {
    iterator i;
    return i;
  }

 private:
  ptr<T> head;
};

}  // namespace ucore
