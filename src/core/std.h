#pragma once
#ifndef STD_HHHHHH
#define STD_HHHHHH

// these files contain self-implemented items, and the import macro

#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

#ifdef PGE_MIN_PORT
#include "core/null/std_null.h"
#else
#include "core/sdl/std_sdl.h"
#endif

#include <cstdint>

namespace XStd
{

#ifdef THEXTECH_BIG_ENDIAN
// based on SDL formula
inline uint32_t SwapLE32(uint32_t x)
{
    return static_cast<uint32_t>(((x << 24) | ((x << 8) & 0x00FF0000) |
                                ((x >> 8) & 0x0000FF00) | (x >> 24)));
}
#else
inline uint32_t SwapLE32(uint32_t x)
{
    return x;
}
#endif

template<class value_t>
inline value_t min(value_t x, value_t y)
{
    return x < y ? x : y;
}

template<class value_t>
inline value_t max(value_t x, value_t y)
{
    return x > y ? x : y;
}

uint32_t GetTicks();
uint64_t GetMicroTicks();

TXT_IMPORT(free);

TXT_IMPORT(memset);
TXT_IMPORT(memcpy);
TXT_IMPORT(memcmp);
TXT_IMPORT(strlcpy);
TXT_IMPORT(strdup);
TXT_IMPORT(strlen);
TXT_IMPORT(strtol);
TXT_IMPORT(atoi);
TXT_IMPORT(atof);
TXT_IMPORT(sscanf);
TXT_IMPORT(strcasecmp);
TXT_IMPORT(strncasecmp);
TXT_IMPORT(strstr);

TXT_IMPORT(fabs);
TXT_IMPORT(floor);
TXT_IMPORT(ceil);
TXT_IMPORT(pow);

};

#undef TXT_IMPORT

#endif // #ifndef STD_HHHHHH
