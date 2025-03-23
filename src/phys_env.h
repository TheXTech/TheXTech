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
#ifndef PHYS_ENV_H
#define PHYS_ENV_H

#include <array>

#include "globals.h"

enum PhysEnv_Maze_Status
{
    MAZE_DIR_LEFT = 0,
    MAZE_DIR_UP = 1,
    MAZE_DIR_RIGHT = 2,
    MAZE_DIR_DOWN = 3,
    MAZE_CAN_EXIT = 4,
    MAZE_PLAYER_FLIP = 128,
    MAZE_DIR_FLIP_BIT = 2,
};

// applies maze physics to a location. modifies maze_index and effect_data as needed.
// indicates leaving the env by setting maze_index to 0.
void PhysEnv_Maze(Location_t& loc, vbint_t& maze_index, uint8_t& maze_state, int npc_A, int plr_A, int speedvar, std::array<bool, 4> controls);

// picks direction for a possible maze entry
void PhysEnv_Maze_PickDirection(const Location_t& loc, vbint_t maze_index, uint8_t& maze_state);

#endif // #ifndef PHYS_ENV_H
