#include <vector>
#include <cstdint>
#include <algorithm>

#include "gtest/gtest.h"

#include "hm/decode.h"
#include "hm/encode.h"

#include "hlp/get-test-data.h"
#include "hlp/testing-types.h"


namespace {

template <typename T>
class HmDecodeDataT : public ::testing::Test {};
TYPED_TEST_CASE(HmDecodeDataT, ::hlp::testing_types);
TYPED_TEST(HmDecodeDataT, DecodesData)
{
  typedef TypeParam entity_type;
  auto const inputs = ::hlp::get_test_data<entity_type>();

  for(const auto& input : inputs)
  {
    if( input.size() )
    {
      std::vector<uint8_t> out_tree;
      std::vector<uint8_t> out_entities;
      std::vector<uint8_t> out_data;
      std::vector<uint8_t> in_data;
      std::vector<entity_type> output;

      hm::meta md;
      md.entity_size = sizeof(entity_type);

      // encode data
      auto tree = hm::build_huffman_tree<entity_type>(input.begin(), input.end());
      hm::encode_entities(tree.get(), std::back_inserter(out_entities), md);
      hm::encode_tree(tree.get(), std::back_inserter(out_tree), md);
      hm::encode_data(input.begin(), input.end(), tree.get(), std::back_inserter(out_data), md);

      // decode data
      auto dec_entities = hm::decode_entities(out_entities.begin(), out_entities.end(), md);
      auto dec_tree = hm::decode_tree(out_tree.begin(), out_tree.end(), dec_entities, md);
      hm::decode_data(out_data.begin(), out_data.end(), dec_tree.get(), md, std::back_inserter(in_data));

      // expect the amount of bytes to equal
      EXPECT_EQ(in_data.size(), input.size());

      // expect input to equal output
      EXPECT_TRUE(std::equal(input.begin(), input.end(), in_data.begin()));
    }
  }
}


}

