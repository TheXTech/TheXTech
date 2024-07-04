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

#pragma once

#ifndef ENUM_PLAYER_EFFECT_HHH
#define ENUM_PLAYER_EFFECT_HHH

#include "global_constants.h"

enum PlayerEffect : vbint_t
{
    PLREFF_NORMAL = 0,
    PLREFF_TURN_BIG = 1,
    PLREFF_TURN_SMALL = 2,
    PLREFF_WARP_PIPE = 3,
    PLREFF_TURN_FIRE = 4,
    PLREFF_TURN_LEAF = 5,
    PLREFF_RESPAWN = 6,
    PLREFF_WARP_DOOR = 7,
    PLREFF_WAITING = 8,
    PLREFF_NO_COLLIDE = 9,
    PLREFF_PET_INSIDE = 10,
    PLREFF_TURN_STATUE = 11,
    PLREFF_TURN_HEAVY = 12,
    PLREFF_TURN_ICE = 41,
    PLREFF_FIRE_TO_BIG = 227,
    PLREFF_ICE_TO_BIG = 228,
    PLREFF_STONE = 500,
};


#endif // ENUM_PLAYER_EFFECT_HHH
