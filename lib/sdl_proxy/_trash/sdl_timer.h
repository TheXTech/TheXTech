/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef SDL_TIMER_HHHHHH
#define SDL_TIMER_HHHHHH

#if defined(PGE_MIN_PORT) && !defined(__WII__)
#include "core/null/sdl_null.h"
#else
#include "core/sdl/sdl_sdl_timer.h"
#endif

#ifndef SDL_timer_h_
extern uint32_t SDL_GetTicks();
#endif
extern uint64_t SDL_GetMicroTicks();

#endif // #ifndef SDL_TIMER_HHHHHH
