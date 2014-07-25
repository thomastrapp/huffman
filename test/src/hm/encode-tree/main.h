#include <type_traits>
#include <memory>

#include "gtest/gtest.h"

#include "hm/encode-tree.h"
#include "util/make-unique.h"

namespace {


TEST(HmEncTree, GetFrequency)
{
  size_t frequency = 23;
  hm::enc_tree<int> tree(frequency);
  hm::enc_node<int> * node = &tree;

  ASSERT_EQ(tree.get_frequency(), frequency);
  ASSERT_EQ(node->get_frequency(), frequency);
}

TEST(HmEncTree, NodeHasVirtualDestructor)
{
  EXPECT_TRUE(std::has_virtual_destructor<hm::enc_node<int>>::value);
}

TEST(HmEncTree, LeafIsANode)
{
  auto b = std::is_convertible<
    hm::enc_leaf<int>,
    hm::enc_node<int>
  >::value;
  EXPECT_TRUE(b);
}

TEST(HmEncTree, LeafGetFrequencyEntity)
{
  size_t frequency = 23;
  int entity = 32;
  hm::enc_leaf<int> leaf(frequency, entity);

  ASSERT_EQ(leaf.get_frequency(), frequency);
  ASSERT_EQ(leaf.get_entity(), entity);
}

TEST(HmEncTree, TreeIsANode)
{
  auto b = std::is_convertible<
    hm::enc_tree<int>,
    hm::enc_node<int>
  >::value;
  EXPECT_TRUE(b);
}

TEST(HmEncTree, TreeGetChildren)
{
  size_t l_frequency = 23;
  size_t r_frequency = 32;
  size_t t_frequency = l_frequency + r_frequency;
  hm::enc_tree<int> tree(
    t_frequency,
    util::make_unique<hm::enc_node<int>>(l_frequency),
    util::make_unique<hm::enc_node<int>>(r_frequency)
  );

  EXPECT_EQ(tree.get_frequency(), t_frequency);
  EXPECT_EQ(tree.get_left()->get_frequency(), l_frequency);
  EXPECT_EQ(tree.get_right()->get_frequency(), r_frequency);

  hm::enc_tree<int> empty_tree(0);
  EXPECT_EQ(empty_tree.get_frequency(), 0);
  EXPECT_EQ(empty_tree.get_left(), nullptr);
  EXPECT_EQ(empty_tree.get_right(), nullptr);
}

TEST(HmEncTree, TreeHasDeletedCopyConstructor)
{
  EXPECT_FALSE(std::is_copy_constructible<hm::enc_tree<int>>::value);
}

TEST(HmEncTree, TreeHasDeletedAssignmentOperator)
{
  auto b = std::is_assignable<
    hm::enc_tree<int>,
    hm::enc_tree<int>
  >::value;
  EXPECT_FALSE(b);
}

TEST(HmEncTree, TreeSingleChild)
{
  size_t l_frequency = 23;
  size_t t_frequency = l_frequency;

  hm::enc_tree<int> tree(
    t_frequency,
    util::make_unique<hm::enc_node<int>>(l_frequency)
  );

  EXPECT_EQ(tree.get_frequency(), t_frequency);
  EXPECT_EQ(tree.get_left()->get_frequency(), l_frequency);
  EXPECT_EQ(tree.get_right(), nullptr);
}


}

