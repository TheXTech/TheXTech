/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "globals.h"
#include "sorting.h"
#include "npc_traits.h"

#include <algorithm>

// these are now used only when saving levels
void qSortBlocksY(int min, int max)
{
    Block_t medBlock;
    int hi = 0;
    int lo = 0;
    int i = 0;
    if(min >= max)
        return;
    i = floor((max + min) / 2.0);
    medBlock = Block[i];
    Block[i] = Block[min];
    lo = min;
    hi = max;
    do
    {
        while(Block[hi].Location.Y >= medBlock.Location.Y)
        {
            hi -= 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            Block[lo] = medBlock;
            break;
        }
        Block[lo] = Block[hi];
        lo += 1;
        while(Block[lo].Location.Y < medBlock.Location.Y)
        {
            lo += 1;
            if(lo >= hi)
                break;
        }
        if(lo >= hi)
        {
            lo = hi;
            Block[hi] = medBlock;
            break;
        }
        Block[hi] = Block[lo];
    } while(true);
    qSortBlocksY(min, lo - 1);
    qSortBlocksY(lo + 1, max);
}

void qSortBlocksX(int min, int max)
{

    Block_t medBlock;
    int hi = 0;
    int lo = 0;
    int i = 0;
    if(min >= max)
        return;
    i = floor((max + min) / 2.0);
    medBlock = Block[i];
    Block[i] = Block[min];
    lo = min;
    hi = max;
    do
    {
        while(Block[hi].Location.X >= medBlock.Location.X)
        {
            hi -= 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            Block[lo] = medBlock;
            break;
        }
        Block[lo] = Block[hi];
        lo += 1;
        while(Block[lo].Location.X < medBlock.Location.X)
        {
            lo += 1;
            if(lo >= hi)
                break;
        }
        if(lo >= hi)
        {
            lo = hi;
            Block[hi] = medBlock;
            break;
        }
        Block[hi] = Block[lo];
    } while(true);
    qSortBlocksX(min, lo - 1);
    qSortBlocksX(lo + 1, max);
}

void qSortBackgrounds(int min, int max, bool use_x)
{
    if(min >= max)
        return;

    std::stable_sort(&Background[min], (&Background[max]) + 1,
    [use_x](const Background_t& a, const Background_t& b)
    {
        return a.SortPriority < b.SortPriority || (use_x && a.SortPriority == b.SortPriority && a.Location.X < b.Location.X);
    });

    // old code was acceptable but didn't make it easy to sort by SortPriority first, Location second
#if 0
    Background_t medBackground;
    double medBackgroundPri = 0.0;
    int hi = 0;
    int lo = 0;
    int i = 0;
    if(min >= max)
        return;
    i = floor((max + min) / 2.0);
    medBackground = Background[i];
    medBackgroundPri = BackGroundPri(i);
    Background[i] = Background[min];
    lo = min;
    hi = max;
    do
    {
        while(BackGroundPri(hi) >= medBackgroundPri)
        {

            hi -= 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            Background[lo] = medBackground;
            break;
        }
        Background[lo] = Background[hi];
        lo += 1;
        while(BackGroundPri(lo) < medBackgroundPri)
        {
            lo += 1;
            if(lo >= hi)
                break;
        }
        if(lo >= hi)
        {
            lo = hi;
            Background[hi] = medBackground;
            break;
        }
        Background[hi] = Background[lo];
    } while(true);
    qSortBackgrounds(min, lo - 1);
    qSortBackgrounds(lo + 1, max);
#endif
}

// deprecated by block quadtree

#if 0
void FindBlocks()
{
    int A = 0;
    int B = 0;
//    int C = 0;
    int curBlk = 0;
//    bool fBool = false;
    curBlk = 1;
    for(A = -FLBlocks; A <= FLBlocks; A++)
    {
        for(B = curBlk; B <= numBlock; B++)
        {
            if(Block[B].Location.X + Block[B].Location.Width >= A * 32)
            {
                curBlk = int(B);
                break;
            }
        }
        FirstBlock[A] = curBlk;
    }
    curBlk = numBlock;

    for(A = FLBlocks; A >= -FLBlocks; A--)
    {
//        fBool = false;
        for(B = curBlk; B >= 1; B--)
        {
            if(Block[B].Location.X <= A * 32)
            {
                curBlk = int(B);
                break;
            }
        }
        LastBlock[A] = curBlk;
    }
    // BlocksSorted = true;
}

