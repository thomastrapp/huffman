#include <type_traits>
#include <memory>

#include "gtest/gtest.h"

#include "hm/decode-tree.h"
#include "util/make-unique.h"

namespace {


TEST(HmDecTree, NodeHasVirtualDestructor)
{
  EXPECT_TRUE(std::has_virtual_destructor<hm::dec_node<int>>::value);
}

TEST(HmDecTree, LeafIsANode)
{
  auto b = std::is_convertible<
    hm::dec_leaf<int>,
    hm::dec_node<int>
  >::value;
  EXPECT_TRUE(b);
}

TEST(HmDecTree, LeafGetEntity)
{
  int entity = 32;
  hm::dec_leaf<int> leaf(entity);

  ASSERT_EQ(leaf.get_entity(), entity);
}

TEST(HmDecTree, TreeIsANode)
{
  auto b = std::is_convertible<
    hm::dec_tree<int>,
    hm::dec_node<int>
  >::value;
  EXPECT_TRUE(b);
}

TEST(HmDecTree, TreeGetChildren)
{
  int l_entity = 23;
  int r_entity = 32;
  hm::dec_tree<int> tree(
    util::make_unique<hm::dec_leaf<int>>(l_entity),
    util::make_unique<hm::dec_leaf<int>>(r_entity)
  );

  auto left = dynamic_cast<hm::dec_leaf<int> *>(tree.get_left());
  auto right = dynamic_cast<hm::dec_leaf<int> *>(tree.get_right());
  ASSERT_NE(left, nullptr);
  ASSERT_NE(right, nullptr);
  EXPECT_EQ(left->get_entity(), l_entity);
  EXPECT_EQ(right->get_entity(), r_entity);

  hm::dec_tree<int> empty_tree;
  EXPECT_EQ(empty_tree.get_left(), nullptr);
  EXPECT_EQ(empty_tree.get_right(), nullptr);
}

TEST(HmDecTree, TreeHasDeletedCopyConstructor)
{
  EXPECT_FALSE(std::is_copy_constructible<hm::dec_tree<int>>::value);
}

TEST(HmDecTree, TreeHasDeletedAssignmentOperator)
{
  auto b = std::is_assignable<
    hm::dec_tree<int>,
    hm::dec_tree<int>
  >::value;
  EXPECT_FALSE(b);
}

TEST(HmDecTree, TreeSingleChild)
{
  int l_entity = 23;

  hm::dec_tree<int> tree(
    util::make_unique<hm::dec_leaf<int>>(l_entity)
  );

  auto left = dynamic_cast<hm::dec_leaf<int> *>(tree.get_left());
  ASSERT_NE(left, nullptr);
  EXPECT_EQ(left->get_entity(), l_entity);
  EXPECT_EQ(tree.get_right(), nullptr);
}

TEST(HmDecTree, EmplaceNextChild)
{
  int l_entity = 23;
  int r_entity = 32;
  hm::dec_tree<int> tree;

  EXPECT_NO_THROW(tree.emplace_next_child<hm::dec_leaf<int>>(l_entity));
  EXPECT_NO_THROW(tree.emplace_next_child<hm::dec_leaf<int>>(r_entity));

  EXPECT_THROW(tree.emplace_next_child<hm::dec_leaf<int>>(l_entity), std::out_of_range);

  {
    std::unique_ptr<hm::dec_node<int>> dummy =
      util::make_unique<hm::dec_leaf<int>>(123);
    EXPECT_THROW(tree.set_next_child_and_own(std::move(dummy)), std::out_of_range);
  }

  auto left = dynamic_cast<hm::dec_leaf<int> *>(tree.get_left());
  ASSERT_NE(left, nullptr);
  EXPECT_EQ(left->get_entity(), l_entity);

  auto right = dynamic_cast<hm::dec_leaf<int> *>(tree.get_right());
  ASSERT_NE(right, nullptr);
  EXPECT_EQ(right->get_entity(), r_entity);

  EXPECT_TRUE(tree.is_full());
}

TEST(HmDecTree, SetNextChildAndOwn)
{
  int l_entity = 23;
  int r_entity = 32;
  hm::dec_tree<int> tree;

  {
    std::unique_ptr<hm::dec_node<int>> left =
      util::make_unique<hm::dec_leaf<int>>(l_entity);
    std::unique_ptr<hm::dec_node<int>> right =
      util::make_unique<hm::dec_leaf<int>>(r_entity);
    std::unique_ptr<hm::dec_node<int>> dummy =
      util::make_unique<hm::dec_leaf<int>>(123);

    EXPECT_NO_THROW(tree.set_next_child_and_own(std::move(left)));
    EXPECT_NO_THROW(tree.set_next_child_and_own(std::move(right)));

    EXPECT_THROW(tree.emplace_next_child<hm::dec_leaf<int>>(1234), std::out_of_range);
    EXPECT_THROW(tree.set_next_child_and_own(std::move(dummy)), std::out_of_range);
  }

  auto left = dynamic_cast<hm::dec_leaf<int> *>(tree.get_left());
  ASSERT_NE(left, nullptr);
  EXPECT_EQ(left->get_entity(), l_entity);

  auto right = dynamic_cast<hm::dec_leaf<int> *>(tree.get_right());
  ASSERT_NE(right, nullptr);
  EXPECT_EQ(right->get_entity(), r_entity);

  EXPECT_TRUE(tree.is_full());
}


}

