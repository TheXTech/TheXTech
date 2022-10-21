#pragma once
#ifndef SDL_HHHHHH
#define SDL_HHHHHH

// these files contain self-implemented items, and the import macro

#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

#if defined(PGE_MIN_PORT) && !defined(__WII__)
#include "core/null/sdl_null.h"
#else
#include "core/sdl/sdl_sdl.h"
#endif

#include <cstdint>


#ifdef THEXTECH_BIG_ENDIAN
// based on SDL formula
inline uint32_t SDL_SwapLE32(uint32_t x)
{
    return static_cast<uint32_t>(((x << 24) | ((x << 8) & 0x00FF0000) |
                                ((x >> 8) & 0x0000FF00) | (x >> 24)));
}
inline uint32_t SDL_SwapBE32(uint32_t x)
{
    return x;
}
#else
inline uint32_t SDL_SwapLE32(uint32_t x)
{
    return x;
}
inline uint32_t SDL_SwapBE32(uint32_t x)
{
    return static_cast<uint32_t>(((x << 24) | ((x << 8) & 0x00FF0000) |
                                ((x >> 8) & 0x0000FF00) | (x >> 24)));
}
#endif


template<class value_t>
inline value_t SDL_min(value_t x, value_t y)
{
    return x < y ? x : y;
}

template<class value_t>
inline value_t SDL_max(value_t x, value_t y)
{
    return x > y ? x : y;
}

uint32_t SDL_GetTicks();
uint64_t SDL_GetMicroTicks();

SDL_IMPORT(free)

SDL_IMPORT(memset)
SDL_IMPORT(memcpy)
SDL_IMPORT(memcmp)
SDL_IMPORT(strdup)
SDL_IMPORT(strlen)
SDL_IMPORT(strtol)
SDL_IMPORT(atoi)
SDL_IMPORT(atof)
SDL_IMPORT(sscanf)
SDL_IMPORT(strcasecmp)
SDL_IMPORT(strncasecmp)

SDL_IMPORT_MATH(fabs)
SDL_IMPORT_MATH(floor)
SDL_IMPORT_MATH(ceil)

#undef SDL_IMPORT

#endif // #ifndef SDL_HHHHHH
