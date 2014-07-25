#include <cmath>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <limits>
#include <numeric>
#include <iterator>
#include <unordered_map>
#include <vector>
#include <utility>

#include "gtest/gtest.h"

#include "util/make-unique.h"
#include "hlp/get-test-data.h"
#include "hlp/testing-types.h"
#include "hm/encode.h"


namespace {

namespace hlp {
  template<
    typename target_type,
    typename source_type
  >
  bool is_a(source_type * node)
  {
    return dynamic_cast<target_type *>(node);
  }

  template<typename entity_type>
  unsigned int get_max_depth(hm::enc_node<entity_type> * node, unsigned int depth = 0)
  {
    if( auto tree = dynamic_cast<hm::enc_tree<entity_type> *>(node) )
    {
      return std::max(
        get_max_depth(tree->get_left(), depth + 1),
        get_max_depth(tree->get_right(), depth + 1)
      );
    }

    return depth;
  }

  template<typename entity_type>
  int get_entity_depth(hm::enc_node<entity_type> * node, entity_type entity, int depth = 0)
  {
    if( auto tree = dynamic_cast<hm::enc_tree<entity_type> *>(node) )
    {
      int left = get_entity_depth(tree->get_left(), entity, depth + 1);
      if( left > -1 )
        return left;

      int right = get_entity_depth(tree->get_right(), entity, depth + 1);
      if( right > -1 )
        return right;
    }
    else if( auto leaf = dynamic_cast<hm::enc_leaf<entity_type> *>(node) )
    {
      if( leaf->get_entity() == entity )
        return depth;
    }

    return -1;
  }

  template<typename entity_type>
  std::vector<std::pair<entity_type, size_t>>
  get_entities_sorted_by_freq(const std::unordered_map<entity_type, size_t>& table)
  {
    typedef std::pair<entity_type, size_t> pair_type;
    std::vector<pair_type> entities_by_freq(
      table.begin(),
      table.end()
    );
    auto sort_by_freq = [](const pair_type& left, const pair_type& right)
    {
      return left.second < right.second;
    };
    std::stable_sort(entities_by_freq.begin(), entities_by_freq.end(), sort_by_freq);
    return entities_by_freq;
  }

