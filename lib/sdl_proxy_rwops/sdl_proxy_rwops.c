/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <malloc.h>
#include <SDL2/SDL_rwops.h>

SDL_RWops* SDL_AllocRW()
{
    return (SDL_RWops*)(malloc(sizeof(SDL_RWops)));
}

void SDL_FreeRW(SDL_RWops * area)
{
    free(area);
}

/* C definitions of standard RWops functions */
#define RWOPS_C_DEFINITIONS
#include <SDL2/_priv_rwops_funcs.h>
