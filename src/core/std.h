// #include "core/std.h"
// XStd::assert_release

#ifndef STD_HHHHHH
#define STD_HHHHHH

#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstring>

#define TXT_FORCE_INLINE inline
#define UNUSED(x) (void)x

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

inline void assert_release(bool arg)
{
    UNUSED(arg);
}

inline void assert_debug(bool arg)
{
    UNUSED(arg);
}

uint32_t GetTicks();

inline uint32_t SwapLE32(uint32_t x)
{
    return x;
}

template<class value_t>
inline bool min(value_t x, value_t y)
{
    return x < y ? x : y;
}

template<class value_t>
inline bool max(value_t x, value_t y)
{
    return x > y ? x : y;
}

using ::free;

using ::memset;
using ::memcpy;
using ::memcmp;
using ::strlcpy;
using ::strdup;
using ::strlen;
using ::strtol;
using ::atoi;
using ::atof;
using ::sscanf;
using ::strcasecmp;
using ::strncasecmp;
using ::strstr;

using ::fabs;
using ::floor;
using ::ceil;
using ::pow;

}; // namespace XStd

#endif // #ifndef STD_HHHHHH
