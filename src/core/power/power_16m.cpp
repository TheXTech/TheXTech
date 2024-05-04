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

#include <nds.h>
#include "sdl_proxy/sdl_timer.h"

#include "core/power.h"

#include <Logger/logger.h>


namespace XPower
{

static uint32_t s_last_power_check = -10000;
static uint32_t s_sys_status;

StatusInfo devicePowerStatus()
{
    uint32_t ticks = SDL_GetTicks();

    if(ticks - s_last_power_check > 10000)
    {
        s_last_power_check = ticks;
        s_sys_status = getBatteryLevel();
    }

    StatusInfo res;

    res.power_status = StatusInfo::POWER_DISCHARGING;
    res.power_level = 0.5f;

    if(s_sys_status & 0x80)
        res.power_status = StatusInfo::POWER_CHARGING;
    else if(s_sys_status == 0xf)
        res.power_level = 1.0f;
    else if(s_sys_status == 0xb)
        res.power_level = 0.75f;
    else if(s_sys_status == 0x7)
        res.power_level = 0.5f;
    else if(s_sys_status == 0x3)
        res.power_level = 0.25f;
    else if(s_sys_status == 0x1)
        res.power_level = 0.1f;
    else
        res.power_status = StatusInfo::POWER_UNKNOWN;

    return res;
}

}
