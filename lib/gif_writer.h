#ifndef GIF_WRITER_HHHH
#define GIF_WRITER_HHHH

#include <stdint.h>  // for integer typedefs
#include <string>

struct SDL_RWops;

namespace GIF_H
{

using buf_t = std::basic_string<uint8_t>;

struct GifWriter
{
    SDL_RWops* f;
    uint8_t* oldImage;
    bool firstFrame;
    long int delaypos;
    buf_t buffer;
};


}

#endif // GIF_WRITER_HHHH
