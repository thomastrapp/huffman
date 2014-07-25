#ifndef HLP_GET_TEST_DATA_H
#define HLP_GET_TEST_DATA_H

#include <vector>
#include <numeric>
#include <limits>
#include <cstring>
#include <cstdint>
#include <cassert>


namespace hlp
{

template<typename entity_type>
std::vector<std::vector<uint8_t>> get_test_data()
{
  auto char_data = hlp::get_test_data<char>();

  std::vector<std::vector<uint8_t>> inputs;
  for(const auto& string : char_data)
  {
    std::vector<uint8_t> line;
    for(auto c : string)
    {
      for(unsigned int i = 0; i < sizeof(entity_type); ++i)
      {
        line.push_back(c - i);
      }
    }

    while( line.size() % sizeof(entity_type) != 0 )
    {
      line.push_back(0);
    }

    assert(line.size() % sizeof(entity_type) == 0);

    inputs.push_back(line);
  }

  return inputs;
}

template<>
std::vector<std::vector<uint8_t>> get_test_data<char>()
{
  std::vector<std::vector<uint8_t>> inputs;

  std::vector<uint8_t> all_characters(256, '\0');
  std::iota(all_characters.begin(), all_characters.end(), std::numeric_limits<char>::min());
  assert(all_characters.at(255) == std::numeric_limits<char>::max());

  const char * str_inputs[] = {
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.,#*!$%&/()={[]}<>;:_~",
    "ABBBBBBC",
    "AAABBBCCC",
    "AAAAABBBBB",
    "AAAAAAAABBBBBBBCCCCCCDDDDDEEEEFFFGGH",
    "AAAAAAAAABBBBBBBBCCCCCCCDDDDDDEEEEEFFFFGGGHHI",
    "AAAAAAAAAABBBBBBBBBCCCCCCCCDDDDDDDEEEEEEFFFFFGGGGHHHIIJ",
    "AAAAAAAAAAABBBBBBBBBBCCCCCCCCCDDDDDDDDEEEEEEEFFFFFFGGGGGHHHHIIIJJK",
    "AAAAABBHNNNJRRRQQQQTTTZ",
    "Sed ut perspiciatis, unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam eaque ipsa, quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt, explicabo. Nemo enim ipsam voluptatem, quia voluptas sit, aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos, qui ratione voluptatem sequi nesciunt, neque porro quisquam est, qui dolorem ipsum, quia dolor sit amet consectetur adipisci[ng] velit, sed quia non numquam [do] eius modi tempora inci[di]dunt, ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit, qui in ea voluptate velit esse, quam nihil molestiae consequatur, vel illum, qui dolorem eum fugiat, quo voluptas nulla pariatur? "
    "At vero eos et accusamus et iusto odio dignissimos ducimus, qui blanditiis praesentium voluptatum deleniti atque corrupti, quos dolores et quas molestias excepturi sint, obcaecati cupiditate non provident, similique sunt in culpa, qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio, cumque nihil impedit, quo minus id, quod maxime placeat, facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet, ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat"
  };

  for(auto str : str_inputs)
  {
    inputs.push_back(std::vector<uint8_t>(str, str + strlen(str)));
  }

  std::vector<uint8_t> line;
  for(char c = 'A'; c <= 'Z'; ++c)
  {
    inputs.push_back(line);
    line.push_back(c);
  }
  inputs.push_back(line);

  inputs.push_back(all_characters);

  return inputs;
}


} // namespace hlp


#endif // HLP_GET_TEST_DATA_H

