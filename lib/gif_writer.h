#ifndef GIF_WRITER_HHHH
#define GIF_WRITER_HHHH

#include <stdio.h>   // for FILE*
#include <stdint.h>  // for integer typedefs

namespace GIF_H
{

struct GifWriter
{
    FILE* f;
    uint8_t* oldImage;
    bool firstFrame;
    long int delaypos;
};


}

#endif // GIF_WRITER_HHHH