void BlockSort()
{
    int A = 0;
    int B = 0;
    Block_t tempBlock;

    // Sort Sizable Blocks
    for(A = 1; A <= numBlock; A++)
    {
        if(BlockIsSizable[Block[A].Type])
        {
            for(B = 1; B < A; B++)
            {
                if(!BlockIsSizable[Block[B].Type])
                {
                    tempBlock = Block[A];
                    Block[A] = Block[B];
                    Block[B] = tempBlock;
                    break;
                }
            }
        }
    }

    for(A = 1; A <= numBlock; A++)
    {
        if(BlockIsSizable[Block[A].Type])
        {
            for(B = 1; B <= numBlock; B++)
            {
                if(BlockIsSizable[Block[B].Type])
                {
                    if(B != 1)
                    {
                        if(Block[A].Location.Y < Block[B].Location.Y && A > B)
                        {
                            tempBlock = Block[A];
                            Block[A] = Block[B];
                            Block[B] = tempBlock;
                        }
                        else if(Block[A].Location.Y > Block[B].Location.Y && A < B)
                        {
                            tempBlock = Block[A];
                            Block[A] = Block[B];
                            Block[B] = tempBlock;
                        }
                    }
                }
            }
        }
    }
}

void BlockSort2()
{
    int A = 0;
    int B = 0;
    Block_t tempBlock;
    bool sortAgain = false;
    do
    {
        sortAgain = false;
        for(A = 1; A <= numBlock; A++)
        {
            for(B = 1; B <= numBlock; B++)
            {
                if(B != A)
                {
                    if(Block[A].Location.Y < Block[B].Location.Y && A > B)
                    {
                        tempBlock = Block[A];
                        Block[A] = Block[B];
                        Block[B] = tempBlock;
                        sortAgain = true;
                    }
                }
            }
        }
    } while(sortAgain);
}

void BackgroundSort()
{

    int A = 0;
    int B = 0;
    Background_t tempBackground;
    bool sortAgain = false;

    do
    {
        sortAgain = false;
        for(A = 1; A <= numBackground; A++)
        {
            for(B = 1; B <= numBackground; B++)
            {
                if(B != A)
                {
                    if(BackGroundPri(A) < BackGroundPri(B) && A > B)
                    {
                        tempBackground = Background[A];
                        Background[A] = Background[B];
                        Background[B] = tempBackground;
                        sortAgain = true;
                    }
                }
            }
        }
    } while(sortAgain);
}
#endif

//! checks if a custom layer is set, returns -2, -1, +1, or +2 if so, 0 if not
int Background_t::GetCustomLayer() const
{
    switch(SortPriority & 0xf8)
    {
    case(0x08):
    case(0x18):
    case(0x28):
        return -2;
    case(0x30):
    case(0x48):
    case(0x98):
        return -1;
    case(0xA0):
    case(0xB8):
        return +1;
    case(0xC0):
        return +2;
    default:
        return 0;
    }
}

//! returns the custom offset of a BGO
int Background_t::GetCustomOffset() const
{
    return int((SortPriority) & 0x07) - 4;
}

//! update a background's priority based on its current type, custom layer, and custom offset
void Background_t::UpdateSortPriority()
{
    // check user-specified Z-layer
    int user_order = GetCustomLayer();

    // clear upper 5 bits
    SortPriority &= ~0xf8;

    // set upper 5 bits by user order
    if(user_order)
    {
        int user_offset = GetCustomOffset();

        if(user_order == -2 && user_offset < 0)
            SortPriority |= 0x08;
        else if(user_order == -2 && user_offset == 0)
            SortPriority |= 0x18;
        else if(user_order == -2)
            SortPriority |= 0x28;
        else if(user_order == -1 && user_offset < 0)
            SortPriority |= 0x30;
        else if(user_order == -1 && user_offset == 0)
            SortPriority |= 0x48;
        else if(user_order == -1)
            SortPriority |= 0x98;
        else if(user_order == +1 && user_offset <= 0)
            SortPriority |= 0xA0;
        else if(user_order == +1)
            SortPriority |= 0xB8;
        else
            SortPriority |= 0xC0;

        return;
    }

    // set upper 5 bits by Type-based order

    // PLANE_LVL_BGO_LOW
    // custom -2 with - offset is 0x08
    if(Type == 75 || Type == 76 || Type == 77 || Type == 78 || Type == 14)
        SortPriority |= 0x10;
    // custom -2 with 0 offset is 0x18
    else if(Type == 11 || Type == 12 || Type == 60 || Type == 61)
        SortPriority |= 0x20;
    // custom -2 with + offset is 0x28
    // PLANE_LVL_BGO_NORM
    // custom -1 with - offset is 0x30
    else if(Type == 168 || Type == 159 || Type == 172 || Type == 66 || Type == 158) // WATER FALLS
        SortPriority |= 0x38;
    else if(Type == 65 || Type == 26 || Type == 82 || Type == 83 || Type == 164 || Type == 165 || Type == 166 || Type == 167 || Type == 168 || Type == 169) // WATER
        SortPriority |= 0x40;
    // custom -1 with 0 offset is 0x48
    else if(Type == 79 || Type == 52)
        SortPriority |= 0x50;
    else if(Type == 66)
        SortPriority |= 0x58;
    // default is 0x60
    else if(Type == 1)
        SortPriority |= 0x68;
    else if(Type >= 129 && Type <= 131)
        SortPriority |= 0x70;
    else if(Type == 139 || Type == 140 || Type == 48)
        SortPriority |= 0x78;
    else if(Type == 70 || Type == 71 || Type == 72 || Type == 73 || Type == 74 || Type == 141)
        SortPriority |= 0x80;
    else if(Type == 87 || Type == 88 || Type == 92 || Type == 107 || Type == 105 || Type == 104) // Doors
        SortPriority |= 0x88;
    else if(Type == 99)
        SortPriority |= 0x90; // Always doors + 1
    // custom -1 with + offset is 0x98
    // PLANE_LVL_BGO_FG
    // custom +1 with - or 0 offset is 0xA0
    else if(Foreground[Type])
        SortPriority |= 0xA8;
    else if(Type == 65 || Type == 165)
        SortPriority |= 0xB0;
    // custom +1 with + offset is 0xB8
    // PLANE_LVL_BGO_TOP
    // custom +2 is 0xC0
    else
        SortPriority |= 0x60;
}

