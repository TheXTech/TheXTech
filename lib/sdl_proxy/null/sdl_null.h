/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef STD_NULL_H
#define STD_NULL_H

#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstring>

#ifndef UNUSED // To avoid IDE-side errors
#define UNUSED(x) (void)x
#endif

typedef int64_t Sint64;
typedef uint64_t Uint64;
typedef int32_t Sint32;
typedef uint32_t Uint32;
typedef int16_t Sint16;
typedef uint16_t Uint16;
typedef int8_t Sint8;
typedef uint8_t Uint8;

#define SDL_IMPORT(func) static constexpr auto& SDL_ ## func = func;
#define SDL_IMPORT_MATH(func) inline double SDL_ ## func(double arg) { return std::func(arg); }

#define SDL_INLINE inline
#define SDL_FORCE_INLINE static inline

inline const char* SDL_strstr(const char* haystack, const char* needle)
{
    return strstr(haystack, needle);
}

inline char* SDL_strstr(char* haystack, const char* needle)
{
    return strstr(haystack, needle);
}

inline size_t SDL_strlcpy(char* dst, const char* src, size_t maxlen)
{
    size_t srclen = strlen(src);
    if(maxlen > 0)
    {
        size_t len = srclen < maxlen - 1 ? srclen : maxlen - 1;
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return srclen;
}

inline double SDL_pow(double x, double y)
{
    return std::pow(x, y);
}

#endif // #ifndef STD_NULL_H
