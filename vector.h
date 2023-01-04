#pragma once

#include "ptr.h"

namespace ucore {

template <typename T>
struct vector {
  vector() { buffer = ucore::make_owning_ptr<T>(T()); }

 private:
  gen_ptr<T> buffer;
};

}  // namespace ucore
