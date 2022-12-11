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
#ifndef SDL_SDL_TYPES_H
#define SDL_SDL_TYPES_H

#if defined(SDLRPOXY_3DS) || defined(SDLRPOXY_NULL)
#include <stdint>

#ifndef UNUSED // To avoid IDE-side errors
#   define UNUSED(x) (void)x
#endif

typedef int64_t Sint64;
typedef uint64_t Uint64;
typedef int32_t Sint32;
typedef uint32_t Uint32;
typedef int16_t Sint16;
typedef uint16_t Uint16;
typedef int8_t Sint8;
typedef uint8_t Uint8;

#define SDL_INLINE inline
#define SDL_FORCE_INLINE static inline

#else
#   include <SDL2/SDL_types.h>
#endif

#endif // #ifndef SDL_SDL_TYPES_H
