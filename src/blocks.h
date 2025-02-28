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
//! Safely destroy a block
void SafelyKillBlock(int A);
//! Unsafely destroy a block; returns true if this procedure should be resumed. (this procedure will become private)
bool KillBlock(int A, bool Splode = true);
// update the frames for animated blocks
void BlockFrames();
//! Update the blocks; returns true if this procedure should be resumed.
bool UpdateBlocks();

//! turns all the blocks to coins and vice versa (now private, because it can interrupt the gameplay logic)
// bool PSwitch(bool enabled);
//! drops coins and shakes all blocks on screen when player throws a POW block
void PowBlock();

// Extra: restore the normal music after a switch ends
void SwitchEndResumeMusic();

#endif // BLOCKS_H
