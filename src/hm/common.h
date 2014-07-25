#ifndef HM_COMMON_H
#define HM_COMMON_H

#include <memory>
#include <vector>
#include <map>
#include <cstdint>
#include <unordered_map>
#include <iterator>
#include <cassert>
#include <algorithm>
#include <limits>
#include <type_traits>

#include "hm/exception.h"

namespace hm
{

/// The huffman meta data.
/// A description of the binary layout.
struct meta
{
  meta()
  : version(0),
    tree_last_bits(0),
    data_last_bits(0),
    entity_size(0),
    entity_count(0),
    tree_byte_count(0),
    data_byte_count(0)
  {
  }

  typedef uint8_t  version_type;
  typedef uint8_t  last_bits_type;
  typedef uint8_t  entity_size_type;
  typedef uint32_t entity_count_type;
  typedef uint32_t tree_count_type;
  typedef uint64_t data_count_type;

  // The version number of the binary layout
  version_type version;

  // The number of valid bits in the last tree byte
  last_bits_type tree_last_bits;

  // The number of valid bits in the last data byte
  last_bits_type data_last_bits;

  // The size of an entity from the input sequence, in bytes
  entity_size_type entity_size;

  // The number of entities (number of leaves)
  entity_count_type entity_count;

  // The number of bytes in the tree section
  tree_count_type tree_byte_count;

  // The number of bytes in the data section
  data_count_type data_byte_count;
};


/// Each element represents a bit in a huffman code.
typedef std::vector<bool> code_type;


/// The maximum number of shifts we can do in a byte without overflow.
/// (Avoid the magic number 7 popping up everywhere in the code)
const hm::meta::last_bits_type max_shifts_in_byte
  = std::numeric_limits<uint8_t>::digits - 1U;


/// Isolate the bit at pos in byte.
/// pos is an offset from the most significant bit.
inline uint8_t get_bit(uint8_t byte, uint8_t pos)
{
  assert(pos <= hm::max_shifts_in_byte);

  // Omitting the final cast generates a warning with gcc -Wconversion
  // if target_type is smaller than int:
  // > conversion to ‘target_type’ from ‘int’ may alter its value
  // (arithmetic operations are always promoted to int if operands are
  // smaller than int)
  // The warning does not apply because the shift can never outgrow
  // target_type.
  return
    byte
    &
    static_cast<uint8_t>(
      1U << static_cast<uint8_t>(hm::max_shifts_in_byte - pos)
    )
  ;
}


/// Set the bit at pos in byte to 1.
/// pos is an offset from the most significant bit.
inline uint8_t set_bit(uint8_t byte, uint8_t pos)
{
  assert(pos <= hm::max_shifts_in_byte);
  return
    byte
    |
    static_cast<uint8_t>(
      1U << static_cast<uint8_t>(hm::max_shifts_in_byte - pos)
    )
  ;
}


/// Extract target_type from input stream.
///
/// in_begin is passed as a reference: if in_begin is a single pass input
/// iterator, increments will not have an effect on the call site, letting the
/// original iterator stay at the previous element. There's no better way
/// around this problem than passing by reference.
///
/// Parameters:
///   target_type:
///     The type of data to extract. Must be integral.
///   in_begin, in_end:
///     A range of input iterators pointing to bytes.
///
/// Throws hm::invalid_layout if in_end was reached before target_type was
/// filled fully.
///
/// Side effects:
///   * in_begin incremented by sizeof(entity_type) or
///   * if an exception was thrown: in_begin will point to in_end
template<
  typename target_type,
  typename in_iter,
  typename = typename std::enable_if<
    std::is_integral<target_type>::value
  >::type
>
target_type decode_type(in_iter& in_begin, in_iter in_end)
{
  uint8_t bytes[sizeof(target_type)] = {0};

  for(size_t i = 0; i < sizeof(target_type); ++i)
  {
    if( in_begin == in_end )
      throw hm::invalid_layout("unexpected end");
    bytes[i] = *in_begin++;
  }

  return *reinterpret_cast<target_type *>(bytes);
}


/// Convert source_type to bytes.
///
/// Parameters:
///   source:
///     Data to be encoded. Must be integral and unsigned.
///   out:
///     An output iterator expecting bytes.
template<
  typename source_type,
  typename out_iter,
  typename = typename std::enable_if<
    std::is_integral<source_type>::value
  >::type
>
inline void encode_type(source_type source, out_iter out)
{
  uint8_t * byte_wise = reinterpret_cast<uint8_t *>(&source);
  for(size_t i = 0; i < sizeof(source); ++i)
  {
    *out++ = byte_wise[i];
  }
}


/// Convenience template alias to check if an iterator is a ForwardIterator
template<typename iterator>
using is_forward_iterator =
  std::is_base_of<
    std::forward_iterator_tag,
    typename std::iterator_traits<iterator>::iterator_category
  >
;


/// Comparison class for nodes from a huffman tree.
///
/// Comparison of nodes in a huffman tree is based on the frequency the
/// tree or leaf represents.
template<
  typename node
>
class node_freq_compare
{
public:
  bool operator()(
    const std::unique_ptr<node>& left,
    const std::unique_ptr<node>& right
  ) const
  {
    if( left.get() != nullptr && right.get() != nullptr )
    {
      return left->get_frequency() > right->get_frequency();
    }
    else
    {
      return false;
    }
  }
};


} // end namespace hm

#endif // HM_COMMON_H
