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

#pragma once
#ifndef POWER_HHHH
#define POWER_HHHH

#include "numeric_types.h"

namespace XPower
{

// information about device or a bound input method
struct StatusInfo
{
    typedef enum _PowerStatus
    {
        POWER_DISABLED = -1,
        POWER_UNKNOWN,
        POWER_WIRED,
        POWER_DISCHARGING,
        POWER_CHARGING,
        POWER_CHARGED
    } PowerStatus;
    PowerStatus power_status = POWER_DISABLED;
    numf_t power_level = 1;
    const char *info_string = nullptr;
};


/*!
 * \brief Finds current device power status
 */
StatusInfo devicePowerStatus();


} // namespace XPower

#endif // POWER_HHHH
