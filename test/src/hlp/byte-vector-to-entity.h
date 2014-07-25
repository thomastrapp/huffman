#ifndef HLP_BYTE_VECTOR_TO_ENTITY_H
#define HLP_BYTE_VECTOR_TO_ENTITY_H

#include <cstdint>
#include <cassert>
#include <vector>


namespace hlp {

  template<typename entity_type>
  entity_type byte_vector_to_entity(const std::vector<uint8_t>& bytes)
  {
    assert(sizeof(entity_type) == bytes.size());
    auto it = bytes.cbegin();
    entity_type entity = 0;
    for(unsigned int i = 0; i < sizeof(entity_type); ++i)
    {
      entity |= static_cast<entity_type>(bytes.at(i)) << (i * 8);
    }

    return entity;
  }


}

#endif // HLP_BYTE_VECTOR_TO_ENTITY_H

