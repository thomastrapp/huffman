#include <cstdint>
#include <iterator>
#include <vector>

#include "gtest/gtest.h"

#include "hm/decode.h"

#include "hlp/is-same-meta.h"

namespace {

TEST(HmDecodeMetaData, ThrowsInvalidLayout)
{
  uint8_t empty = 0;
  EXPECT_THROW(hm::decode_meta_data(&empty, &empty), hm::invalid_layout);

  constexpr size_t expected_bytes =
    sizeof(hm::meta::version_type) +
    sizeof(hm::meta::last_bits_type) +
    sizeof(hm::meta::last_bits_type) +
    sizeof(hm::meta::entity_size_type) +
    sizeof(hm::meta::entity_count_type) +
    sizeof(hm::meta::tree_count_type) +
    sizeof(hm::meta::data_count_type);

  uint8_t not_enough[expected_bytes - 1] = {0};
  EXPECT_THROW(
    hm::decode_meta_data(
      std::begin(not_enough),
      std::end(not_enough)
    ),
    hm::invalid_layout
  );

  uint8_t enough[expected_bytes] = {0};
  EXPECT_NO_THROW(
    hm::decode_meta_data(
      std::begin(enough),
      std::end(enough)
    )
  );
}

TEST(HmDecodeMetaData, DecodesMeta)
{
  hm::meta md;
  md.version = 255;
  md.tree_last_bits = 250;
  md.data_last_bits = 249;
  md.entity_size = 230;
  md.entity_count = 221;
  md.tree_byte_count = 212;
  md.data_byte_count = 203;

  std::vector<uint8_t> bytes;
  hm::encode_meta_data(md, std::back_inserter(bytes));
  auto md_decoded = hm::decode_meta_data(bytes.begin(), bytes.end());

  EXPECT_TRUE(::hlp::is_same_meta(md, md_decoded));
}


}
