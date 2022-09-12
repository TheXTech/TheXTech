#pragma once
#ifndef STD_NULL_H
#define STD_NULL_H

#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstring>

#define TXT_FORCE_INLINE inline

#define TXT_IMPORT(x) using :: x

inline void TXT_assert_release(bool arg)
{
    UNUSED(arg);
}

inline void TXT_assert_debug(bool arg)
{
    UNUSED(arg);
}

namespace XStd
{

using atomic_t = volatile int;

inline void AtomicSet(atomic_t* loc, int value)
{
    *loc = value;
}

inline int AtomicGet(const atomic_t* loc)
{
    return *loc;
}

inline size_t strlcpy(char* dst, const char* src, size_t maxlen)
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

}; // namespace XStd

#endif // #ifndef STD_NULL_H
