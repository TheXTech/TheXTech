/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../sdl_timer.h"

#include <nds.h>

uint32_t curTime_ticks = 0u;
bool inited = false;

void s_updateTimer()
{
    if(!inited)
    {
        timerStart(1, ClockDivider_1024, 0, nullptr);
        timerStart(2, (ClockDivider)TIMER_CASCADE, 0, nullptr);
        inited = true;
    }

    curTime_ticks = TIMER_DATA(1) + (TIMER_DATA(2) << 16);
}

uint32_t SDL_GetTicks()
{
    s_updateTimer();

    return (uint64_t)curTime_ticks * 1000 / (BUS_CLOCK>>10);
}

uint64_t SDL_GetMicroTicks()
{
    s_updateTimer();

    return (uint64_t)curTime_ticks * 1000000 / (BUS_CLOCK>>10);
}

void SDL_Delay(int ms)
{
    uint64_t want = SDL_GetMicroTicks() + ms * 1000;

    while(SDL_GetMicroTicks() < want)
        swiDelay(0x126f); // ~120us
}
