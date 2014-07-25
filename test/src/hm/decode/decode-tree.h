#include <vector>
#include <cstdint>

#include "gtest/gtest.h"

#include "hlp/get-test-data.h"
#include "hlp/byte-vector-to-entity.h"
#include "hlp/testing-types.h"

#include "hm/encode.h"
#include "hm/decode.h"


namespace {

namespace hlp {

  template<
    typename entity_type,
    typename byte_vector_type
  >
  bool is_same_tree(hm::enc_node<entity_type> * left, hm::dec_node<byte_vector_type> * right)
  {
    typedef hm::dec_tree<byte_vector_type> dec_tree;
    typedef hm::dec_leaf<byte_vector_type> dec_leaf;
    typedef hm::enc_tree<entity_type> enc_tree;
    typedef hm::enc_leaf<entity_type> enc_leaf;

    if( auto tree = dynamic_cast<enc_tree *>(left) )
    {
      auto r_tree = dynamic_cast<dec_tree *>(right);
      return
           r_tree
        && is_same_tree<entity_type>(tree->get_left(), r_tree->get_left())
        && is_same_tree<entity_type>(tree->get_right(), r_tree->get_right())
      ;
    }
    else if( auto leaf = dynamic_cast<enc_leaf *>(left) )
    {
      auto r_leaf = dynamic_cast<dec_leaf *>(right);
      return
           r_leaf
        && leaf->get_entity() == ::hlp::byte_vector_to_entity<entity_type>(r_leaf->get_entity());
    }
    else
    {
      return left == nullptr && right == nullptr;
    }
  }


}

TEST(HmDecodeTree, ThrowsInvalidLayout)
{
  const char * input = "This is a test.";
  const char * input_end = input + strlen(input);

  std::vector<uint8_t> out_tree;
  std::vector<uint8_t> out_entities;

  hm::meta md_enc;
  md_enc.entity_size = sizeof(char);

  auto tree = hm::build_huffman_tree<char>(input, input_end);

  // convert entities from entity_type to byte-vector
  hm::encode_entities(tree.get(), std::back_inserter(out_entities), md_enc);
  auto dec_entities = hm::decode_entities(out_entities.begin(), out_entities.end(), md_enc);
  ASSERT_EQ(dec_entities.size(), md_enc.entity_count);

  hm::encode_tree(tree.get(), std::back_inserter(out_tree), md_enc);

  // throws on missing byte
  EXPECT_THROW(hm::decode_tree(out_tree.begin(), out_tree.begin() + out_tree.size() - 1, dec_entities, md_enc), hm::invalid_layout);

  // add an entity
  dec_entities.push_back(std::vector<uint8_t>(md_enc.entity_size, 12));
  md_enc.entity_count++;

  // throws on too many entities
  EXPECT_THROW(hm::decode_tree(out_tree.begin(), out_tree.end(), dec_entities, md_enc), hm::invalid_layout);
  md_enc.entity_count--;

  // remove 2 entities (we now have 1 entity less than we should have)
  dec_entities.erase(dec_entities.begin() + dec_entities.size() - 2, dec_entities.end());
  ASSERT_EQ(dec_entities.size(), md_enc.entity_count - 1);

  // throws on missing entity
  EXPECT_THROW(hm::decode_tree(out_tree.begin(), out_tree.end(), dec_entities, md_enc), hm::invalid_layout);
}


template <typename T>
class HmDecodeTreeT : public ::testing::Test {};
TYPED_TEST_CASE(HmDecodeTreeT, ::hlp::testing_types);
TYPED_TEST(HmDecodeTreeT, DecodesTree)
{
  typedef TypeParam entity_type;
  const auto inputs = ::hlp::get_test_data<entity_type>();
  for(const auto& input : inputs)
  {
    if( input.size() )
    {
      std::vector<uint8_t> out_tree;
      std::vector<uint8_t> out_entities;

      hm::meta md;
      md.entity_size = sizeof(entity_type);

      // build and encode a tree and its entities
      auto tree = hm::build_huffman_tree<entity_type>(input.begin(), input.end());
      hm::encode_entities<entity_type>(tree.get(), std::back_inserter(out_entities), md);
      hm::encode_tree<entity_type>(tree.get(), std::back_inserter(out_tree), md);

      // decode the tree and its entities
      auto dec_entities = hm::decode_entities(out_entities.begin(), out_entities.end(), md);
      auto dec_tree = hm::decode_tree(out_tree.begin(), out_tree.end(), dec_entities, md);

      // expect the trees to have the same structure and same entities in the same places
      EXPECT_TRUE(hlp::is_same_tree(tree.get(), dec_tree.get()));
    }
  }
}


}

