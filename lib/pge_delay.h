#ifndef PGE_DELAY_H
#define PGE_DELAY_H

#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#   define PGE_Delay(x) emscripten_sleep(x)
#elif defined(__3DS__)
#   define PGE_Delay(x) UNUSED(x)
#else
#   define PGE_Delay(x) SDL_Delay(x)
#endif

#endif // PGE_DELAY_H
