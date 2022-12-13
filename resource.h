#pragma once

#include <map>
#include <utility>

namespace ucore {
struct resource {
  resource() : counter(0) {}
  int counter;
#ifdef DEBUG
  void *operator new(size_t s) {
    void *ptr = ::operator new(s);
    heap_addresses.insert((size_t)ptr);
    return ptr;
  }
  void operator delete(void *p) {
    auto it = heap_addresses.find((size_t)p);
    if (it != heap_addresses.end()) {
      heap_addresses.erase(it);
    }
    return ::operator delete(p);
  }
  std::map<size_t, std::pair<std::string, size_t>> aliases_locations;
  std::pair<std::string, size_t> owner_location;
  static std::set<size_t> heap_addresses;
#endif
};

std::set<size_t> resource::heap_addresses;

}  // namespace ucore
