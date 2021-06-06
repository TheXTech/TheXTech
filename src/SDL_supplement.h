#ifndef SDL_SUPPLEMENT_H
#define SDL_SUPPLEMENT_H

#include <cstdint>

// include any platform-specific information
#ifdef __3DS__
#include "3ds/SDL_supplement_3DS.h"
#endif

typedef unsigned int uint;

typedef struct _SDL_Point {
    int x;
    int y;
} SDL_Point;

// defined in a platform-specific file
extern uint32_t SDL_GetMicroTicks(void);
extern uint32_t SDL_GetTicks(void);

const uint SDL_FLIP_NONE = 0;
const uint SDL_FLIP_HORIZONTAL = 1;
const uint SDL_FLIP_VERTICAL = 2; // bitwise combos

#define SDL_INLINE inline

typedef int64_t Sint64;
typedef uint8_t Uint8;
typedef uint32_t Uint32;
typedef bool SDL_bool;

#define SDL_memset memset
#define SDL_memcpy memcpy
#define SDL_fabs fabs
#define SDL_strcpy strcpy
#define SDL_strlcpy strlcpy
#define SDL_strcasecmp strcasecmp
#define SDL_strtol strtol
#define SDL_fmod fmod
#define SDL_floor floor
#define SDL_ceil ceil

inline void SDL_assert_release(bool arg) { (void)arg; };
inline void SDL_assert(bool arg) { (void)arg; };

#endif // SDL_SUPPLEMENT_H
