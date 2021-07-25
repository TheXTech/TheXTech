/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef BLOCKS_H
#define BLOCKS_H

// Check is this a player filter block that matches a character and must be ignored in checks
bool BlockCheckPlayerFilter(int blockIdx, int playerIdx);

//! The block was hit by a player
void BlockHit(int A, bool HitDown = false, int whatPlayer = 0);
//! Shake the block up
void BlockShakeUp(int A);
//! Shake the block up
void BlockShakeUpPow(int A);
//! Shake the block down
void BlockShakeDown(int A);

void BlockHitHard(int A);
//! Destroy a block
void KillBlock(int A, bool Splode = true);
// update the frames for animated blocks
void BlockFrames();
//! Update the blocks
void UpdateBlocks();

//! turns all the blocks to coins and vice versa
void PSwitch(bool enabled);
//! drops coins and shakes all blocks on screen when player throws a POW block
void PowBlock();

#endif // BLOCKS_H
