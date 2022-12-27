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
#ifndef SDL_SDL_ASSERT_H
#define SDL_SDL_ASSERT_H

#if defined(SDLRPOXY_NULL)

#   include <assert.h>

#   define SDL_assert(x)         assert(x)

// Doesn't work as intended (trigger a fail on release builds), this is just a dummy
#   define SDL_assert_release(x) assert(x)

#else

#include <SDL2/SDL_assert.h>

#endif

#endif // #ifndef SDL_SDL_ASSERT_H
