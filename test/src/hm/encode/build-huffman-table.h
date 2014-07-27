#include <algorithm>
#include <numeric>
#include <limits>
#include <vector>

#include "gtest/gtest.h"

#include "hlp/get-test-data.h"
#include "hlp/testing-types.h"

#include "hm/encode.h"


namespace {

TEST(HmBuildHuffmanTable, NoTree)
{
  std::unordered_map<char, hm::code_type> table;
  hm::code_type prefix;
  hm::build_huffman_table<char>(static_cast<hm::enc_node<char> *>(nullptr), table, prefix);
  EXPECT_EQ(table.size(), 0);
}


template <typename T>
class HmBuildHuffmanTableT: public ::testing::Test {};
TYPED_TEST_CASE(HmBuildHuffmanTableT, ::hlp::testing_types);
TYPED_TEST(HmBuildHuffmanTableT, ValidCodes)
{
  typedef TypeParam entity_type;
  auto inputs = ::hlp::get_test_data<entity_type>();
  for(auto vec : inputs)
  {
    auto freq_table = hm::build_frequency_table<entity_type>(vec.begin(), vec.end());

    auto tree = hm::build_huffman_tree<entity_type>(vec.begin(), vec.end());

    std::unordered_map<entity_type, hm::code_type> table;
    hm::code_type prefix;
    hm::build_huffman_table<entity_type>(tree.get(), table, prefix);
    ASSERT_FALSE(tree.get() == nullptr && table.size() > 0);
    EXPECT_EQ(table.size(), freq_table.size());

    for(const auto& code : table)
    {
      hm::enc_node<entity_type> * walker = tree.get();
      bool just_found_a_leaf = false;
      // for each bit we walk through the tree until a leaf is found
      for(const auto bit : code.second)
      {
        just_found_a_leaf = false;

        if( auto tree = dynamic_cast<hm::enc_tree<entity_type> *>(walker) )
        {
          walker = ( bit ? tree->get_right() : tree->get_left() );
        }

        if( auto leaf = dynamic_cast<hm::enc_leaf<entity_type> *>(walker) )
        {
          EXPECT_EQ(leaf->get_entity(), code.first);
          walker = tree.get();
          just_found_a_leaf = true;
        }

        ASSERT_NE(walker, nullptr);
      }

      EXPECT_TRUE(just_found_a_leaf);
    }
  }
}


}

