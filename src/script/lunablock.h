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
