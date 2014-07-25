#ifndef SP_PROGRAM_OPTIONS_VALIDATE_H
#define SP_PROGRAM_OPTIONS_VALIDATE_H

#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>

namespace sp {

/// Dummy struct for parameter entity-size.
/// We do this to be able to overload boost::program_option's valdiation
/// function.
struct pov_entity_size
{
  explicit pov_entity_size(unsigned int size)
  : entity_size(size)
  {
  }

  unsigned int entity_size;
};


/// Validate pov_entity_size or throw validation_error.
void validate(
  boost::any& v,
  const std::vector<std::string>& values,
  sp::pov_entity_size *,
  int
)
{
  namespace po = boost::program_options;

  po::validators::check_first_occurrence(v);
  const std::string& s = po::validators::get_single_string(values);

  unsigned int size = 0;
  try
  {
    size = boost::lexical_cast<unsigned int>(s);
  }
  catch(const boost::bad_lexical_cast&)
  {
    size = 0;
  }

  switch( size )
  {
    case 1:
    case 2:
    case 4:
    case 8:
      break;
    default:
      throw po::validation_error(po::validation_error::invalid_option_value);
  }

  v = boost::any(sp::pov_entity_size(size));
}


}

#endif // SP_PROGRAM_OPTIONS_VALIDATE_H
