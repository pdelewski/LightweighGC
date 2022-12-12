#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <stdexcept>
#include <utility>

namespace ucore {
enum ptr_ownersip_property { OWNER, ALIAS };

struct instruction {
  std::string op;
  std::string file;
  size_t line;
};

struct rule_break_exception : public std::runtime_error {
  rule_break_exception(const std::string& what) : std::runtime_error(what) {}
};

struct resource {
  resource() : counter(0) {}
  int counter;
#ifdef DEBUG
  std::map<size_t, std::pair<std::string, size_t>> aliases_locations;
  std::pair<std::string, size_t> owner_location;
#endif
};

template <typename T>
struct gen_ptr {
  gen_ptr(const std::string& file = std::string("undefined"),
          const size_t line = 0)
      : owner(true), ptr(nullptr) {
    init_source_location(file, line);
  }
  gen_ptr(T* p, const std::string& file = std::string("undefined"),
          const size_t line = 0)
      : owner(false), ptr(p) {
    init_source_location(file, line);
  }
  explicit gen_ptr(bool owner, T* p,
                   const std::string& file = std::string("undefined"),
                   const size_t line = 0)
      : owner(owner), ptr(p), file(file) {
    if (ptr && !owner) {
      ++ptr->counter;
    }
    init_source_location(file, line);
  }

  gen_ptr(gen_ptr&& rhs)
      : owner(std::move(rhs.owner)),
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
    rhs.owner = false;
    rhs.ptr = nullptr;
    init_source_location(file, line);
  }

  gen_ptr& operator=(gen_ptr&& rhs) {
    remove_source_location();
    owner = std::move(rhs.owner);
    ptr = std::move(rhs.ptr);
    file = std::move(rhs.file);
    line = std::move(rhs.line);
    move_source_location();
    return *this;
  }

  gen_ptr(const gen_ptr& rhs) {
    assert(!rhs.owner);
    ptr = rhs.ptr;
    owner = rhs.owner;
    file = rhs.file;
    line = rhs.line;
    init_source_location(file, line);
    if (ptr && !owner) {
      ++ptr->counter;
    }
  }

  gen_ptr& operator=(const gen_ptr& rhs) {
    if (&rhs == this) {
      return *this;
    }
    remove_source_location();
    assert((owner && rhs.is_owner()) == false);
    if (ptr && !owner) {
      --ptr->counter;
    }

    ptr = rhs.ptr;

    if (ptr && !owner) {
      ++ptr->counter;
    }
    move_source_location();
    return *this;
  }

  ~gen_ptr() {
    remove_source_location();
    if (!owner && ptr) {
      // rule alias has to be nullptr during destruction
      std::cout << "each alias has to be null during destruction : " << file
                << "," << line << std::endl;
      assert(ptr == nullptr);
    }

    if (ptr && owner) {
      dump_all_references();
      assert(ptr->counter == 0);
      delete ptr;
    }
  }

  T* operator->() { return ptr; }

  bool operator<(const gen_ptr& rhs) const { return ptr < rhs.ptr; }

  operator bool() { return ptr != 0; }

  gen_ptr& with_source_location(const std::string& f, size_t l) const {
#ifdef DEBUG
    file = f;
    line = l;
#endif
    return const_cast<gen_ptr&>(*this);
  }

  bool is_owner() const { return owner; }

  void convert_to_alias() {
    owner = false;
    if (ptr) {
      ++ptr->counter;
    }
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
    if (ptr && !owner) {
      --ptr->counter;
    }
    owner = false;
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
      if (owner) {
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
      if (owner) {
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
  mutable bool owner;
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
  return gen_ptr<T>(true, ptr, file, line);
}

template <typename T>
auto make_alias(T* ptr = nullptr, const std::string& file = std::string(),
                const size_t line = 0) -> gen_ptr<T> {
  return gen_ptr<T>(false, ptr, file, line);
}
}  // namespace ucore
