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

#ifndef ENUMBLKID_HHH
#define ENUMBLKID_HHH

enum BLKID
{
    BLKID_SPIN_BLOCK = 90,
    BLKID_RED_BRICK = 186,
    BLKID_GRY_BRICK = 457,

    // these can be expanded into user-modifiable ranges in the future.
    // currently the "CONV" blocks can't be placed or modified in any way by the user
    BLKID_CONVEYOR_L_START = 701,
    BLKID_CONVEYOR_L_CONV = 701,
    BLKID_CONVEYOR_L_END = 701,

    BLKID_CONVEYOR_R_START = 702,
    BLKID_CONVEYOR_R_CONV = 702,
    BLKID_CONVEYOR_R_END = 702,
};


#endif // ENUMBLKID_HHH