//! sets custom layer and offset bits for a bgo and updates its sort priority
void Background_t::SetSortPriority(int layer, int offset)
{
    SortPriority = 0;

    // set upper 5 bits by layer
    if(layer == -2)
        SortPriority = 0x18;
    else if(layer == -1)
        SortPriority = 0x48;
    else if(layer == +1)
        SortPriority = 0xA0;
    else if(layer == +2)
        SortPriority = 0xC0;

    // set lower 3 bits by offset
    uint8_t offset_bits = uint8_t(offset + 4) & 7;
    SortPriority |= offset_bits;

    UpdateSortPriority();
}


#if 0
double BackGroundPri(int A)
{
    double tempBackGroundPri = 0;


    if(Background[A].SortPriority > 0) // Custom priority per every BGO
        tempBackGroundPri = Background[A].SortPriority;
    // Lower Numbers get drawn first
    else if(Background[A].Type == 11 || Background[A].Type == 12 || Background[A].Type == 60 || Background[A].Type == 61)
        tempBackGroundPri = 20;
    else if(Background[A].Type == 65 || Background[A].Type == 26 || Background[A].Type == 82 || Background[A].Type == 83 || Background[A].Type == 164 || Background[A].Type == 165 || Background[A].Type == 166 || Background[A].Type == 167 || Background[A].Type == 168 || Background[A].Type == 169) // WATER

        tempBackGroundPri = 26;
    else if(Background[A].Type == 168 || Background[A].Type == 159 || Background[A].Type == 172 || Background[A].Type == 66 || Background[A].Type == 158) // WATER FALLS
        tempBackGroundPri = 25;
    else if(Background[A].Type == 75 || Background[A].Type == 76 || Background[A].Type == 77 || Background[A].Type == 78 || Background[A].Type == 14)
        tempBackGroundPri = 10;
    else if(Background[A].Type == 79 || Background[A].Type == 52)
        tempBackGroundPri = 30;
    else if(Background[A].Type == 70 || Background[A].Type == 71 || Background[A].Type == 72 || Background[A].Type == 73 || Background[A].Type == 74 || Background[A].Type == 141)
        tempBackGroundPri = 90;
    else if(Background[A].Type == 139 || Background[A].Type == 140 || Background[A].Type == 48)
        tempBackGroundPri = 80;
    else if(Background[A].Type == 65 || Background[A].Type == 165)
        tempBackGroundPri = 150;
    else if(Foreground[Background[A].Type])
        tempBackGroundPri = 125;
    else if(Background[A].Type == 66)
        tempBackGroundPri = 50;
    else if(Background[A].Type == 99)
        tempBackGroundPri = 99; // Always doors + 1
    else if(Background[A].Type == 87 || Background[A].Type == 88 || Background[A].Type == 92 || Background[A].Type == 107 || Background[A].Type == 105 || Background[A].Type == 104) // Doors
        tempBackGroundPri = 98;
    else if(Background[A].Type >= 129 && Background[A].Type <= 131)
        tempBackGroundPri = 76;
    else if(Background[A].Type == 1)
        tempBackGroundPri = 77;
    else
        tempBackGroundPri = 75;

    //tempBackGroundPri += Background[A].Location.X / 10000000.0;
    tempBackGroundPri += Background[A].uid / 100000000000.0;
    tempBackGroundPri += Background[A].zOffset;


    return tempBackGroundPri;
}
#endif

