#include <vector>
#include <cstdint>
#include <iterator>

#include "gtest/gtest.h"

#include "hm/encode.h"


namespace {

TEST(HmEncodeData, EmptySequence)
{
  typedef char entity_type;

  hm::enc_node<entity_type> node(/* frequency: */ 10);
  std::vector<uint8_t> out;
  hm::meta md;

  hm::encode_data<entity_type>(
    static_cast<entity_type *>(nullptr),
    static_cast<entity_type *>(nullptr),
    &node,
    std::back_inserter(out),
    md
  );

  // expect no side effects
  EXPECT_EQ(out.size(), 0);
  EXPECT_EQ(md.data_byte_count, 0);
  EXPECT_EQ(md.data_last_bits, 0);
}

TEST(HmEncodeDataDeathTest, EmptyTree)
{
  typedef char entity_type;

  std::vector<uint8_t> out;
  hm::meta md; // md.entity_size == 0

  // test assert tree != nullptr
  EXPECT_DEATH(
    hm::encode_data<entity_type>(
      static_cast<entity_type *>(nullptr),
      static_cast<entity_type *>(nullptr),
      static_cast<const hm::enc_node<entity_type> *>(nullptr),
      std::back_inserter(out),
      md
    ),
    ""
  );
}


}

