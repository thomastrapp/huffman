#include <iterator>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "gtest/gtest.h"

#include "hlp/is-same-meta.h"
#include "hlp/get-test-data.h"
#include "hlp/testing-types.h"

#include "hm/encode.h"


namespace {

namespace hlp {

  template<typename entity_type>
  void get_entities_from_tree(hm::enc_node<entity_type> * node, std::vector<entity_type>& entities)
  {
    if( auto tree = dynamic_cast<hm::enc_tree<entity_type> *>(node) )
    {
      get_entities_from_tree(tree->get_left(), entities);
      get_entities_from_tree(tree->get_right(), entities);
    }
    else if( auto leaf = dynamic_cast<hm::enc_leaf<entity_type> *>(node) )
    {
      entities.push_back(leaf->get_entity());
    }
  }

}

template <typename T>
class HmEncodeEntitiesT : public ::testing::Test {};
TYPED_TEST_CASE(HmEncodeEntitiesT, ::hlp::testing_types);
TYPED_TEST(HmEncodeEntitiesT, EmptyTree)
{
  typedef TypeParam entity_type;
  std::vector<uint8_t> out;

  hm::meta md;
  md.entity_size = sizeof(entity_type);

  hm::encode_entities<entity_type>(
    static_cast<const hm::enc_node<entity_type> *>(nullptr),
    std::back_inserter(out),
    md
  );

  // expect no side effects
  EXPECT_EQ(out.size(), 0);
  EXPECT_EQ(md.entity_count, 0);
}

TYPED_TEST(HmEncodeEntitiesT, ValidLayout)
{
  typedef TypeParam entity_type;
  const auto inputs = ::hlp::get_test_data<entity_type>();
  for(const auto& input : inputs)
  {
    if( input.size() )
    {
      auto tree = hm::build_huffman_tree<entity_type>(input.begin(), input.end());

      auto freq_table = hm::build_frequency_table<entity_type>(input.begin(), input.end());
      std::vector<std::pair<entity_type, size_t>> unique_chars(freq_table.begin(), freq_table.end());
      auto sort_by_freq_asc = []
        (
          const std::pair<entity_type, size_t>& left,
          const std::pair<entity_type, size_t>& right
        )
        {
          return left.second < right.second;
        }
      ;

      std::sort(unique_chars.begin(), unique_chars.end(), sort_by_freq_asc);

      std::vector<uint8_t> out;

      hm::meta md;
      md.entity_size = sizeof(entity_type);

      hm::encode_entities<entity_type>(tree.get(), std::back_inserter(out), md);

      EXPECT_GT(out.size(), 0);
      EXPECT_EQ(out.size(), md.entity_count * md.entity_size);
      EXPECT_EQ(out.size(), freq_table.size() * md.entity_size);

      std::vector<entity_type> entities;
      hlp::get_entities_from_tree(tree.get(), entities);

      auto out_walker = out.begin();
      auto entity_walker = entities.begin();
      while( out_walker != out.end() && entity_walker != entities.end() )
      {
        entity_type entity = hm::decode_type<entity_type>(out_walker, out.end());
        EXPECT_EQ(entity, *entity_walker);
        ++entity_walker;
      }
    }
    // else: empty sequence covered in another test
  }

}


template <typename T>
class HmEncodeEntitiesDeathTestT : public ::testing::Test {};
TYPED_TEST_CASE(HmEncodeEntitiesDeathTestT, ::hlp::testing_types);
TYPED_TEST(HmEncodeEntitiesDeathTestT, InvalidEntitySize)
{
  typedef TypeParam entity_type;
  const auto inputs = ::hlp::get_test_data<entity_type>();
  std::vector<uint8_t> out;
  hm::meta md; // md.entity_size == 0

  // test assert md.entity_size > 0
  EXPECT_DEATH(
    hm::encode_entities<entity_type>(
      static_cast<const hm::enc_node<entity_type> *>(nullptr),
      std::back_inserter(out),
      md
    ),
    ""
  );
}


}

