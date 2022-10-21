#pragma once
#ifndef SDL_SDL_H
#define SDL_SDL_H

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_assert.h>

#undef SDL_min
#undef SDL_max
#undef SDL_SwapLE32
#undef SDL_SwapBE32

#define SDL_IMPORT(func) /* func */
#define SDL_IMPORT_MATH(func) /* func */

#ifdef __WII__
#define SDL_GetTicks SDL_GetTicks_Wii_
#endif

SDL_IMPORT(AtomicSet)
SDL_IMPORT(AtomicGet)
SDL_IMPORT(strlcpy)
SDL_IMPORT(strstr)

SDL_IMPORT(pow)

#endif // #ifndef SDL_SDL_H
