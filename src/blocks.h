/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef BLOCKS_H
#define BLOCKS_H

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
void PSwitch(bool Bool);
//! drops coins and shakes all blocks on screen when player throws a POW block
void PowBlock();

#endif // BLOCKS_H
