#ifndef HLP_IS_SAME_META_H
#define HLP_IS_SAME_META_H

#include "hm/common.h"

namespace hlp
{

bool is_same_meta(const hm::meta& left, const hm::meta& right)
{
  return
    left.version         == right.version         &&
    left.tree_last_bits  == right.tree_last_bits  &&
    left.data_last_bits  == right.data_last_bits  &&
    left.entity_size     == right.entity_size     &&
    left.entity_count    == right.entity_count    &&
    left.tree_byte_count == right.tree_byte_count &&
    left.data_byte_count == right.data_byte_count
  ;
}


}

#endif // HLP_IS_SAME_META_H

