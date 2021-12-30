#include <string>
#include "md5.h"

namespace md5
{
extern std::string string_to_hash(const std::string &input);
extern std::string file_to_hash(const std::string &filePath);
extern std::string file_to_hashGC(const std::string &filePath);
extern std::string mem_to_hash(const void *data, size_t size);
}
