#include "md5tools.hpp"
#include "../Utils/files.h"


std::string md5::string_to_hash(const std::string &input)
{
    std::string out;
    md5::md5_t h;

    h.process((const void*)input.c_str(), (int)input.size());
    h.finish();

    out.resize(MD5_STRING_SIZE);

    h.get_string(&out[0]);

    return out;
}

std::string md5::file_to_hash(const std::string &filePath)
{
    char block[BLOCK_SIZE];
    std::string out;
    FILE *f = Files::utf8_fopen(filePath.c_str(), "rb");
    size_t got;

    if(!f)
        return out;

    md5::md5_t h;

    do
    {
        got = std::fread((void*)block, 1, 64, f);
        h.process((const void*)block, (int)got);
    } while(got == 64);

    h.finish();
    out.resize(MD5_STRING_SIZE);
    h.get_string(&out[0]);

    std::fclose(f);

    return out;
}

std::string md5::mem_to_hash(const void *data, size_t size)
{
    std::string out;
    md5::md5_t h;

    h.process((const void*)data, (int)size);
    h.finish();
    out.resize(MD5_STRING_SIZE);
    h.get_string(&out[0]);

    return out;
}

std::string md5::file_to_hashGC(const std::string &filePath)
{
    std::string out = file_to_hash(filePath);

    for(size_t i = 0; i < out.size(); ++i)
        out[i] = std::toupper(out[i]);

    return out;
}