  template<typename entity_type>
  bool is_frequency_valid(hm::enc_node<entity_type> * node)
  {
    if( auto tree = dynamic_cast<hm::enc_tree<entity_type> *>(node) )
    {
      size_t freq_left = 0;
      if( tree->get_left() != nullptr )
        freq_left = tree->get_left()->get_frequency();

      size_t freq_right = 0;
      if( tree->get_right() != nullptr )
        freq_right = tree->get_right()->get_frequency();

      // the frequency for this node must equal the sum of the children's frequency
      bool this_valid = tree->get_frequency() == (freq_left + freq_right);

      bool left_valid = is_frequency_valid(tree->get_left());
      bool right_valid = is_frequency_valid(tree->get_right());

      return this_valid && left_valid && right_valid;
    }

    return true;
  }
}


TEST(HmBuildHuffmanTree, EmptySequence)
{
  const char * str = "";
  const char * str_end = str;
  std::unique_ptr<hm::enc_node<char>> node =
    hm::build_huffman_tree<char>(str, str_end);
  EXPECT_EQ(node.get(), nullptr);
}


TEST(HmBuildHuffmanTree, UniqueInput)
{
  const char * inputs[] = {
    "0", "11", "22222222222222222222222222222222222222222222222222"
  };

  for(const char * str : inputs)
  {
    size_t len = strlen(str);
    const char * str_end = str + len;

    // self test
    ASSERT_FALSE(str == str_end);

    std::unique_ptr<hm::enc_node<char>> node =
      hm::build_huffman_tree<char>(str, str_end);

    ASSERT_NE(node.get(), nullptr);

    // expect a tree
    ASSERT_TRUE(hlp::is_a<hm::enc_tree<char>>(node.get()));
    auto tree = dynamic_cast<hm::enc_tree<char> *>(node.get());
    ASSERT_NE(tree->get_left(), nullptr);
    EXPECT_EQ(tree->get_right(), nullptr);
    EXPECT_EQ(tree->get_frequency(), len);
    EXPECT_EQ(tree->get_left()->get_frequency(), len);

    // expect a leaf at the tree's left branch
    ASSERT_TRUE(hlp::is_a<hm::enc_leaf<char>>(tree->get_left()));
    ASSERT_FALSE(hlp::is_a<hm::enc_tree<char>>(tree->get_left()));

    auto leaf = dynamic_cast<hm::enc_leaf<char> *>(tree->get_left());
    EXPECT_EQ(leaf->get_entity(), str[0]);
  }
}


TEST(HmBuildHuffmanTree, HelperSelfTest)
{
  typedef hm::enc_tree<char> t_tree;
  typedef hm::enc_leaf<char> t_leaf;
  typedef hm::enc_node<char> t_node;
  typedef std::unique_ptr<t_node> uniq_node;

  EXPECT_EQ(hlp::get_max_depth(static_cast<hm::enc_node<char> *>(nullptr)), 0);
  EXPECT_EQ(hlp::get_entity_depth(static_cast<hm::enc_node<char> *>(nullptr), '0'), -1);

  {
    t_leaf leaf(0, '0');
    EXPECT_EQ(hlp::get_max_depth(&leaf), 0);
    EXPECT_EQ(hlp::get_entity_depth(&leaf, '0'), 0);
  }

  // construct tree from bottom up
  uniq_node tree_r2 = util::make_unique<t_tree>(
    1,
    util::make_unique<t_leaf>(0, '1'),
    util::make_unique<t_leaf>(0, '2')
  );
  EXPECT_EQ(hlp::get_max_depth(tree_r2.get()), 1);
  EXPECT_EQ(hlp::get_entity_depth(tree_r2.get(), '1'), 1);
  EXPECT_EQ(hlp::get_entity_depth(tree_r2.get(), '2'), 1);

  uniq_node tree_l4 = util::make_unique<t_tree>(0, util::make_unique<t_tree>());
  EXPECT_EQ(hlp::get_max_depth(tree_l4.get()), 2);
  EXPECT_EQ(hlp::get_entity_depth(tree_l4.get(), '0'), -1);

  uniq_node tree_l3 = util::make_unique<t_tree>(
    0,
    util::make_unique<t_tree>(),
    std::move(tree_l4)
  );
  EXPECT_EQ(hlp::get_max_depth(tree_l3.get()), 3);
  EXPECT_EQ(hlp::get_entity_depth(tree_l3.get(), '0'), -1);

  uniq_node tree_l2 = util::make_unique<t_tree>(
    0,
    util::make_unique<t_tree>(),
    std::move(tree_l3)
  );
  EXPECT_EQ(hlp::get_max_depth(tree_l2.get()), 4);
  EXPECT_EQ(hlp::get_entity_depth(tree_l2.get(), '0'), -1);

  auto root = util::make_unique<t_tree>(
    0,
    std::move(tree_l2),
    std::move(tree_r2)
  );
  EXPECT_EQ(hlp::get_max_depth(root.get()), 5);
  EXPECT_EQ(hlp::get_entity_depth(root.get(), '1'), 2);
  EXPECT_EQ(hlp::get_entity_depth(root.get(), '2'), 2);
  EXPECT_EQ(hlp::get_max_depth(root->get_left()), 4);
  EXPECT_EQ(hlp::get_max_depth(root->get_right()), 1);

  // The tree looks like this:
  //
  // 1:      [ root ]
  //        /        \
  // 2:    *          *
  //      / \        / \
  // 3:  *   *     '1' '2'
  //        / \
  // 4:    *   *
  //          /
  // 5:      *
  //
  // * is a hm::enc_tree
  // '1' and '2' are each a hm::enc_leaf
}


template <typename T>
class HmBuildHuffmanTreeT : public ::testing::Test {};
TYPED_TEST_CASE(HmBuildHuffmanTreeT, ::hlp::testing_types);
TYPED_TEST(HmBuildHuffmanTreeT, ValidTree)
{
  typedef TypeParam entity_type;
  auto inputs = ::hlp::get_test_data<entity_type>();
  for(auto vec : inputs)
  {
    auto freq_table = hm::build_frequency_table<entity_type>(vec.begin(), vec.end());

    std::unique_ptr<hm::enc_node<entity_type>> node =
      hm::build_huffman_tree<entity_type>(vec.begin(), vec.end());

    // check if entities with lower frequency have higher or equal depth
    // than entities with higher frequency
    if( freq_table.size() )
    {
      auto max_depth = hlp::get_max_depth(node.get());
      ASSERT_GT(max_depth, 0);

      // collect all frequencies on each tree level
      std::vector<std::vector<size_t>> freqs_by_level(max_depth + 1);
      for(const auto& entity_freq : freq_table)
      {
        auto depth = hlp::get_entity_depth(node.get(), entity_freq.first);
        ASSERT_GT(depth, -1);
        freqs_by_level.at(depth).push_back(entity_freq.second);
      }

      // the maximum frequency on each level must be less or equal to the maximum
      // frequency of the previous level
      size_t last_max = std::numeric_limits<size_t>::max();
      for(const auto& freqs : freqs_by_level)
      {
        // some levels don't have any leafs and therefore their frequency is
        // irrelevant
        if( freqs.begin() != freqs.end() )
        {
          const auto it_this_max = std::max_element(freqs.begin(), freqs.end());
          ASSERT_NE(it_this_max, freqs.end());
          EXPECT_LE(*it_this_max, last_max);
          last_max = *it_this_max;
        }
      }

      // check if all enc_node's frequencies are valid
      EXPECT_TRUE(hlp::is_frequency_valid(node.get()));
    }
    else
    {
      EXPECT_EQ(hlp::get_max_depth(node.get()), 0);
    }
  }
}


}

