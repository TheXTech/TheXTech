#include "md5tools.hpp"
#include "../Utils/files.h"
#include <SDL2/SDL_rwops.h>


std::string md5::string_to_hash(const std::string &input)
{
    std::string out;
    md5::md5_t h;

    h.process((const void*)input.c_str(), (int)input.size());
    h.finish();

    out.resize(MD5_STRING_SIZE - 1);

    h.get_string(&out[0]);

    return out;
}

std::string md5::file_to_hash(const std::string &filePath)
{
    char block[BLOCK_SIZE];
    std::string out;
    SDL_RWops *f = Files::open_file(filePath, "rb");
    size_t got;

    if(!f)
        return out;

    md5::md5_t h;

    do
    {
        got = SDL_RWread(f, (void*)block, 1, 64);
        h.process((const void*)block, (int)got);
    } while(got == 64);

    h.finish();
    out.resize(MD5_STRING_SIZE - 1);
    h.get_string(&out[0]);

    SDL_RWclose(f);

    return out;
}

std::string md5::mem_to_hash(const void *data, size_t size)
{
    std::string out;
    md5::md5_t h;

    h.process((const void*)data, (int)size);
    h.finish();
    out.resize(MD5_STRING_SIZE - 1);
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
