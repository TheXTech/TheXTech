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
