#pragma once

#include <cstdint>

#include "resource.h"

namespace ucore {

struct int_8 : public resource {
  int_8(int8_t s = 0) : storage(s) {}
  operator int8_t() { return storage; }

 private:
  int8_t storage;
};

struct int_16 : public resource {
  int_16(int16_t s = 0) : storage(s) {}
  operator int16_t() { return storage; }

 private:
  int16_t storage;
};

struct int_32 : public resource {
  int_32(int32_t s = 0) : storage(s) {}
  operator int32_t() { return storage; }

 private:
  int32_t storage;
};

}  // namespace ucore