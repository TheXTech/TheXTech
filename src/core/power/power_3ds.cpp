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

#include <3ds.h>

#include "core/power.h"


namespace XPower
{

StatusInfo devicePowerStatus()
{
    uint8_t percent;
    uint8_t charging;
    bool plugged;

    MCUHWC_GetBatteryLevel(&percent);
    PTMU_GetBatteryChargeState(&charging);
    PTMU_GetAdapterState(&plugged);

    StatusInfo res;

    if(charging)
        res.power_status = StatusInfo::POWER_CHARGING;
    else if(plugged)
        res.power_status = StatusInfo::POWER_CHARGED;
    else
        res.power_status = StatusInfo::POWER_DISCHARGING;

    res.power_level = percent / 100.f;

    return res;
}

}