void NPCSort()
{
    int A = 0;
    int B = 0;
    NPC_t tempNPC;

    for(A = 1; A <= numNPCs; A++)
    {
        if(NPC[A]->IsACoin)
        {
            for(B = 1; B < A; B++)
            {
                if(!NPC[B]->IsACoin)
                {
                    tempNPC = NPC[A];
                    NPC[A] = NPC[B];
                    NPC[B] = tempNPC;
                    break;
                }
            }
        }
    }
}

// deprecated
#if 0
void FindSBlocks()
{
    int A = 0;
    sBlockNum = 0;
    for(A = 1; A <= numBlock; A++)
    {
        if(BlockIsSizable[Block[A].Type])
        {
            sBlockNum += 1;
            sBlockArray[sBlockNum] = A;
        }
    }
    qSortSBlocks(1, sBlockNum);
}

void qSortSBlocks(int min, int max)
{
    int medBlock = 0;
    int hi = 0;
    int lo = 0;
    int i = 0;
    if(min >= max)
        return;

    i = floor((max + min) / 2.0);
    medBlock = sBlockArray[i];
    sBlockArray[i] = sBlockArray[min];
    lo = min;
    hi = max;
    do
    {
        while(Block[sBlockArray[hi]].Location.Y >= Block[medBlock].Location.Y)
        {
            hi -= 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            sBlockArray[lo] = medBlock;
            break;
        }
        sBlockArray[lo] = sBlockArray[hi];
        lo += 1;
        while(Block[sBlockArray[lo]].Location.Y < Block[medBlock].Location.Y)
        {
            lo += 1;
            if(lo >= hi)
                break;
        }
        if(lo >= hi)
        {
            lo = hi;
            sBlockArray[hi] = medBlock;
            break;
        }
        sBlockArray[hi] = sBlockArray[lo];
    } while(true);
    qSortSBlocks(min, lo - 1);
    qSortSBlocks(lo + 1, max);
}
#endif

void qSortNPCsY(int min, int max)
{
    NPC_t medNPC;
    int hi = 0;
    int lo = 0;
    int i = 0;
    if(min >= max)
        return;
    i = floor((max + min) / 2.0);
    medNPC = NPC[i];
    NPC[i] = NPC[min];
    lo = min;
    hi = max;
    do
    {
        while(NPC[hi].Location.Y < medNPC.Location.Y)
        {
            hi -= 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            NPC[lo] = medNPC;
            break;
        }
        NPC[lo] = NPC[hi];
        lo += 1;
        while(NPC[lo].Location.Y >= medNPC.Location.Y)
        {
            lo += 1;
            if(lo >= hi)
                break;
        }
        if(lo >= hi)
        {
            lo = hi;
            NPC[hi] = medNPC;
            break;
        }
        NPC[hi] = NPC[lo];
    } while(true);
    qSortNPCsY(min, lo - 1);
    qSortNPCsY(lo + 1, max);
}

void UpdateBackgrounds()
{
    int A = 0;
    int B = 0;
    LastBackground = numBackground;
    MidBackground = 1;
    for(A = 1; A <= numBackground; A++)
    {
        if(Background[A].SortPriority >= Background_t::PRI_NORM_START)
        {
            for(B = A; B <= numBackground; B++)
            {
                if(Background[B].SortPriority >= Background_t::PRI_FG_START)
                {
                    LastBackground = B - 1;
                    break;
                }
            }
            break;
        }
    }
    MidBackground = A;
//    if(noUpdate == false)
//        Netplay::sendData "s" + std::to_string(numBackground) + LB;
}

// deprecated
#if 0
void qSortTempBlocksX(int min, int max)
{
    Block_t medBlock;
    int hi = 0;
    int lo = 0;
    int i = 0;

    if(min >= max)
        return;

    i = floor((max + min) / 2.0);
    medBlock = Block[i];
    Block[i] = Block[min];
    lo = min;
    hi = max;

    do
    {
        while(Block[hi].Location.X >= medBlock.Location.X)
        {
            hi -= 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            Block[lo] = medBlock;
            break;
        }
        Block[lo] = Block[hi];
        lo += 1;
        while(Block[lo].Location.X < medBlock.Location.X)
        {
            lo += 1;
            if(lo >= hi)
                break;
        }
        if(lo >= hi)
        {
            lo = hi;
            Block[hi] = medBlock;
            break;
        }
        Block[hi] = Block[lo];
    } while(true);

    qSortBlocksX(min, lo - 1);
    qSortBlocksX(lo + 1, max);
}
#endif
