#ifndef HM_EXCEPTION_H
#define HM_EXCEPTION_H

#include <stdexcept>

namespace hm
{


class invalid_layout : public std::runtime_error
{
public:
  explicit invalid_layout(const char * msg)
  : std::runtime_error(msg)
  {
  }
};


} // end namespace hm

#endif // HM_EXCEPTION_H
