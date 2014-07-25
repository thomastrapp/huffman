#ifndef SP_FILE_EXISTS_H
#define SP_FILE_EXISTS_H

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace sp {

inline bool file_exists(const std::string& file_name)
{
  struct stat buf;
  return stat(file_name.c_str(), &buf) == 0;
}


}


#endif // SP_FILE_EXISTS_H
