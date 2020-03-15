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

#include "globals.h"
#include "sorting.h"

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
            hi = hi - 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            Block[lo] = medBlock;
            break;
        }
        Block[lo] = Block[hi];
        lo = lo + 1;
        while(Block[lo].Location.Y < medBlock.Location.Y)
        {
            lo = lo + 1;
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
            hi = hi - 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            Block[lo] = medBlock;
            break;
        }
        Block[lo] = Block[hi];
        lo = lo + 1;
        while(Block[lo].Location.X < medBlock.Location.X)
        {
            lo = lo + 1;
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

void qSortBackgrounds(int min, int max)
{
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

            hi = hi - 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            Background[lo] = medBackground;
            break;
        }
        Background[lo] = Background[hi];
        lo = lo + 1;
        while(BackGroundPri(lo) < medBackgroundPri)
        {
            lo = lo + 1;
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
}

void FindBlocks()
{
    double A = 0;
    double B = 0;
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
    BlocksSorted = true;
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
    } while(sortAgain == true);
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
    } while(sortAgain == true);
}

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
    else if(Foreground[Background[A].Type] == true)
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

void NPCSort()
{
    int A = 0;
    int B = 0;
    NPC_t tempNPC;

    for(A = 1; A <= numNPCs; A++)
    {
        if(NPCIsACoin[NPC[A].Type])
        {
            for(B = 1; B < A; B++)
            {
                if(!NPCIsACoin[NPC[B].Type])
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

void FindSBlocks()
{
    int A = 0;
    sBlockNum = 0;
    for(A = 1; A <= numBlock; A++)
    {
        if(BlockIsSizable[Block[A].Type])
        {
            sBlockNum = sBlockNum + 1;
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
            hi = hi - 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            sBlockArray[lo] = medBlock;
            break;
        }
        sBlockArray[lo] = sBlockArray[hi];
        lo = lo + 1;
        while(Block[sBlockArray[lo]].Location.Y < Block[medBlock].Location.Y)
        {
            lo = lo + 1;
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
            hi = hi - 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            NPC[lo] = medNPC;
            break;
        }
        NPC[lo] = NPC[hi];
        lo = lo + 1;
        while(NPC[lo].Location.Y >= medNPC.Location.Y)
        {
            lo = lo + 1;
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
        if(BackGroundPri(A) >= 25)
        {
            for(B = A; B <= numBackground; B++)
            {
                if(BackGroundPri(B) >= 100)
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
            hi = hi - 1;
            if(hi <= lo)
                break;
        }
        if(hi <= lo)
        {
            Block[lo] = medBlock;
            break;
        }
        Block[lo] = Block[hi];
        lo = lo + 1;
        while(Block[lo].Location.X < medBlock.Location.X)
        {
            lo = lo + 1;
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
