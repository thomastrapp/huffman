#ifndef HM_ENCODE_TREE_H
#define HM_ENCODE_TREE_H

#include <memory>
#include <utility>


namespace hm
{


/// An enc_node serves as a common base class for the huffman tree. Since all
/// members of a huffman tree get weighted on a frequency field it is
/// stored in the base.
template<typename entity_type>
class enc_node
{
public:
  explicit enc_node(size_t frequency)
  : freq(frequency)
  {
  }

  size_t get_frequency() const
  {
    return this->freq;
  }

  virtual ~enc_node()
  {
  }

private:
  const size_t freq;
};


/// Only leaves carry the actual data (entity_type).
template<typename entity_type>
class enc_leaf : public hm::enc_node<entity_type>
{
public:
  enc_leaf(size_t frequency, const entity_type& entity)
  : enc_node<entity_type>(frequency),
    ent(entity)
  {
  }

  entity_type get_entity() const
  {
    return this->ent;
  }

private:
  const entity_type ent;
};


/// A huffman tree has two children, either is another tree or a leaf.
/// The tree is self-managing. Losing the handle to the top of a tree
/// automatically propagates destruction to all children.
template<typename entity_type>
class enc_tree : public hm::enc_node<entity_type>
{
public:
  enc_tree(
    size_t frequency = 0,
    std::unique_ptr<enc_node<entity_type>> l = nullptr,
    std::unique_ptr<enc_node<entity_type>> r = nullptr
  )
  : enc_node<entity_type>(frequency),
    left(std::move(l)),
    right(std::move(r))
  {
  }

  // return a non-owning handle
  enc_node<entity_type> * get_left() const
  {
    return this->left.get();
  }

  enc_node<entity_type> * get_right() const
  {
    return this->right.get();
  }

private:
  enc_tree<entity_type>(const enc_tree<entity_type>&) = delete;
  enc_tree<entity_type>& operator=(const enc_tree<entity_type>&)
    = delete;

  std::unique_ptr<enc_node<entity_type>> left;
  std::unique_ptr<enc_node<entity_type>> right;
};


} // end namespace hm

#endif // HM_ENCODE_TREE_H
