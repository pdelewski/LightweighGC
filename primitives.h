#pragma once

#include <cstdint>

#include "resource.h"

namespace ucore {

struct int_8 : public resource {
  int_8(int8_t s = 0) : storage(s) {}
  int8_t storage;
};

}  // namespace ucore