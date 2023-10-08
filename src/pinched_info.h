/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef PINCHED_INFO_H
#define PINCHED_INFO_H

// structure used to store information about whether a player / NPC is being crushed by blocks
struct PinchedInfo_t
{
    unsigned Bottom1   : 2;
    unsigned Left2     : 2;
    unsigned Top3      : 2;
    unsigned Right4    : 2;
    unsigned Moving    : 2;
    bool     MovingLR  : 1;
    bool     MovingUD  : 1;

    // players only: frame counter to use classic (strict) pinched death condition
    unsigned Strict    : 4;

    inline PinchedInfo_t()
    {
        reset_non_strict();
        Strict = 0;
    }

    inline void reset_non_strict()
    {
        Bottom1 = 0;
        Left2 = 0;
        Top3 = 0;
        Right4 = 0;
        Moving = 0;
        MovingLR = false;
        MovingUD = false;
    }
};

#endif // #ifndef PINCHED_INFO_H
