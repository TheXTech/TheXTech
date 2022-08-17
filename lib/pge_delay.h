#ifndef PGE_DELAY_H
#define PGE_DELAY_H

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#   define PGE_Delay(x) emscripten_sleep(x)
#elif defined(__3DS__)
#   include <3ds.h>
#   define PGE_Delay(x) svcSleepThread((uint64_t)(x) * 1000000)
#elif defined(__WII__)
extern "C" void udelay(unsigned us);
#   define PGE_Delay(x) udelay((x) * 1000)
#elif defined(NO_SDL)
#   define PGE_Delay(x) UNUSED(x)
#else
#   define PGE_Delay(x) SDL_Delay(x)
#endif

#endif // PGE_DELAY_H
