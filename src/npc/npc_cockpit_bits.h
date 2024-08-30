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
#ifndef NPC_COCKPIT_BITS_H
#define NPC_COCKPIT_BITS_H

enum NPC_CockpitBits
{
    NPC_COCKPIT_DRIVING = (1 << 0),
    NPC_COCKPIT_LEFT = (1 << 1),
    NPC_COCKPIT_RIGHT = (1 << 2),
    NPC_COCKPIT_UP = (1 << 3),
    NPC_COCKPIT_DOWN = (1 << 4),
};

#endif // #ifndef NPC_COCKPIT_BITS_H
