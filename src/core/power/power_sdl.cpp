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

#include <SDL2/SDL_power.h>
#include <SDL2/SDL_timer.h>

#include "core/power.h"


namespace XPower
{

static uint32_t s_last_power_check = -10000;
static StatusInfo s_recent_status;

static StatusInfo s_devicePowerStatus_REAL()
{
    int pct;
    SDL_PowerState status = SDL_GetPowerInfo(nullptr, &pct);

    StatusInfo res;

    switch(status)
    {
    case SDL_POWERSTATE_NO_BATTERY:
        res.power_status = StatusInfo::POWER_WIRED;
        break;
    case SDL_POWERSTATE_ON_BATTERY:
        res.power_status = StatusInfo::POWER_DISCHARGING;
        break;
    case SDL_POWERSTATE_CHARGING:
        res.power_status = StatusInfo::POWER_CHARGING;
        break;
    case SDL_POWERSTATE_CHARGED:
        res.power_status = StatusInfo::POWER_CHARGED;
        break;
    case SDL_POWERSTATE_UNKNOWN:
    default:
        res.power_status = StatusInfo::POWER_UNKNOWN;
        break;
    }

    if(pct == -1 && res.power_status == StatusInfo::POWER_DISCHARGING)
        res.power_status = StatusInfo::POWER_UNKNOWN;

    if(pct != -1)
        res.power_level = pct / 100.0f;

    return res;
}

StatusInfo devicePowerStatus()
{
    uint32_t ticks = SDL_GetTicks();

    if(ticks - s_last_power_check > 10000)
    {
        s_last_power_check = ticks;
        s_recent_status = s_devicePowerStatus_REAL();
    }

    return s_recent_status;
}

}
