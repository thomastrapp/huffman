#include <vector>
#include <cstdint>
#include <iterator>

#include "gtest/gtest.h"

#include "hm/encode.h"

#include "hlp/get-test-data.h"
#include "hlp/is-same-meta.h"
#include "hlp/testing-types.h"


namespace {

namespace hlp {
  template<typename entity_type>
  bool verify_tree_by_encoded_data(
    hm::enc_node<entity_type> * node,
    const std::vector<uint8_t>& data,
    hm::meta::last_bits_type data_last_bits,
    hm::meta& md
  )
  {
    uint8_t is_leaf = hm::get_bit(data.at(md.tree_byte_count), md.tree_last_bits);

    if( auto tree = dynamic_cast<hm::enc_tree<entity_type> *>(node) )
    {
      md.tree_last_bits++;
      if( md.tree_last_bits > hm::max_shifts_in_byte )
      {
        md.tree_last_bits = 0;
        md.tree_byte_count++;
      }

      bool ret = !is_leaf;
      if( ret && tree->get_left() != nullptr )
        ret = verify_tree_by_encoded_data<entity_type>(tree->get_left(), data, data_last_bits, md);

      if( ret && tree->get_right() != nullptr )
        ret = verify_tree_by_encoded_data<entity_type>(tree->get_right(), data, data_last_bits, md);

      return ret;
    }
    else if( auto leaf = dynamic_cast<hm::enc_leaf<entity_type> *>(node) )
    {
      md.tree_last_bits++;
      if( md.tree_last_bits > hm::max_shifts_in_byte )
      {
        md.tree_last_bits = 0;
        md.tree_byte_count++;
      }

      return is_leaf;
    }

    return false;
  }


}

TEST(HmEncodeTree, NoTree)
{
  std::vector<uint8_t> out;
  hm::meta md;
  const hm::meta md_init;
  hm::encode_tree(
    static_cast<hm::enc_node<char> *>(nullptr),
    std::back_inserter(out),
    md
  );
  EXPECT_EQ(out.size(), 0);
  EXPECT_TRUE(::hlp::is_same_meta(md, md_init));
}


template <typename T>
class HmEncodeTreeT : public ::testing::Test {};
TYPED_TEST_CASE(HmEncodeTreeT, ::hlp::testing_types);
TYPED_TEST(HmEncodeTreeT, ValidLayout)
{
  typedef TypeParam entity_type;
  const auto inputs = ::hlp::get_test_data<entity_type>();
  for(const auto& input : inputs)
  {
    if( input.begin() != input.end() )
    {
      std::vector<uint8_t> out;
      auto tree = hm::build_huffman_tree<entity_type>(input.begin(), input.end());

      hm::meta md_encode;
      hm::encode_tree<entity_type>(tree.get(), std::back_inserter(out), md_encode);

      hm::meta md_actual;
      EXPECT_TRUE(hlp::verify_tree_by_encoded_data<entity_type>(
        tree.get(),
        out,
        md_encode.tree_last_bits,
        md_actual
      ));

      if( md_actual.tree_last_bits > 0 )
        md_actual.tree_byte_count++;

      EXPECT_TRUE(::hlp::is_same_meta(md_encode, md_actual));
    }
  }
}


}

