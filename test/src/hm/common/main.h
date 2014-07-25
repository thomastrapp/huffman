#include <type_traits>
#include <cstdint>
#include <vector>
#include <utility>
#include <limits>
#include <algorithm>
#include <iterator>

#include "gtest/gtest.h"

#include "hm/common.h"

namespace {

TEST(HmCommon, HmMetaUnchanged)
{
  EXPECT_TRUE((std::is_same<hm::meta::version_type, uint8_t>::value));
  EXPECT_TRUE((std::is_same<hm::meta::last_bits_type, uint8_t>::value));
  EXPECT_TRUE((std::is_same<hm::meta::entity_size_type, uint8_t>::value));
  EXPECT_TRUE((std::is_same<hm::meta::entity_count_type, uint32_t>::value));
  EXPECT_TRUE((std::is_same<hm::meta::tree_count_type, uint32_t>::value));
  EXPECT_TRUE((std::is_same<hm::meta::data_count_type, uint64_t>::value));

  EXPECT_EQ(hm::max_shifts_in_byte, 7U);
}

TEST(HmCommon, GetBit)
{
  uint8_t byte = 255;
  for(uint8_t i = 0; i < 8; ++i)
    EXPECT_TRUE(hm::get_bit(byte, i));

  byte = 0;
  for(uint8_t i = 0; i < 8; ++i)
    EXPECT_FALSE(hm::get_bit(byte, i));

  byte = 10;
  for(uint8_t i = 0; i < 8; ++i)
  {
    if( i == 6 || i == 4 ) // 2^1 = 2; 2^3 = 8; 2 + 8 = 10
      EXPECT_TRUE(hm::get_bit(byte, i));
    else
      EXPECT_FALSE(hm::get_bit(byte, i));
  }

  byte = 1;
  for(uint8_t i = 0; i < 8; ++i)
  {
    if( i == 7 )
      EXPECT_TRUE(hm::get_bit(byte, i));
    else
      EXPECT_FALSE(hm::get_bit(byte, i));
  }

  byte = 128;
  for(uint8_t i = 0; i < 8; ++i)
  {
    if( i == 0 )
      EXPECT_TRUE(hm::get_bit(byte, i));
    else
      EXPECT_FALSE(hm::get_bit(byte, i));
  }
}

TEST(HmCommon, SetBit)
{
  uint8_t byte = 0;
  std::vector<std::pair<uint8_t, uint8_t>> powers(8);
  for(uint8_t i = 0; i < 8; ++i)
  {
    byte |= 1U << (7U - i);
    powers.at(i) = std::pair<uint8_t, uint8_t>(i, byte);
  }

  byte = 0;
  for(const auto& p : powers)
  {
    byte = hm::set_bit(byte, p.first);
    EXPECT_EQ(byte, p.second);
  }

  EXPECT_EQ(hm::set_bit(0, 0), 128);
  EXPECT_EQ(hm::set_bit(0, 7), 1);
}


TEST(HmCommon, IsForwardIterator)
{
  EXPECT_TRUE(hm::is_forward_iterator<int *>::value);
  EXPECT_TRUE(hm::is_forward_iterator<const int *>::value);
  EXPECT_TRUE(hm::is_forward_iterator<std::vector<int>::iterator>::value);
  EXPECT_TRUE(hm::is_forward_iterator<std::vector<int>::const_iterator>::value);
  EXPECT_FALSE(hm::is_forward_iterator<std::istreambuf_iterator<char>>::value);
  EXPECT_FALSE(hm::is_forward_iterator<std::istream_iterator<const char>>::value);
}

TEST(HmCommon, NodeFreqCompare)
{
  typedef hm::enc_node<int> node_type;
  typedef std::unique_ptr<node_type> uniq_node;

  auto left =
    util::make_unique<node_type>(64);
  auto right =
    util::make_unique<node_type>(32);

  auto comp = hm::node_freq_compare<node_type>();

  EXPECT_TRUE(comp(left, right));
  EXPECT_FALSE(comp(right, left));
  EXPECT_FALSE(comp(right, right));

  // empty nodes
  EXPECT_FALSE(comp(uniq_node(nullptr), right));
  EXPECT_FALSE(comp(right, uniq_node(nullptr)));
  EXPECT_FALSE(comp(uniq_node(nullptr), uniq_node(nullptr)));
}


TEST(HmCommonDecodeType, DecodesTypes)
{
  uint64_t my_magic_number = std::numeric_limits<uint64_t>::max() - 23;
  uint8_t bytes[] = {
    0xff - 23, 0xff, 0xff, 0xff,
    0xff,      0xff, 0xff, 0xff
  };

  uint8_t * it = std::begin(bytes);
  auto ret = hm::decode_type<uint64_t>(it, std::end(bytes));
  EXPECT_EQ(ret, my_magic_number);
  EXPECT_EQ(it, std::end(bytes));
}

TEST(HmCommonDecodeType, ThrowsOnUnexpectedEnd)
{
  {
    const char * str = "T";
    EXPECT_NO_THROW(hm::decode_type<char>(str, str + strlen(str)));
  }

  {
    const int i = 0;
    const int * iter = &i;
    EXPECT_THROW(hm::decode_type<uint8_t>(iter, iter), hm::invalid_layout);
  }

  {
    const uint8_t buffer[8] = {0};
    typedef uint32_t entity_type;
    for(size_t range = 0; range <= sizeof(buffer); ++range)
    {
      const uint8_t * walker = buffer;
      if( range < sizeof(entity_type) )
      {
        // expecting more bytes than given
        EXPECT_THROW(
          hm::decode_type<entity_type>(walker, walker + range),
          hm::invalid_layout
        );
        EXPECT_EQ(walker, buffer + range);
      }
      else if( range >= sizeof(entity_type) )
      {
        // given enough bytes or more
        EXPECT_NO_THROW(
          hm::decode_type<entity_type>(walker, walker + range)
        );
        EXPECT_EQ(walker, buffer + sizeof(entity_type));
      }
    }
  }
}

TEST(HmCommonEncodeType, EncodesTypes)
{
  std::vector<uint8_t> out;
  std::vector<uint8_t> bytes;
  uint64_t my_magic_number = std::numeric_limits<uint64_t>::max();
  for(size_t i = 0; i < sizeof(uint64_t); ++i)
  {
    bytes.push_back(static_cast<uint8_t>(my_magic_number >> (i * 8)));
  }

  hm::encode_type(my_magic_number, std::back_inserter(out));

  ASSERT_EQ(bytes.size(), out.size());
  EXPECT_TRUE(std::equal(bytes.begin(), bytes.end(), out.begin()));
}

TEST(HmCommonDeathTest, GetBit)
{
  // test assert
  EXPECT_DEATH(hm::get_bit(0, 8), "");
}

TEST(HmCommonDeathTest, SetBit)
{
  // test assert
  EXPECT_DEATH(hm::set_bit(0, 8), "");
}


}

