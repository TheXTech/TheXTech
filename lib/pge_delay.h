#ifndef PGE_DELAY_H
#define PGE_DELAY_H

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#   define PGE_Delay(x) emscripten_sleep(x)
#elif defined(__3DS__)
#	include <3ds.h>
#	define PGE_Delay(x) svcSleepThread((uint64_t)x*1000000ull)
#	define PGE_Nano_Delay(x) svcSleepThread((uint64_t)x)
#else
#	include <SDL2/SDL_timer.h>
#   define PGE_Delay(x) SDL_Delay(x)
#endif

#endif // PGE_DELAY_H
