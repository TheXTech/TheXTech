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
#ifndef LUNABLOCK_H
#define LUNABLOCK_H

#include "globals.h"

namespace BlocksF
{
Block_t* Get(int index);			// Get ptr to a block

int TestCollision(Player_t* pMobPOS, Block_t* pBlockPOS);

void SetAll(int type1, int type2);  // Set ID of all blocks of type 1 to type 2
void SwapAll(int type1, int type2); // Swap ID of all blocks of type 1 to type 2, and vice versa
void ShowAll(int type);				// Show all blocks of type
void HideAll(int type);				// Hide all blocks of type

bool IsPlayerTouchingType(int BlockType, int sought_collision, Player_t* pMobPOS); // See if player touching block of BlockType

}

#endif // LUNABLOCK_H
