/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef SDL_STDINC_HHHHHH
#define SDL_STDINC_HHHHHH

#if defined(PGE_MIN_PORT) && !defined(__WII__)
#include "core/null/sdl_null_stdinc.h"
#else
#include "core/sdl/sdl_sdl_stdinc.h"
#endif

#include <cstdint>

#ifdef SDL_min
#   undef SDL_min
#endif

#ifdef SDL_max
#   undef SDL_max
#endif

#ifdef SDL_SwapLE32
#   undef SDL_SwapLE32
#endif

#ifdef SDL_SwapBE32
#   undef SDL_SwapBE32
#endif

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

SDL_IMPORT(strlcpy)
SDL_IMPORT(strstr)

SDL_IMPORT(pow)

SDL_IMPORT_MATH(fabs)
SDL_IMPORT_MATH(floor)
SDL_IMPORT_MATH(ceil)

#endif // #ifndef SDL_STDINC_HHHHHH
