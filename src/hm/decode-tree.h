#ifndef HM_DECODE_TREE_H
#define HM_DECODE_TREE_H

#include <memory>
#include <utility>
#include <stdexcept>


namespace hm
{


/// A dec_node serves as a common base class for the huffman decode tree.
/// When decoding, we dont care about frequencies, this is why there are two
/// versions of each node type:
///   enc_node,dec_node   enc_leaf,dec_leaf   enc_tree,dec_tree;
template<typename entity_type>
class dec_node
{
public:
  dec_node()
  {}

  virtual ~dec_node()
  {}
};


/// Only leaves carry the actual data (entity_type).
template<typename entity_type>
class dec_leaf : public hm::dec_node<entity_type>
{
public:
  explicit dec_leaf(const entity_type& entity)
  : dec_node<entity_type>(),
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
class dec_tree : public hm::dec_node<entity_type>
{
public:
  explicit dec_tree(
    std::unique_ptr<dec_node<entity_type>> l = nullptr,
    std::unique_ptr<dec_node<entity_type>> r = nullptr
  )
  : left(std::move(l)),
    right(std::move(r))
  {
  }

  // return a non-owning handle
  dec_node<entity_type> * get_left() const
  {
    return this->left.get();
  }

  dec_node<entity_type> * get_right() const
  {
    return this->right.get();
  }

  // construct a new child
  template<typename node_type, typename ...Args>
  void emplace_next_child(Args&& ...args)
  {
    if( this->left.get() == nullptr )
      this->left.reset(new node_type(std::forward<Args>(args)...));
    else if( this->right.get() == nullptr )
      this->right.reset(new node_type(std::forward<Args>(args)...));
    else
      throw std::out_of_range("cannot emplace child, already full");
  }

  // take ownership of another node
  void set_next_child_and_own(
    std::unique_ptr<hm::dec_node<entity_type>> node
  )
  {
    if( this->left.get() == nullptr )
      this->left.swap(node);
    else if( this->right.get() == nullptr )
      this->right.swap(node);
    else
      throw std::out_of_range("cannot add child, already full");
  }

  bool is_full() const
  {
    return
         this->left.get() != nullptr
      && this->right.get() != nullptr
    ;
  }

private:
  dec_tree<entity_type>(const dec_tree<entity_type>&) = delete;
  dec_tree<entity_type>& operator=(const dec_tree<entity_type>&)
    = delete;

  std::unique_ptr<dec_node<entity_type>> left;
  std::unique_ptr<dec_node<entity_type>> right;
};


} // end namespace hm

#endif // HM_DECODE_TREE_H
