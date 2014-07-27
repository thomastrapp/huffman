#ifndef HM_ENCODE_H
#define HM_ENCODE_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#include <limits>
#include <iterator>
#include <memory>
#include <unordered_map>

#include "util/make-unique.h"
#include "ds/priority-queue.h"

#include "hm/exception.h"
#include "hm/encode-tree.h"
#include "hm/common.h"


namespace hm
{

/// Encode the binary layout description.
///
/// Parameters:
///   md:
///     The description of the binary layout.
///   out:
///     An output iterator expecting bytes.
template<
  typename out_iter
>
void encode_meta_data(const hm::meta& md, out_iter out)
{
  hm::encode_type(md.version, out);
  hm::encode_type(md.tree_last_bits, out);
  hm::encode_type(md.data_last_bits, out);
  hm::encode_type(md.entity_size, out);
  hm::encode_type(md.entity_count, out);
  hm::encode_type(md.tree_byte_count, out);
  hm::encode_type(md.data_byte_count, out);
}


/// Build the frequency table.
///
/// Parameters:
///   in_begin, in_end:
///     A range of input iterators pointing to bytes.
///
/// Return a table of mappings between entities and their number of occurrences in
/// the input sequence.
/// We use size_t for the counter since in the worst case we may have a sequence
/// that consists of only one unique byte.
template<
  typename entity_type,
  typename in_iter
>
std::unordered_map<entity_type, size_t>
build_frequency_table(in_iter in_begin, in_iter in_end)
{
  std::unordered_map<entity_type, size_t> table;

  while(in_begin != in_end)
  {
    // passing in_begin by reference
    auto entity = hm::decode_type<entity_type>(in_begin, in_end);
    table[entity] += 1;
  }

  return table;
}


/// Build the huffman tree.
///
/// Entities with high frequency get placed higher than entities with low frequency.
/// The higher the placement, the lesser the width of the resulting huffman code.
///
/// Parameters:
///   entity_type:
///     The byte-wise input will be interpreted as this type.
///     Example: If entity_type is uint64_t, 8 bytes will form a single entity.
///   in_begin, in_end:
///     A range of input iterators pointing to bytes. The amount of bytes
///     must be a multiple of sizeof(entity_type).
///
/// Returns a managed pointer to the top of the tree.
/// Returns nullptr on empty input.
template<
  typename entity_type,
  typename in_iter
>
std::unique_ptr<hm::enc_node<entity_type>>
build_huffman_tree(in_iter in_begin, in_iter in_end)
{
  typedef hm::enc_tree<entity_type> tree_type;
  typedef hm::enc_node<entity_type> node_type;
  typedef hm::enc_leaf<entity_type> leaf_type;

  const auto frequencies = hm::build_frequency_table<entity_type>(in_begin, in_end);

  // empty input
  if( frequencies.size() == 0 )
    return std::unique_ptr<tree_type>(nullptr);

  // only one distinct letter in input
  if( frequencies.size() == 1 )
  {
    const auto& f = frequencies.begin();
    return util::make_unique<tree_type>(
      f->second,   // frequency
      util::make_unique<leaf_type>(
        f->second, // frequency
        f->first   // entity
      )
    );
  }

  // std::priority_queue only returns a const_reference to top,
  // to prevent users messing with its internal state.
  // This however makes it impossible to move unique_ptrs out of the
  // queue. This is why I use my own version here.
  ds::priority_queue<
    std::unique_ptr<node_type>,
    std::vector<std::unique_ptr<node_type>>,
    hm::node_freq_compare<node_type>
  > trees;

  for(const auto& f : frequencies)
  {
    trees.push(
      util::make_unique<leaf_type>(f.second, f.first)
    );
  }

  // pop the two nodes with lowest frequency. Combine them into a new tree
  // with added frequencies. Push the tree back into the queue. Repeat this
  // until you have only one top node left.
  // Nodes with higher frequency will be further to the top than nodes with
  // lower frequency.
  while( trees.size() > 1 )
  {
    auto left = std::move(trees.top());
    trees.pop();

    auto right = std::move(trees.top());
    trees.pop();

    auto tree = util::make_unique<tree_type>(
      left->get_frequency() + right->get_frequency(),
      std::move(left),
      std::move(right)
    );

    trees.push(std::move(tree));
  }

  return std::move(trees.top());
}


/// Recursively build a huffman table from a huffman tree.
///
/// A huffman code for an entity is the path taken from the top of the tree
/// down to the leaf containing the entity, appending 0 for left branches,
/// 1 for right branches.
///
/// Parameters:
///   node:
///     A non-owning handle to a huffman tree.
///
/// Returns a table mapping entities to their encoded representation.
template<
  typename entity_type
>
void
build_huffman_table(
  const hm::enc_node<entity_type> * node,
  std::unordered_map<entity_type, hm::code_type>& table,
  hm::code_type& prefix
)
{
  if( auto tree = dynamic_cast<const hm::enc_tree<entity_type> *>(node) )
  {
    hm::code_type left_prefix = prefix;
    left_prefix.push_back(0);
    hm::build_huffman_table(tree->get_left(), table, left_prefix);

    prefix.push_back(1);
    hm::build_huffman_table(tree->get_right(), table, prefix);
  }
  else if( auto leaf = dynamic_cast<const hm::enc_leaf<entity_type> *>(node) )
  {
    table[leaf->get_entity()] = prefix;
  }
  // else: node == nullptr, ignore
}


/// Encode the corpus.
///
/// Parameters:
///   in_begin, in_end:
///     A range of input iterators pointing to bytes.
///   tree:
///     A non-owning handle to a huffman tree.
///   out:
///     An output iterator expecting bytes.
///   md:
///     The description of the binary layout.
template<
  typename entity_type,
  typename in_iter,
  typename out_iter
>
void encode_data(
  in_iter in_begin,
  in_iter in_end,
  const hm::enc_node<entity_type> * tree,
  out_iter out,
  hm::meta& md
)
{
  assert(tree != nullptr);

  // my stdlib (gnu) uses an identity hash function for trivial types
  // which will just cast entity_type to size_t.
  // (See _Cxx_hashtable_define_trivial_hash in functional_hash.h)
  std::unordered_map<entity_type, hm::code_type> table;
  hm::code_type prefix;
  hm::build_huffman_table<entity_type>(tree, table, prefix);

  uint8_t byte = 0;

  while( in_begin != in_end )
  {
    // passing in_begin by reference
    auto entity = hm::decode_type<entity_type>(in_begin, in_end);

    // get the huffman code
    const auto& code = table.at(entity);

    // fill byte with bits from left to right
    for(const auto& bit : code)
    {
      if( bit )
      {
        byte = hm::set_bit(byte, md.data_last_bits);
      }

      // if byte is full
      if( md.data_last_bits >= hm::max_shifts_in_byte )
      {
        *out++ = byte;
        md.data_byte_count++;
        byte = 0;
        md.data_last_bits = 0;
      }
      else
      {
        md.data_last_bits++;
      }
    }
  }

  // if we didn't fill the last byte fully
  if( md.data_last_bits > 0 )
  {
    *out++ = byte;
    md.data_byte_count++;
  }
}


/// Encode the tree recursively. This function is not meant to be called
/// directly, see encode_tree.
///
/// Due to the recursion we have no means of detecting the last written bit,
/// so we return the last byte and let the caller handle the last part.
///
/// Parameters:
///   node:
///     A non-owning handle to a huffman tree.
///   out:
///     An output iterator expecting bytes.
///   md:
///     The description of the binary layout.
///
/// Returns the byte that is currently being filled.
template<
  typename entity_type,
  typename out_iter
>
uint8_t encode_tree_recursive(
  const hm::enc_node<entity_type> * node,
  out_iter out,
  hm::meta& md,
  uint8_t byte = 0
)
{
  if( node == nullptr )
    return byte;

  // if byte is full
  if( md.tree_last_bits > hm::max_shifts_in_byte )
  {
    *out++ = byte;
    md.tree_byte_count++;
    byte = 0;
    md.tree_last_bits = 0;
  }

  if( auto tree = dynamic_cast<const hm::enc_tree<entity_type> *>(node) )
  {
    // write 0
    md.tree_last_bits++;
    byte = encode_tree_recursive(tree->get_left(), out, md, byte);
    byte = encode_tree_recursive(tree->get_right(), out, md, byte);
  }
  else // is leaf
  {
    // write 1
    byte = hm::set_bit(byte, md.tree_last_bits);
    md.tree_last_bits++;
  }

  return byte;
}


/// Encode the tree.
///
/// Convenience wrapper for encode_tree_recursive.
///
/// Parameters:
///   node:
///     A non-owning handle to a huffman tree.
///   out:
///     An output iterator expecting bytes.
///   md:
///     The description of the binary layout.
template<
  typename entity_type,
  typename out_iter
>
void encode_tree(
  const hm::enc_node<entity_type> * node,
  out_iter out,
  hm::meta& md
)
{
  auto last_byte = encode_tree_recursive<entity_type>(node, out, md);

  if( md.tree_last_bits > 0 )
  {
    *out++ = last_byte;
    md.tree_byte_count++;
  }
}


/// Encode the entities.
///
/// Recursively traverses the tree and encodes all leaves, left-first and bottom-up.
///
/// Parameters:
///   node:
///     A non-owning handle to a huffman tree.
///   out:
///     An output iterator expecting bytes.
///   md:
///     The description of the binary layout.
template<
  typename entity_type,
  typename out_iter
>
void encode_entities(
  const hm::enc_node<entity_type> * node,
  out_iter out,
  hm::meta& md
)
{
  assert(md.entity_size == sizeof(entity_type));
  if( auto tree = dynamic_cast<const hm::enc_tree<entity_type> *>(node) )
  {
    hm::encode_entities(tree->get_left(), out, md);
    hm::encode_entities(tree->get_right(), out, md);
  }
  else if( auto leaf = dynamic_cast<const hm::enc_leaf<entity_type> *>(node) )
  {
    hm::encode_type(leaf->get_entity(), out);
    md.entity_count++;
  }
  // else: node == null, ignore
}


/// Transform an input sequence into huffman codes.
///
/// Parameters:
///   in_begin, in_end:
///     A range of input iterators pointing to bytes.
///   tree:
///     A non-owning handle to a huffman tree.
///   out:
///     An output iterator expecting bytes.
///
/// Returns a description of written binary data.
template<
  typename entity_type,
  typename in_iter,
  typename out_iter
>
hm::meta encode(
  in_iter in_begin,
  in_iter in_end,
  const hm::enc_node<entity_type> * tree,
  out_iter out
)
{
  hm::meta md;
  md.version = 10;
  md.entity_size = sizeof(entity_type);

  if( tree == nullptr )
    return md;

  hm::encode_entities(tree, out, md);
  hm::encode_tree(tree, out, md);
  hm::encode_data(in_begin, in_end, tree, out, md);

  return md;
}


} // end namespace hm

#endif // HM_ENCODE_H
