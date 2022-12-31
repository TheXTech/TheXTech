/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "lunablock.h"
#include "collision.h"

#include "main/trees.h"

Block_t *BlocksF::Get(int index)
{
    return &Block[index];
}

int BlocksF::TestCollision(Player_t *pMobPOS, Block_t *pBlockPOS)
{
    return FindCollision(pMobPOS->Location, pBlockPOS->Location);
}

void BlocksF::SetAll(int type1, int type2)
{
    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type1)
            Block[i].Type = type2;
    }
}

void BlocksF::SwapAll(int type1, int type2)
{
    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type1)
            Block[i].Type = type2;
        else if(Block[i].Type == type2)
            Block[i].Type = type1;
    }
}

void BlocksF::ShowAll(int type)
{
    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type)
            Block[i].Invis = false;
    }
}

void BlocksF::HideAll(int type)
{
    for(int i = 1; i <= numBlock; i++)
    {
        if(Block[i].Type == type)
            Block[i].Invis = true;
    }
}

bool BlocksF::IsPlayerTouchingType(int type, int sought, Player_t *demo)
{
    //    Block* blocks = Blocks::GetBase();
    double playerX = demo->Location.X - 0.20;
    double playerY = demo->Location.Y - 0.20;
    double playerX2 = demo->Location.X + demo->Location.Width + 0.20;
    double playerY2 = demo->Location.Y + demo->Location.Height + 0.20;

    for(Block_t* block : treeBlockQuery(demo->Location, SORTMODE_NONE))
    {
        if(block->Type == type)
        {
            if(playerX > block->Location.X + block->Location.Width ||
               playerX2 < block->Location.X  ||
               playerY > block->Location.Y + block->Location.Height ||
               playerY2 < block->Location.Y)
                continue;

            if(TestCollision(demo, block) == sought)
                return true;
        }
    }

    return false; // no collision
}
