#include "benchmark/benchmark.h"

#include <vector>
#include <cstdint>
#include <utility>
#include <memory>
#include <unordered_map>

#include "hm/common.h"
#include "hm/encode.h"

namespace old {

template<
  typename entity_type
>
void
orig_build_huffman_table(
  const hm::enc_node<entity_type> * node,
  std::unordered_map<entity_type, hm::code_type>& table,
  hm::code_type prefix = hm::code_type()
)
{
  if( auto tree = dynamic_cast<const hm::enc_tree<entity_type> *>(node) )
  {
    hm::code_type left_prefix = prefix;
    left_prefix.push_back(0);
    old::orig_build_huffman_table(tree->get_left(), table, left_prefix);

    hm::code_type right_prefix = prefix;
    right_prefix.push_back(1);
    old::orig_build_huffman_table(tree->get_right(), table, right_prefix);
  }
  else if( auto leaf = dynamic_cast<const hm::enc_leaf<entity_type> *>(node) )
  {
    table[leaf->get_entity()] = prefix;
  }
  // else: node == nullptr, ignore
}


}

namespace {

/// create a huffman tree with unique leaves
std::unique_ptr<hm::enc_node<uint64_t>> build_tree(size_t num_leaves)
{
  std::vector<uint64_t> vec(num_leaves);
  for(uint64_t i = 0; i < vec.size(); ++i)
  {
    vec[i] = i;
  }
  uint8_t * begin = reinterpret_cast<uint8_t *>(vec.data());
  uint8_t * end = reinterpret_cast<uint8_t *>(vec.data() + vec.size());
  auto tree = hm::build_huffman_tree<uint64_t>(begin, end);
  return std::move(tree);
}

hm::enc_node<uint64_t> * get_tree()
{
  static std::unique_ptr<hm::enc_node<uint64_t>> tree = build_tree(1ULL << 16);
  return tree.get();
}

static void BM_BuildHuffmanTable(benchmark::State& state)
{
  state.PauseTiming();
  auto tree = get_tree();
  state.ResumeTiming();
  while( state.KeepRunning() )
  {
    std::unordered_map<uint64_t, hm::code_type> table;
    hm::code_type prefix;
    hm::build_huffman_table(tree, table, prefix);
  }
}
BENCHMARK(BM_BuildHuffmanTable);


static void BM_BuildHuffmanTableOriginal(benchmark::State& state)
{
  state.PauseTiming();
  auto tree = get_tree();
  state.ResumeTiming();
  while( state.KeepRunning() )
  {
    std::unordered_map<uint64_t, hm::code_type> table;
    ::old::orig_build_huffman_table(tree, table);
  }
}
BENCHMARK(BM_BuildHuffmanTableOriginal);


}

