#include <vector>
#include <cstdint>

#include "gtest/gtest.h"

#include "hlp/byte-vector-to-entity.h"


namespace {

TEST(HlpByteVectorToEntity, ConvertsToEntity)
{
  std::vector<uint8_t> bytes {1,2,3,4,5,6,7,8};
  uint64_t entity = ::hlp::byte_vector_to_entity<uint64_t>(bytes);
  // violating strict aliasing rule
  uint8_t * byte_wise = reinterpret_cast<uint8_t *>(&entity);
  for(unsigned int i = 0; i < sizeof(uint64_t); ++i)
  {
    EXPECT_EQ(bytes.at(i), byte_wise[i]);
  }
}


}
