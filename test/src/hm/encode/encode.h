#include <iterator>
#include <cstdint>
#include <vector>

#include "gtest/gtest.h"

#include "hm/encode.h"

#include "hlp/is-same-meta.h"

namespace {

TEST(HmEncode, EmptySequence)
{
  typedef char entity_type;
  std::vector<uint8_t> out;

  hm::meta md_expected;
  md_expected.version = 10;
  md_expected.entity_size = sizeof(entity_type);

  const char * empty = "";
  const char * empty_end = empty;

  auto tree = hm::build_huffman_tree<entity_type>(empty, empty_end);

  hm::meta md_encoded = hm::encode<entity_type>(
    empty,
    empty_end,
    tree.get(),
    std::back_inserter(out)
  );

  EXPECT_EQ(out.size(), 0);
  EXPECT_TRUE(::hlp::is_same_meta(md_encoded, md_expected));

  hm::meta md_encoded_nulltree = hm::encode<entity_type>(
    empty,
    empty_end,
    static_cast<hm::enc_node<entity_type> *>(nullptr),
    std::back_inserter(out)
  );

  EXPECT_EQ(out.size(), 0);
  EXPECT_TRUE(::hlp::is_same_meta(md_encoded_nulltree, md_expected));
}


}

