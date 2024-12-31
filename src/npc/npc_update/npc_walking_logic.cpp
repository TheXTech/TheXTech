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

#include "globals.h"
#include "player.h"
#include "npc.h"
#include "npc_traits.h"
#include "config.h"
#include "collision.h"

#include "main/trees.h"

void NPCWalkingLogic(int A, const double tempHit, const int tempHitBlock, float tempSpeedA)
{
    // tempSpeedA does not check for walking collisions in vanilla
    if(g_config.fix_npc_downward_clip)
    {
        tempSpeedA = Block[tempHitBlock].Location.SpeedY;
        if(tempSpeedA < 0)
            tempSpeedA = 0;
    }

    if(NPC[A].Type == NPCID_RED_FLY_FODDER) // Walking code for Flying Goomba
    {
        if(NPC[A].Special <= 30)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = 0;

            if(NPC[A].Slope > 0)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                if(NPC[A].Location.SpeedY < 0)
                    NPC[A].Location.SpeedY = 0;
            }

            if(tempSpeedA != 0)
                NPC[A].Location.SpeedY = tempSpeedA;
        }
        else if(NPC[A].Special == 31)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -4;
        }
        else if(NPC[A].Special == 32)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -4;
        }
        else if(NPC[A].Special == 33)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -4;
        }
        else if(NPC[A].Special == 34)
        {
            NPC[A].Special = 0;
            NPC[A].Location.SpeedY = -7;
        }
    }
    else if(NPC[A].Type == NPCID_FLY_CARRY_FODDER) // Walking code for SMW Flying Goomba
    {
        if(NPC[A].Special <= 60)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = 0;

            if(NPC[A].Slope > 0)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                if(NPC[A].Location.SpeedY < 0)
                    NPC[A].Location.SpeedY = 0;
            }

            if(tempSpeedA != 0)
                NPC[A].Location.SpeedY = tempSpeedA;
        }
        else if(NPC[A].Special == 61)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -3;
        }
        else if(NPC[A].Special == 62)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -3;
        }
        else if(NPC[A].Special == 63)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -3;
        }
        else if(NPC[A].Special == 64)
        {
            NPC[A].Special += 1;
            NPC[A].Location.SpeedY = -3;
        }
        else if(NPC[A].Special == 65)
        {
            NPC[A].Special = 0;
            NPC[A].Location.SpeedY = -7;
        }
    }
    else if(NPC[A]->TurnsAtCliffs && !NPC[A].Projectile) // Walking code NPCs that turn
    {
        bool tempTurn = true; // used for turning the npc around
        Location_t tempLocation = NPC[A].Location;
        tempLocation.SpeedX = 0;
        tempLocation.SpeedY = 0;
        tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 8;
        tempLocation.Height = 16;
        if(NPC[A].Slope > 0)
            tempLocation.Height = 32;
        tempLocation.Width = 16;

        bool isPokeyHead = (NPC[A].Type == NPCID_STACKER && NPC[A].Special2 == 0);

        // If .Location.SpeedX > 0 Then
        if(NPC[A].Direction > 0)
        {
            tempLocation.X += NPC[A].Location.Width - 20;
            if(isPokeyHead)
                tempLocation.X += 16;
            // If .Type = 189 Then tempLocation.X += -10
        }
        else
        {
            tempLocation.X += -tempLocation.Width + 20;
            if(isPokeyHead)
                tempLocation.X -= 16;
            // If .Type = 189 Then tempLocation.X += 10
        }

        for(int bCheck2 = 1; bCheck2 <= 2; bCheck2++)
        {
            // if(bCheck2 == 1)
            // {
            //     // fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
            //     // lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            //     blockTileGet(tempLocation, fBlock2, lBlock2);
            // }
            // else
            // {
            //     fBlock2 = numBlock - numTempBlock;
            //     lBlock2 = numBlock;
            // }
            auto collBlockSentinel2 = (bCheck2 == 1)
                ? treeFLBlockQuery(tempLocation, SORTMODE_NONE)
                : treeTempBlockQuery(tempLocation, SORTMODE_NONE);

            for(BlockRef_t block : collBlockSentinel2)
            {
                int B = block;
                //If BlockNoClipping(Block(B).Type) = False And Block(B).Invis = False And Block(B).Hidden = False And Not (BlockIsSizable(Block(B).Type) And Block(B).Location.Y < .Location.Y + .Location.Height - 3) Then

                // Don't collapse Pokey during walking on slopes and other touching surfaces
                if(g_config.fix_npc247_collapse && isPokeyHead && Block[B].tempBlockNpcType != NPCID_STACKER)
                    continue;

                if((tempLocation.X + tempLocation.Width >= Block[B].Location.X) &&
                   (tempLocation.X <= Block[B].Location.X + Block[B].Location.Width) &&
                   (tempLocation.Y + tempLocation.Height >= Block[B].Location.Y) &&
                   (tempLocation.Y <= Block[B].Location.Y + Block[B].Location.Height) &&
                   (!BlockNoClipping[Block[B].Type] && !Block[B].Invis && !Block[B].Hidden && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 3)))
                {
                    // If CheckCollision(tempLocation, Block(B).Location) = True Then
                    tempTurn = false;
                    break;
                    // End If
                }
                else
                {
                    // not working
                }

                // End If
            }

            if(!tempTurn)
                break;
        }

        if(tempTurn)
            NPC[A].TurnAround = true;

        NPC[A].Location.SpeedY = 0;
        if(NPC[A].Slope > 0)
        {
            NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = 0;
        }

        if(tempSpeedA != 0)
            NPC[A].Location.SpeedY = tempSpeedA;
    }
    else if(NPC[A].Type == NPCID_JUMPER_S4) // ninja code
    {
        bool tempTurn = true; // used for turning the npc around

        Location_t tempLocation = NPC[A].Location;
        tempLocation.SpeedX = 0;
        tempLocation.SpeedY = 0;
        tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 8;
        tempLocation.Height = 16;

        if(NPC[A].Slope > 0)
            tempLocation.Height = 32;

        tempLocation.Width = 16;

        if(NPC[A].Location.SpeedX > 0)
            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width - 16;
        else
            tempLocation.X = NPC[A].Location.X - tempLocation.Width + 16;

        for(int bCheck2 = 1; bCheck2 <= 2; bCheck2++)
        {
            // if(bCheck2 == 1)
            // {
            //     // fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
            //     // lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            //     blockTileGet(tempLocation, fBlock2, lBlock2);
            // }
            // else
            // {
            //     fBlock2 = numBlock - numTempBlock;
            //     lBlock2 = numBlock;
            // }
            auto collBlockSentinel2 = (bCheck2 == 1)
                ? treeFLBlockQuery(tempLocation, SORTMODE_NONE)
                : treeTempBlockQuery(tempLocation, SORTMODE_NONE);

            for(BlockRef_t block : collBlockSentinel2)
            {
                int B = block;
                if(!BlockNoClipping[Block[B].Type] && !Block[B].Invis && !Block[B].Hidden && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 3))
                {
                    if(CheckCollision(tempLocation, Block[B].Location))
                    {
                        tempTurn = false;
                        break;
                    }
                }

                if(!tempTurn)
                    break;
            }
        }

        tempLocation = NPC[A].Location;
        tempLocation.SpeedX = 0;
        tempLocation.SpeedY = 0;
        tempLocation.Y = NPC[A].Location.Y + 8;
        tempLocation.Height -= 16;
        tempLocation.Width = 32;
        if(NPC[A].Location.SpeedX > 0)
            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width;
        else
            tempLocation.X = NPC[A].Location.X - tempLocation.Width;

        // we are able to wrap this whole thing in the inner-loop check that (NPC[A].Slope <= 0)
        // commenting for now to avoid inadvertently introducing any bugs
        // if(NPC[A].Slope <= 0)
        for(int bCheck2 = 1; bCheck2 <= 2; bCheck2++)
        {
            // if(bCheck2 == 1)
            // {
            //     // fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
            //     // lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
            //     blockTileGet(tempLocation, fBlock2, lBlock2);
            // }
            // else
            // {
            //     fBlock2 = numBlock - numTempBlock;
            //     lBlock2 = numBlock;
            // }
            auto collBlockSentinel2 = (bCheck2 == 1)
                ? treeFLBlockQuery(tempLocation, SORTMODE_NONE)
                : treeTempBlockQuery(tempLocation, SORTMODE_NONE);

            for(BlockRef_t block : collBlockSentinel2)
            {
                int B = block;
                if(!BlockNoClipping[Block[B].Type] && !Block[B].Invis && !Block[B].Hidden && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 1))
                {
                    if(CheckCollision(tempLocation, Block[B].Location))
                    {
                        if(NPC[A].Slope > 0)
                        {

                        }
                        else if(BlockSlope[Block[B].Type] == 0)
                            tempTurn = true;
                        break;
                    }
                }

                if(tempTurn)
                    break;
            }
        }

        if(tempTurn)
        {
            NPC[A].Location.Y -= 0.1;
            NPC[A].Location.SpeedY = -6.55;
        }
        else
        {
            NPC[A].Location.SpeedY = 0;
            if(NPC[A].Slope > 0)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                if(NPC[A].Location.SpeedY < 0)
                    NPC[A].Location.SpeedY = 0;
            }
        }

        if(tempSpeedA != 0)
            NPC[A].Location.SpeedY += tempSpeedA;
    }
    else // Walking code for everything else
    {
        if(NPCIsAParaTroopa(NPC[A]))
        {
            if(NPC[A].Special == 1)
                NPC[A].Location.SpeedY = -9;
            else
            {
                if(NPC[A].Location.SpeedY > 0)
                    NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY;
            }
        }
        else
        {
            NPC[A].Location.SpeedY = 0;
            if(NPC[A].Slope > 0 && !NPC[A]->IsAShell && NPC[A].Type != NPCID_SLIDE_BLOCK)
            {
                NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                if(NPC[A].Location.SpeedY < 0)
                    NPC[A].Location.SpeedY = 0;
            }
        }

        if(NPC[A].Type == NPCID_BIRD)
        {
            NPC[A].Special += 1;
            if(NPC[A].Special <= 3)
                NPC[A].Location.SpeedY = -3.5;
            else
            {
                NPC[A].Location.SpeedY = -5.5;
                NPC[A].Special = 0;
            }
        }

        if(NPC[A].Type == NPCID_KNIGHT)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount > 1)
                NPC[A].FrameCount = 0;
            NPC[A].Location.SpeedY = -3;
        }

        if(tempSpeedA != 0)
            NPC[A].Location.SpeedY = tempSpeedA;

        // assigned to Special in SMBX 1.3
        if(NPC[A].Type == NPCID_SAW)
            NPC[A].SpecialY = NPC[A].Location.SpeedY;
    }

    if(NPC[A].Slope == 0)
        NPC[A].Location.Y = tempHit;

#if 0
    tempHit = 0;
    tempHitBlock = 0;

    // impossible
    if(Block[tempHitBlock].tempBlockNpcType > 0 && NPC[Block[tempHitBlock].tempBlockNpcIdx].Slope > 0)
    {
        // .Location.SpeedY = 0
        NPC[A].Slope = NPC[Block[tempHitBlock].tempBlockNpcIdx].Slope;
        // Stop
    }
#endif
}
