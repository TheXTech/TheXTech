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

#include "../sdl_timer.h"

#include <nds.h>

uint32_t curTime = 0u;
bool inited = false;

void s_timerCallBack()
{
    curTime++;
}

uint32_t SDL_GetTicks()
{
    if(!inited)
    {
        timerStart(2, ClockDivider_1024, TIMER_FREQ_1024(1000), s_timerCallBack);
        inited = true;
    }

    return curTime;
}

uint64_t SDL_GetMicroTicks()
{
    return curTime * 1000;
}
