#pragma once
#ifndef STD_NULL_H
#define STD_NULL_H

#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstring>

#define SDL_IMPORT(func) static constexpr auto& SDL_ ## func = func;
#define SDL_IMPORT_MATH(func) inline double SDL_ ## func(double arg) { return std::func(arg); }

#define SDL_FORCE_INLINE inline

inline void SDL_assert_release(bool arg)
{
    UNUSED(arg);
}

inline void SDL_assert(bool arg)
{
    UNUSED(arg);
}

using SDL_atomic_t = volatile int;

inline void SDL_AtomicSet(SDL_atomic_t* loc, int value)
{
    *loc = value;
}

inline int SDL_AtomicGet(const SDL_atomic_t* loc)
{
    return *loc;
}

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
