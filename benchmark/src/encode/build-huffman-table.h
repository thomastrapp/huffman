#include "benchmark/benchmark.h"

#include <vector>
#include <cstdint>
#include <utility>
#include <memory>
#include <unordered_map>

#include "hm/common.h"
#include "hm/encode.h"

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
  while( state.KeepRunning() )
  {
    std::unordered_map<uint64_t, hm::code_type> table;
    hm::build_huffman_table(get_tree(), table);
  }
}

BENCHMARK(BM_BuildHuffmanTable);


}

