#ifndef HM_DECODE_H
#define HM_DECODE_H

#include <memory>
#include <cstdint>
#include <limits>
#include <cassert>
#include <vector>
#include <stack>
#include <cstring>

#include "util/make-unique.h"
#include "hm/common.h"
#include "hm/decode-tree.h"
#include "hm/exception.h"

namespace hm
{

/// Decode the huffman tree.
///
/// Parameters:
///   in_begin, in_end:
///     A range of input iterators pointing to bytes containing an encoded
///     huffman tree
///   entities:
///     the leaves of the tree, left-first and bottom-up
///   md:
///     description of the binary layout
///
/// Throws hm::invalid_layout.
/// Returns a managed dec_tree containing byte-vectors.
template<
  typename in_iter
>
std::unique_ptr<hm::dec_tree<std::vector<uint8_t>>>
decode_tree(
  in_iter in_begin,
  in_iter in_end,
  const std::vector<std::vector<uint8_t>>& entities,
  const hm::meta& md
)
{
  typedef std::vector<uint8_t> entity_type;

  std::stack<std::unique_ptr<hm::dec_tree<entity_type>>> stk;
  hm::meta::tree_count_type bytes = 0;
  hm::meta::entity_count_type entities_applied = 0;

  std::vector<entity_type>::const_iterator next_leaf = std::begin(entities);

  while( bytes < md.tree_byte_count )
  {
    if( in_begin == in_end )
      throw hm::invalid_layout("missing data in tree section");

    const uint8_t byte = static_cast<uint8_t>(*in_begin++);

    // Calculate the maximum bit-offset we're allowed to read in this byte
    // (Find a better way to do this)
    hm::meta::last_bits_type max_pos = hm::max_shifts_in_byte;
    const bool is_last_byte = ( bytes == md.tree_byte_count - 1 );
    if( is_last_byte
        && md.tree_last_bits
        && md.tree_last_bits <= hm::max_shifts_in_byte )
    {
      max_pos = static_cast<uint8_t>(md.tree_last_bits - 1);
    }

    hm::meta::last_bits_type pos = 0;
    while( pos <= max_pos )
    {
      // 1 represents a leaf
      if( hm::get_bit(byte, pos) )
      {
        if( stk.empty() )
          throw hm::invalid_layout("unexpected leaf");

        if( next_leaf == std::end(entities) )
          throw hm::invalid_layout("missing leaf");

        stk.top()->emplace_next_child<hm::dec_leaf<entity_type>>(*next_leaf++);
        entities_applied++;

        // append nodes from top of the stack to the next node on top of the stack
        // until the top node has a free place for a new child
        while( stk.top()->is_full() && stk.size() > 1 )
        {
          // implicit conversion from dec_tree to dec_node
          std::unique_ptr<hm::dec_node<entity_type>> node = std::move(stk.top());
          stk.pop();

          if( stk.top()->is_full() )
            throw hm::invalid_layout("invalid tree (branch full)");

          stk.top()->set_next_child_and_own(std::move(node));
        }
      }
      // 0 represents a branch
      else
      {
        stk.push(util::make_unique<hm::dec_tree<entity_type>>());
      }

      pos++;
    }

    bytes++;
  }

  if( entities_applied != md.entity_count )
    throw hm::invalid_layout("missing entities");

  // should never happen
  if( stk.empty() )
    throw hm::invalid_layout("missing node");

  return std::move(stk.top());
}


/// Decode the binary layout description.
///
/// Parameters:
///   in_begin, in_end:
///     A range of input iterators pointing to bytes containing an encoded hm::meta
///
/// Throws hm::invalid_layout if in_end was reached before hm::meta was filled fully.
/// Returns hm::meta, the binary layout description.
template<
  typename in_iter
>
hm::meta decode_meta_data(in_iter in_begin, in_iter in_end)
{
  // We can't just read and write structs, because their padding is
  // implementation defined, therefore leading to possible differences
  // in the struct's size between compilers and even compiler versions.
  // So we read each member directly.

  hm::meta md;
  md.version         = hm::decode_type<hm::meta::version_type>(in_begin, in_end);
  md.tree_last_bits  = hm::decode_type<hm::meta::last_bits_type>(in_begin, in_end);
  md.data_last_bits  = hm::decode_type<hm::meta::last_bits_type>(in_begin, in_end);
  md.entity_size     = hm::decode_type<hm::meta::entity_size_type>(in_begin, in_end);
  md.entity_count    = hm::decode_type<hm::meta::entity_count_type>(in_begin, in_end);
  md.tree_byte_count = hm::decode_type<hm::meta::tree_count_type>(in_begin, in_end);
  md.data_byte_count = hm::decode_type<hm::meta::data_count_type>(in_begin, in_end);

  return md;
}


/// Decode entities.
///
/// Parameters:
///   in_begin, in_end:
///     A range of input iterators pointing to bytes containing encoded entities.
///   md:
///     The binary layout.
///
/// Throws hm::invalid_layout if in_end is reached before all entities have been
/// read (as described by the binary layout).
/// Returns a vector containing entities (leaves of a huffman tree),
/// which are represented by byte-vectors.
template<
  typename in_iter
>
std::vector<std::vector<uint8_t>>
decode_entities(in_iter in_begin, in_iter in_end, const hm::meta& md)
{
  std::vector<std::vector<uint8_t>> entities(
    md.entity_count,
    std::vector<uint8_t>(md.entity_size, 0)
  );

  for(hm::meta::entity_count_type i = 0; i < md.entity_count; ++i)
  {
    for(hm::meta::entity_size_type c = 0; c < md.entity_size; ++c)
    {
      if( in_begin == in_end )
        throw hm::invalid_layout("too few entities");
      entities.at(i).at(c) = static_cast<uint8_t>(*in_begin++);
    }
  }

  return entities;
}


/// Decode the corpus.
///
/// Parameters:
///   in_begin, in_end:
///     A range of input iterators pointing to bytes containing the encoded corpus.
///   tree:
///     A non-owning handle to the decoded huffman tree.
///   md:
///     The binary layout description.
///   out:
///     An output iterator accepting decoded bytes (=the original input)
///
/// Throws hm::invalid_layout on unexpected or missing input.
template<
  typename in_iter,
  typename out_iter
>
void decode_data(
  in_iter in_begin,
  in_iter in_end,
  const hm::dec_tree<std::vector<uint8_t>> * tree,
  const hm::meta& md,
  out_iter out
)
{
  typedef std::vector<uint8_t> entity_type;
  hm::meta::data_count_type bytes = 0;

  auto walker = dynamic_cast<const hm::dec_node<entity_type> *>(tree);

  while( bytes < md.data_byte_count )
  {
    if( in_begin == in_end )
      throw hm::invalid_layout("missing data in data section");

    uint8_t byte = static_cast<uint8_t>(*in_begin++);

    // Calculate the maximum bit-offset we're allowed to read in this byte
    // (This is uneccessarily complex)
    hm::meta::last_bits_type max_pos = hm::max_shifts_in_byte;
    const bool is_last_byte = ( bytes == md.data_byte_count - 1 );
    if( is_last_byte
        && md.data_last_bits
        && md.data_last_bits <= hm::max_shifts_in_byte )
    {
      max_pos = static_cast<uint8_t>(md.data_last_bits - 1);
    }

    hm::meta::last_bits_type pos = 0;
    while( pos <= max_pos )
    {
      if( auto branch = dynamic_cast<const hm::dec_tree<entity_type> *>(walker) )
        // traverse the tree right on 1; left on 0
        walker = hm::get_bit(byte, pos) ? branch->get_right() : branch->get_left();
      else
        throw hm::invalid_layout("invalid sequence");

      if( auto leaf = dynamic_cast<const hm::dec_leaf<entity_type> *>(walker) )
      {
        // output all bytes from the entity
        for( auto e : leaf->get_entity() )
          *out++ = e;

        // reset the walker by pointing it back to the root of the tree
        walker = dynamic_cast<const hm::dec_node<entity_type> *>(tree);
      }

      pos++;
    }

    bytes++;
  }

  if( bytes != md.data_byte_count )
    throw hm::invalid_layout("missing bytes in data section");
}


/// Decode the binary layout.
/// Calls decode_entities, decode_tree and finally decode_data.
///
/// Parameters:
///   md:
///     The description of the binary layout.
///   in_begin, in_end:
///     A range of input iterators pointing to bytes containing the binary layout.
///   out:
///     An output iterator accepting decoded bytes (=the original input)
template<
  typename in_iter,
  typename out_iter
>
void decode(const hm::meta& md, in_iter in_begin, in_iter in_end, out_iter out)
{
  auto entities = hm::decode_entities(in_begin, in_end, md);
  if( hm::is_forward_iterator<in_iter>::value )
  {
    // decode_entities throws if we reached end prematurely,
    // meaning we can safely advance without accidentally
    // skipping end
    std::advance(in_begin, md.entity_size * md.entity_count);
  }

  auto tree = hm::decode_tree(in_begin, in_end, entities, md);
  if( hm::is_forward_iterator<in_iter>::value )
  {
    // same case with decode_tree
    std::advance(in_begin, md.tree_byte_count);
  }

  hm::decode_data(in_begin, in_end, tree.get(), md, out);
}


} // end namespace hm

#endif // HM_DECODE_H
