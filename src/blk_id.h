/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "globals.h"

enum BLKID
{
    BLKID_HIT_BLOCK_S3 = 2,
    BLKID_HIT_BLOCK_S4 = 89,
    BLKID_HIT_BLOCK_S1 = 192,
    BLKID_BIG_HIT_BLOCK_S3 = 225,
    BLKID_RED_HIT_BLOCK = 280,

    BLKID_BRICK_S3 = 4,
    BLKID_BRICK_S1 = 188,
    BLKID_BLU_BRICK_S1 = 60,
    BLKID_BIG_BRICK_S3 = 226,
    BLKID_BOMB_DIRT_S2 = 293,
    BLKID_SPACE_BRICK = 526,
    BLKID_DIG_SAND = 370,

    BLKID_BOUNCE_BLOCK = 55,
    BLKID_SPIN_BLOCK = 90,
    BLKID_HITTABLE_BLOCK = 159,
    BLKID_RED_BRICK = 186,
    BLKID_GRY_BRICK = 457,

    BLKID_CONVEYOR_SWITCH = 169,
    BLKID_YEL_SWITCH = 170,
    BLKID_YEL_BLOCK_ON = 171,
    BLKID_YEL_BLOCK_OFF = 172,
    BLKID_BLU_SWITCH = 173,
    BLKID_BLU_BLOCK_ON = 174,
    BLKID_BLU_BLOCK_OFF = 175,
    BLKID_GRN_SWITCH = 176,
    BLKID_GRN_BLOCK_ON = 177,
    BLKID_GRN_BLOCK_OFF = 178,
    BLKID_RED_SWITCH = 179,
    BLKID_RED_BLOCK_ON = 180,
    BLKID_RED_BLOCK_OFF = 181,

    BLKID_SWITCH_ON = 282,
    BLKID_SWITCH_OFF = 283,

    BLKID_PET_HURT = 598,

    BLKID_ICE_COIN = 620,
    BLKID_ICE_PLANT = 621,

    BLKID_TO_CHAR1 = 622,
    BLKID_TO_CHAR2 = 623,
    BLKID_TO_CHAR3 = 624,
    BLKID_TO_CHAR4 = 625,
    BLKID_TO_CHAR5 = 631,

    BLKID_CHAR1_PASS = 626,
    BLKID_CHAR2_PASS = 627,
    BLKID_CHAR3_PASS = 628,
    BLKID_CHAR4_PASS = 629,
    BLKID_CHAR5_PASS = 632,

    // these can be expanded into user-modifiable ranges in the future.
    // currently the "CONV" blocks can't be placed or modified in any way by the user
    BLKID_CONVEYOR_L_START = 701,
    BLKID_CONVEYOR_L_CONV = 701,
    BLKID_CONVEYOR_L_END = 701,

    BLKID_CONVEYOR_R_START = 702,
    BLKID_CONVEYOR_R_CONV = 702,
    BLKID_CONVEYOR_R_END = 702,
};

inline bool BlockTypeSlippy(int type)
{
    return (type == 189 || type == 190 || type == 191
        || type == 270 || type == 271 || type == 272
        || type == 620 || type == 621 || type == 633
        || type == 634 || type == 241 || type == 242);
}

inline bool BlockTypeBreakable(int Type)
{
    return (Type == BLKID_BRICK_S3 || Type == BLKID_BLU_BRICK_S1 ||
       Type == BLKID_SPIN_BLOCK || Type == BLKID_BRICK_S1 ||
       Type == BLKID_BIG_BRICK_S3 || Type == BLKID_BOMB_DIRT_S2 ||
       Type == BLKID_SPACE_BRICK);
}

inline bool BlockIsBreakable(const Block_t& block)
{
    return BlockTypeBreakable(block.Type) && block.Special <= 0;
}

#endif // ENUMBLKID_HHH
