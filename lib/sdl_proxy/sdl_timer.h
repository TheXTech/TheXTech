/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef SDL_SDL_TIMER_H
#define SDL_SDL_TIMER_H

#ifndef SDLRPOXY_NULL

#include <SDL2/SDL_timer.h>

#else

#include <cstdint>

//#ifdef __WII__
//#define SDL_GetTicks SDL_GetTicks_Wii_
//#endif

#ifndef SDL_timer_h_
extern uint32_t SDL_GetTicks();
#endif

inline void SDL_Delay(int x)
{
    (void)(x);
}

#endif

extern uint64_t SDL_GetMicroTicks();

#endif // #ifndef SDL_SDL_TIMER_H
