#ifndef SP_PROGRAM_OPTIONS_H
#define SP_PROGRAM_OPTIONS_H

#include <string>
#include <boost/program_options.hpp>

#include "sp/program-options-validate.h"


namespace sp {

class program_options
{
public:
  program_options(int argc, const char * argv[])
  : desc("Options"),
    vm()
  {
    namespace po = boost::program_options;

    // e.g.: huffman -s 4 -e original-file -o encoded-file
    //       huffman      -d encoded-file  -o original-file
    this->desc.add_options()
      ("help", "This help message")
      ("encode-file,e", po::value<std::string>(), "File to be encoded")
      ("decode-file,d", po::value<std::string>(), "File to be decoded")
      ("entity-size,s",
        po::value<sp::pov_entity_size>()->default_value(sp::pov_entity_size(1), "1"),
          "When encoding, interpret input in blocks of this size in bytes. "
          "Input file size must be a multiple of this size. Possible values: "
          "1, 2, 4, 8")
      ("output-file,o", po::value<std::string>(), "Output file. Must not exist.")
    ;

    po::store(po::parse_command_line(argc, argv, this->desc), this->vm);
    po::notify(this->vm);
  }

  bool contains(const char * key) const
  {
    return this->vm.count(key);
  }

  unsigned int get_entity_size() const
  {
    // vm[entity-size] will always be filled, since it has a default value
    return this->vm["entity-size"].as<sp::pov_entity_size>().entity_size;
  }

  template<typename value_type>
  value_type get(const char * key) const
  {
    return this->vm[key].as<value_type>();
  }

  void print(const char * program_name, std::ostream& out = std::cout) const
  {
    out << "Usage:\n"
        << "  Encode: " << program_name << " -e input-file -o output-file [-s 1|2|4|8]\n"
        << "  Decode: " << program_name << " -d input-file -o output-file\n\n";
    out << this->desc;
  }

  bool validate_or_print_error(std::ostream& out = std::cerr) const
  {
    if( this->contains("encode-file") && this->contains("decode-file") )
    {
      out << "Error: Supply either encode-file or decode-file, but not both\n";
      return false;
    }

    if( !this->contains("encode-file") && !this->contains("decode-file") )
    {
      out << "Error: Expecting either encode-file or decode-file\n";
      return false;
    }

    if( this->contains("decode-file")
        && this->contains("entity-size")
        && !this->vm["entity-size"].defaulted() )
    {
      out << "Error: entity-size may only be supplied when decoding\n";
      return false;
    }

    return true;
  }

private:
  boost::program_options::options_description desc;
  boost::program_options::variables_map vm;
};


}

#endif // SP_PROGRAM_OPTIONS_H

