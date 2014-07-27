#include <iterator>
#include <cstdint>
#include <vector>

#include "gtest/gtest.h"

#include "hlp/get-test-data.h"
#include "hlp/byte-vector-to-entity.h"
#include "hlp/testing-types.h"

#include "hm/encode.h"
#include "hm/decode.h"


namespace {

TEST(HmDecodeEntities, EmptySequence)
{
  uint8_t buffer = 0;
  hm::meta md; // md.entity_count == 0
  auto entities = hm::decode_entities(&buffer, &buffer, md);
  EXPECT_EQ(entities.size(), 0);
}

TEST(HmDecodeEntities, ThrowsInvalid)
{
  uint8_t buffer[20] = {0};
  hm::meta md;
  md.entity_count = 21; // one too many
  md.entity_size = 1;
  EXPECT_THROW(hm::decode_entities(std::begin(buffer), std::end(buffer), md), hm::invalid_layout);
}


template <typename T>
class HmDecodeEntitiesT : public ::testing::Test {};
TYPED_TEST_CASE(HmDecodeEntitiesT, ::hlp::testing_types);
TYPED_TEST(HmDecodeEntitiesT, DecodesEntities)
{
  typedef TypeParam entity_type;
  auto inputs = ::hlp::get_test_data<entity_type>();

  for(const auto& input : inputs)
  {
    if( input.size() )
    {
      std::vector<uint8_t> out;

      hm::meta md;
      md.entity_size = sizeof(entity_type);

      // build a tree and encode its entities
      auto tree = hm::build_huffman_tree<entity_type>(input.begin(), input.end());
      std::unordered_map<entity_type, hm::code_type> table;
      hm::code_type prefix;
      hm::build_huffman_table<entity_type>(tree.get(), table, prefix);
      hm::encode_entities<entity_type>(tree.get(), std::back_inserter(out), md);

      auto entities = hm::decode_entities(out.begin(), out.end(), md);

      // the amount of entities must remain the same
      EXPECT_EQ(entities.size(), table.size());

      // collect all unique entities from input
      std::vector<entity_type> unique_entities;
      for(const auto& freq_pair : table)
      {
        unique_entities.push_back(freq_pair.first);
      }
      // self-test
      ASSERT_EQ(unique_entities.size(), table.size());

      // find each decoded entity in the unique_entities
      for(const auto& byte_vector : entities)
      {
        entity_type entity = ::hlp::byte_vector_to_entity<entity_type>(byte_vector);
        auto it = std::find(unique_entities.begin(), unique_entities.end(), entity);
        ASSERT_TRUE(it != unique_entities.end());
        unique_entities.erase(it);
      }

      // make sure that the decoded entities contained all entities from the input
      EXPECT_EQ(unique_entities.size(), 0);
    }
  }
}


}

