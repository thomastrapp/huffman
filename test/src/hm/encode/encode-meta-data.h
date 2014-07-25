#include <cstdint>
#include <vector>
#include <iterator>

#include "gtest/gtest.h"

#include "hm/encode.h"
#include "hm/decode.h"

#include "hlp/is-same-meta.h"

namespace {

TEST(HmEncodeMetaData, EncodesMeta)
{
  hm::meta md;
  md.version = 23;
  md.tree_last_bits = 42;
  md.data_last_bits = 12;
  md.entity_size = 99;
  md.entity_count = 100;
  md.tree_byte_count = 154;
  md.data_byte_count = 67;

  std::vector<uint8_t> bytes;
  hm::encode_meta_data(md, std::back_inserter(bytes));
  auto md_decoded = hm::decode_meta_data(bytes.begin(), bytes.end());

  EXPECT_TRUE(::hlp::is_same_meta(md, md_decoded));
}


}
