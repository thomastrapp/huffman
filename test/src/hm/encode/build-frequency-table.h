#include <unordered_map>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <iostream>
#include <string>
#include <algorithm>

#include "gtest/gtest.h"

#include "hm/encode.h"


namespace {

namespace helper {

template<typename container_type>
bool is_container_equal(const container_type& left, const container_type& right)
{
  return
    left.size() == right.size() &&
    std::equal(
      std::begin(left),
      std::end(left),
      std::begin(right)
    );
}

}


TEST(HmBuildFrequencyTable, CountsDistinct)
{
  const char * input =
    "This is a test. "
    "Tests, testing, "
    "all the time. "
    "Lorem Ipsum. y.";
  size_t len = strlen(input);
  const char * end = input + len;

  std::stringstream in;
  in.write(input, len);
  EXPECT_EQ(in.tellp(), len);

  // build a table with input iterators
  std::unordered_map<char, size_t> table_from_stream = hm::build_frequency_table<char>(
    std::istreambuf_iterator<char>(in),
    std::istreambuf_iterator<char>()
  );

  // check if all input gone
  EXPECT_EQ(in.tellg(), len);

  // build a table with random access iterators
  std::unordered_map<char, size_t> table_from_rand_iter = hm::build_frequency_table<char>(
    input,
    end
  );

  EXPECT_TRUE(helper::is_container_equal(table_from_stream, table_from_rand_iter));

  // find all unique characters
  std::string unique_characters(input);
  std::sort(unique_characters.begin(), unique_characters.end());
  auto new_end = std::unique(unique_characters.begin(), unique_characters.end());
  unique_characters.erase(new_end, unique_characters.end());

  EXPECT_EQ(unique_characters.size(), table_from_stream.size());
  EXPECT_EQ(unique_characters.size(), table_from_rand_iter.size());

  for(auto c : unique_characters)
  {
    // count how often this character occures in input
    auto count = std::count(input, end, c);
    EXPECT_EQ(table_from_stream.at(c), count);
    EXPECT_EQ(table_from_rand_iter.at(c), count);
  }
}

TEST(HmBuildFrequencyTable, EmptyRange)
{
  int i = 0;
  auto int_map = hm::build_frequency_table<char>(&i, &i);
  EXPECT_EQ(int_map.size(), 0);

  int * n_ptr = nullptr;
  auto null_map = hm::build_frequency_table<char>(n_ptr, n_ptr);
  EXPECT_EQ(null_map.size(), 0);
}


}

