#pragma once
#ifndef STD_SDL_H
#define STD_SDL_H

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_assert.h>

#define TXT_FORCE_INLINE SDL_FORCE_INLINE

#define TXT_assert_release SDL_assert_release
#define TXT_assert_debug SDL_assert

#define TXT_IMPORT(x) static constexpr auto& x = SDL_ ## x

namespace XStd
{

using atomic_t = SDL_atomic_t;

TXT_IMPORT(AtomicSet);
TXT_IMPORT(AtomicGet);
TXT_IMPORT(strlcpy);

inline uint32_t GetTicks()
{
    return SDL_GetTicks();
}

}; // namespace XStd

#endif // #ifndef STD_SDL_H
