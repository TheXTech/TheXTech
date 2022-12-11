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
