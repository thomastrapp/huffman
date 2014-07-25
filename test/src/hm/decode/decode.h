#include <vector>
#include <cstdint>
#include <iterator>
#include <sstream>

#include "gtest/gtest.h"

#include "hm/decode.h"
#include "hm/encode.h"

#include "hlp/get-test-data.h"
#include "hlp/testing-types.h"

namespace {

template<typename T>
class HmDecodeT : public ::testing::Test {};
TYPED_TEST_CASE(HmDecodeT, ::hlp::testing_types);
TYPED_TEST(HmDecodeT, Decodes)
{
  typedef TypeParam entity_type;
  const auto inputs = ::hlp::get_test_data<entity_type>();
  for(const auto& input : inputs)
  {
    if( input.size() )
    {
      std::vector<uint8_t> enc_out;
      std::vector<uint8_t> dec_out;

      auto tree = hm::build_huffman_tree<entity_type>(input.begin(), input.end());
      auto md = hm::encode(input.begin(), input.end(), tree.get(), std::back_inserter(enc_out));
      hm::decode(md, enc_out.begin(), enc_out.end(), std::back_inserter(dec_out));

      // expect the amount of bytes to equal
      EXPECT_EQ(dec_out.size(), input.size());

      // expect input to equal output
      EXPECT_TRUE(std::equal(input.begin(), input.end(), dec_out.begin()));
    }
  }
}

TYPED_TEST(HmDecodeT, InputIterator)
{
  typedef TypeParam entity_type;
  const auto inputs = ::hlp::get_test_data<entity_type>();
  for(const auto& input : inputs)
  {
    if( input.size() )
    {
      std::vector<uint8_t> enc_out;
      std::vector<uint8_t> dec_out;

      std::basic_stringstream<uint8_t> in;
      EXPECT_EQ(in.tellp(), 0);

      std::copy(input.begin(), input.end(), std::ostreambuf_iterator<uint8_t>(in));
      EXPECT_EQ(in.tellp(), input.size());

      auto tree = hm::build_huffman_tree<entity_type>(
        std::istreambuf_iterator<uint8_t>(in),
        std::istreambuf_iterator<uint8_t>()
      );
      EXPECT_EQ(in.tellg(), input.size());

      std::copy(input.begin(), input.end(), std::ostreambuf_iterator<uint8_t>(in));
      EXPECT_EQ(in.tellp(), input.size() * 2);

      auto md = hm::encode(
        std::istreambuf_iterator<uint8_t>(in),
        std::istreambuf_iterator<uint8_t>(),
        tree.get(),
        std::back_inserter(enc_out)
      );
      EXPECT_EQ(in.tellg(), input.size() * 2);

      hm::decode(md, enc_out.begin(), enc_out.end(), std::back_inserter(dec_out));

      // expect the amount of bytes to equal
      EXPECT_EQ(dec_out.size(), input.size());

      // expect input to equal output
      EXPECT_TRUE(std::equal(input.begin(), input.end(), dec_out.begin()));
    }
  }
}


}

