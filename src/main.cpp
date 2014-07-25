#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>

#include "hm/common.h"
#include "hm/encode.h"
#include "hm/decode.h"

// support
#include "sp/program-options.h"
#include "sp/file-exists.h"

int main(int argc, const char * argv[])
{
  std::ios_base::sync_with_stdio(false);

  try
  {
    sp::program_options po(argc, argv);

    if( po.contains("help") )
    {
      po.print(argv[0], std::cout);
      return EXIT_SUCCESS;
    }

    if( !po.validate_or_print_error(std::cerr) )
    {
      return EXIT_FAILURE;
    }

    std::string output_file_name = po.get<std::string>("output-file");
    if( sp::file_exists(output_file_name) )
    {
      std::cerr << "Error: output-file " << output_file_name
                << " already exists; refusing to overwrite.\n";
      return EXIT_FAILURE;
    }

    std::ofstream output_file(
      output_file_name,
      std::ios::binary
    );

    if( !output_file.good() )
    {
      std::cerr << "Error: failed creating output-file " << output_file_name
                << "\n";
      return EXIT_FAILURE;
    }

    //
    // DECODE FILE
    //
    if( po.contains("decode-file") )
    {
      std::ifstream decode_file(
        po.get<std::string>("decode-file"),
        std::ios::binary
      );

      if( !decode_file.good() )
      {
        std::cerr << "Error: failed opening decode-file "
                  << po.get<std::string>("decode-file") << "\n";
        return EXIT_FAILURE;
      }

      auto dec_iter = std::istreambuf_iterator<char>(decode_file);
      auto dec_iter_end = std::istreambuf_iterator<char>();

      hm::meta md_decoded = hm::decode_meta_data(dec_iter, dec_iter_end);

      hm::decode(
        md_decoded,
        dec_iter,
        dec_iter_end,
        std::ostreambuf_iterator<char>(output_file)
      );

      decode_file.close();
    }
    //
    // ENCODE FILE
    //
    else if( po.contains("encode-file") )
    {
      std::ifstream encode_file(
        po.get<std::string>("encode-file"),
        std::ios::binary
      );

      if( !encode_file.good() )
      {
        std::cerr << "Error: failed opening encode-file "
                  << po.get<std::string>("encode-file") << "\n";
        return EXIT_FAILURE;
      }

      unsigned int entity_size = po.get_entity_size();

      auto enc_iter = std::istreambuf_iterator<char>(encode_file);
      auto enc_iter_end = std::istreambuf_iterator<char>();
      auto out_iter = std::ostreambuf_iterator<char>(output_file);

      // write dummy data
      hm::meta md;
      hm::encode_meta_data(md, out_iter);

      // Because we cannot select a type based on runtime input, we either have
      // to use a macro or duplicate code.
      switch( entity_size )
      {
        default:
        case 1:
        {
          auto tree = hm::build_huffman_tree<uint8_t>(
            enc_iter,
            enc_iter_end
          );

          encode_file.seekg(0);
          md = hm::encode(enc_iter, enc_iter_end, tree.get(), out_iter);
          break;
        }
        case 2:
        {
          auto tree = hm::build_huffman_tree<uint16_t>(
            enc_iter,
            enc_iter_end
          );

          encode_file.seekg(0);
          md = hm::encode(enc_iter, enc_iter_end, tree.get(), out_iter);
          break;
        }
        case 4:
        {
          auto tree = hm::build_huffman_tree<uint32_t>(
            enc_iter,
            enc_iter_end
          );

          encode_file.seekg(0);
          md = hm::encode(enc_iter, enc_iter_end, tree.get(), out_iter);
          break;
        }
        case 8:
        {
          auto tree = hm::build_huffman_tree<uint64_t>(
            enc_iter,
            enc_iter_end
          );

          encode_file.seekg(0);
          md = hm::encode(enc_iter, enc_iter_end, tree.get(), out_iter);
          break;
        }
      }

      // overwrite dummy with actual meta data
      output_file.seekp(0);
      hm::encode_meta_data(md, out_iter);

      encode_file.close();
    }
    else
    {
      // program option validation failed
      // "should never happen"
      assert(false);
    }

    output_file.close();
    return EXIT_SUCCESS;
  }
  catch(const boost::program_options::validation_error& e)
  {
    std::cerr << "Error " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  // "should never happen"
  assert(false);
  return EXIT_FAILURE;
}
