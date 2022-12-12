#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <stdexcept>
#include <utility>

#include "resource.h"

namespace ucore {
enum ptr_ownersip_property { ALIAS = 0, OWNER = 1 };

struct instruction {
  std::string op;
  std::string file;
  size_t line;
};

struct rule_break_exception : public std::runtime_error {
  rule_break_exception(const std::string& what) : std::runtime_error(what) {}
};

template <typename T>
struct gen_ptr : public resource {
  gen_ptr(const std::string& file = std::string("undefined"),
          const size_t line = 0)
      : ownership(OWNER), ptr(nullptr) {
    init_source_location(file, line);
  }
  gen_ptr(T* p, const std::string& file = std::string("undefined"),
          const size_t line = 0)
      : ownership(ALIAS), ptr(p) {
    init_source_location(file, line);
  }
  explicit gen_ptr(ptr_ownersip_property owner, T* p,
                   const std::string& file = std::string("undefined"),
                   const size_t line = 0)
      : ownership(owner), ptr(p), file(file) {
    if (ptr && !is_owner()) {
      ++ptr->counter;
    }
    init_source_location(file, line);
  }

  gen_ptr(gen_ptr&& rhs)
      : ownership(std::move(rhs.ownership)),
        ptr(std::move(rhs.ptr)),
        file(std::move(rhs.file)),
        line(std::move(rhs.line)) {
#ifdef SAFE_REF_THROW_EXCEPTIONS
#endif
#ifdef DEBUG
    if (ptr->counter != 0) {
      std::cout << "move is not allowed in case of live aliases" << std::endl;
      dump_aliases();
    }
    assert(ptr->counter == 0);
#endif
    rhs.ownership = ALIAS;
    rhs.ptr = nullptr;
    init_source_location(file, line);
  }

  gen_ptr& operator=(gen_ptr&& rhs) {
    remove_source_location();
    ownership = std::move(rhs.ownership);
    ptr = std::move(rhs.ptr);
    file = std::move(rhs.file);
    line = std::move(rhs.line);
    move_source_location();
    return *this;
  }

  gen_ptr(const gen_ptr& rhs) {
    assert(!is_owner());
    ptr = rhs.ptr;
    ownership = rhs.ownership;
    file = rhs.file;
    line = rhs.line;
    init_source_location(file, line);
    if (ptr && !is_owner()) {
      ++ptr->counter;
    }
  }

  gen_ptr& operator=(const gen_ptr& rhs) {
    if (&rhs == this) {
      return *this;
    }
    remove_source_location();
    assert((is_owner() && rhs.is_owner()) == false);
    if (ptr && !is_owner()) {
      --ptr->counter;
    }

    ptr = rhs.ptr;

    if (ptr && !is_owner()) {
      ++ptr->counter;
    }
    move_source_location();
    return *this;
  }

  ~gen_ptr() {
    remove_source_location();
    if (!is_owner() && ptr) {
      // rule alias has to be nullptr during destruction
      std::cout << "each alias has to be null during destruction : " << file
                << "," << line << std::endl;
      assert(ptr == nullptr);
    }

    if (ptr && is_owner()) {
      dump_all_references();
      assert(ptr->counter == 0);
      delete ptr;
    }
  }

  T* operator->() { return ptr; }

  T& operator*() { return *ptr; }

  bool operator<(const gen_ptr& rhs) const { return ptr < rhs.ptr; }

  operator bool() { return ptr != 0; }

  gen_ptr& with_source_location(const std::string& f, size_t l) const {
#ifdef DEBUG
    file = f;
    line = l;
#endif
    return const_cast<gen_ptr&>(*this);
  }

  bool is_owner() const { return ownership == OWNER; }

  gen_ptr& convert_to_alias() {
    ownership = ALIAS;
    if (ptr) {
      ++ptr->counter;
    }
    return *this;
  }

  void move_ownership_from(gen_ptr<T>& rhs,
                           const std::string& file = std::string("undefined"),
                           const size_t line = 0) {
    rhs.convert_to_alias();
    *this = rhs;
  }

  int alias_counter() const { return ptr->counter; }

  void release() const {
    remove_source_location();
    if (ptr && !is_owner()) {
      --ptr->counter;
    }
    ownership = ALIAS;
    ptr = nullptr;
    file = std::string();
    line = 0;
  }

 private:
  void init_source_location(const std::string& file, size_t line) {
#ifdef DEBUG
    this->file = file;
    this->line = line;
    if (ptr) {
      if (is_owner()) {
        ptr->owner_location = std::make_pair(file, line);
      } else {
        ptr->aliases_locations.insert(
            std::make_pair((size_t)this, std::make_pair(file, line)));
      }
    }
#endif
  }
  void remove_source_location() const {
#ifdef DEBUG
    if (ptr) {
      auto it = ptr->aliases_locations.find(size_t(this));
      if (it != ptr->aliases_locations.end()) {
        ptr->aliases_locations.erase(it);
      }
    }
#endif
  }
  void move_source_location() {
#ifdef DEBUG
    if (ptr) {
      if (is_owner()) {
        ptr->owner_location = std::make_pair(file, line);
      } else {
        ptr->aliases_locations.insert(
            std::make_pair((size_t)this, std::make_pair(file, line)));
      }
    }
#endif
  }
  void dump_aliases() {
#ifdef DEBUG
    if (ptr->counter != 0) {
      for (const auto& alias : ptr->aliases_locations) {
        std::cout << "alias : " << alias.second.first << ":"
                  << alias.second.second << std::endl;
      }
    }
#endif
  }
  void dump_all_references() {
#ifdef DEBUG
    if (ptr->counter != 0) {
      std::cout << "owner : " << ptr->owner_location.first << ":"
                << ptr->owner_location.second << std::endl;
      for (const auto& alias : ptr->aliases_locations) {
        std::cout << "alias : " << alias.second.first << ":"
                  << alias.second.second << std::endl;
      }
    }
#endif
  }
  mutable ptr_ownersip_property ownership;
  mutable T* ptr;
#ifdef DEBUG
  mutable std::string file;
  mutable size_t line;
  mutable std::list<instruction> instructions_log;
#endif
};

template <typename T>
auto make_owning_ptr(T* ptr, const std::string& file = std::string(),
                     const size_t line = 0) -> gen_ptr<T> {
  return gen_ptr<T>(OWNER, ptr, file, line);
}

template <typename T>
auto make_alias(T* ptr = nullptr, const std::string& file = std::string(),
                const size_t line = 0) -> gen_ptr<T> {
  return gen_ptr<T>(ALIAS, ptr, file, line);
}
}  // namespace ucore
