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

#include <ctime>
#include <cstdio>
#include <3ds.h>

#include "../sdl_timer.h"

uint32_t __stacksize__ = 0x00020000;

uint64_t originalTime = -1u;
uint64_t originalMicroTime = -1u;

osTimeRef_s tr;

uint64_t SDL_GetMicroTicks()
{
    if(originalMicroTime == -1u)
    {
        originalMicroTime = svcGetSystemTick();
        tr = osGetTimeRef();
    }

    return (uint64_t)(svcGetSystemTick() - originalTime) * 1000000 / tr.sysclock_hz;
}
