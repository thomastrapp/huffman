#ifndef HLP_TESTING_TYPES_H
#define HLP_TESTING_TYPES_H

#include <cstdint>
#include "gtest/gtest.h"

namespace hlp {

typedef ::testing::Types<
  uint8_t, int8_t,
  uint16_t, int16_t,
  uint32_t, int32_t,
  uint64_t, int64_t
> testing_types;


}

#endif // HLP_TESTING_TYPES_H

