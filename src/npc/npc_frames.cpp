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
#include "npc.h"
#include "sound.h"
#include "collision.h"
#include "effect.h"
#include "npc_id.h"
#include "eff_id.h"
#include "npc_traits.h"

#include "main/trees.h"


void NPCFrames(int A)
{
    double B = 0;
    double C = 0;
    double D = 0;
    Location_t tempLocation;

    if(NPC[A]->TFrames > 0) // custom frames
    {
        NPC[A].FrameCount += 1;
        if(NPC[A]->FrameStyle == 2 && (NPC[A].Projectile || NPC[A].HoldingPlayer > 0))
            NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= NPC[A]->FrameSpeed)
        {
            if(NPC[A]->FrameStyle == 0)
                NPC[A].Frame += 1 * NPC[A].Direction;
            else
                NPC[A].Frame += 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A]->FrameStyle == 0)
        {
            if(NPC[A].Frame >= NPC[A]->TFrames)
                NPC[A].Frame = 0;
            if(NPC[A].Frame < 0)
                NPC[A].Frame = NPC[A]->TFrames - 1;
        }
        else if(NPC[A]->FrameStyle == 1)
        {
            if(NPC[A].Direction == -1)
            {
                if(NPC[A].Frame >= NPC[A]->TFrames)
                    NPC[A].Frame = 0;
                if(NPC[A].Frame < 0)
                    NPC[A].Frame = NPC[A]->TFrames;
            }
            else
            {
                if(NPC[A].Frame >= NPC[A]->TFrames * 2)
                    NPC[A].Frame = NPC[A]->TFrames;
                if(NPC[A].Frame < NPC[A]->TFrames)
                    NPC[A].Frame = NPC[A]->TFrames;
            }
        }
        else if(NPC[A]->FrameStyle == 2)
        {
            if(NPC[A].HoldingPlayer == 0 && !NPC[A].Projectile)
            {
                if(NPC[A].Direction == -1)
                {
                    if(NPC[A].Frame >= NPC[A]->TFrames)
                        NPC[A].Frame = 0;
                    if(NPC[A].Frame < 0)
                        NPC[A].Frame = NPC[A]->TFrames - 1;
                }
                else
                {
                    if(NPC[A].Frame >= NPC[A]->TFrames * 2)
                        NPC[A].Frame = NPC[A]->TFrames;
                    if(NPC[A].Frame < NPC[A]->TFrames)
                        NPC[A].Frame = NPC[A]->TFrames * 2 - 1;
                }
            }
            else
            {
                if(NPC[A].Direction == -1)
                {
                    if(NPC[A].Frame >= NPC[A]->TFrames * 3)
                        NPC[A].Frame = NPC[A]->TFrames * 2;
                    if(NPC[A].Frame < NPC[A]->TFrames * 2)
                        NPC[A].Frame = NPC[A]->TFrames * 3 - 1;
                }
                else
                {
                    if(NPC[A].Frame >= NPC[A]->TFrames * 4)
                        NPC[A].Frame = NPC[A]->TFrames * 3;
                    if(NPC[A].Frame < NPC[A]->TFrames * 3)
                        NPC[A].Frame = NPC[A]->TFrames * 4 - 1;
                }
            }
        }
    }
    else if(NPC[A].Type == NPCID_SQUID_S3 || NPC[A].Type == NPCID_SQUID_S1 || NPC[A].Type == NPCID_VILLAIN_S3 || NPC[A].Type == NPCID_SPIT_BOSS_BALL ||
            NPC[A].Type == NPCID_FALL_BLOCK_RED || NPC[A].Type == NPCID_FALL_BLOCK_BROWN || NPC[A].Type == NPCID_SPIKY_THROWER || NPC[A].Type == NPCID_ITEM_THROWER ||
            NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT ||
            NPC[A].Type == NPCID_VPIPE_LONG || NPC[A].Type == NPCID_BIG_SHELL || NPCIsVeggie(NPC[A]) || NPC[A].Type == NPCID_SHORT_WOOD ||
            NPC[A].Type == NPCID_LONG_WOOD || NPC[A].Type == NPCID_SLANT_WOOD_R || NPC[A].Type == NPCID_SLANT_WOOD_M || NPC[A].Type == NPCID_PLATFORM_S3 ||
            NPC[A].Type == NPCID_CHECKER_PLATFORM || NPC[A].Type == NPCID_PLATFORM_S1 || NPC[A].Type == NPCID_SPIT_GUY_BALL || NPC[A].Type == NPCID_SIGN ||
            (NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D) || NPC[A].Type == NPCID_LIFT_SAND || NPC[A].Type == NPCID_CHECKPOINT ||
            NPC[A].Type == NPCID_GOALTAPE || NPC[A]->IsAVine || NPC[A].Type == NPCID_ICE_BLOCK || NPC[A].Type == NPCID_TNT ||
            NPC[A].Type == NPCID_TIMER_S2 || NPC[A].Type == NPCID_POWER_S5 || NPC[A].Type == NPCID_MAGIC_DOOR || NPC[A].Type == NPCID_COCKPIT) // no frames
    {
        if(!(NPC[A].Type == NPCID_VILLAIN_S3 || NPC[A].Type == NPCID_ITEM_THROWER || NPC[A].Type == NPCID_SPIKY_THROWER) && A == 0) // Reset Frame to 0 unless a specific NPC type
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_STATUE_POWER || NPC[A].Type == NPCID_HEAVY_POWER)
    {
        C = 0;
        for(B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section && Player[B].TimeToLive == 0)
            {
                if(C == 0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) + std::abs(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - (Player[B].Location.Y + Player[B].Location.Height / 2.0)) < C)
                {
                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) + std::abs(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - (Player[B].Location.Y + Player[B].Location.Height / 2.0));
                    if(Player[B].Character == 5)
                        D = 1;
                    else
                        D = 0;
                }
            }
        }
        if(D != NPC[A].Frame)
        {
            if(NPC[A].FrameCount > 0)
                NewEffect(EFFID_SMOKE_S4, NPC[A].Location);
            NPC[A].Frame = D;
        }
        NPC[A].FrameCount = 1;
    }
    else if(NPC[A].Type == NPCID_FLY_BLOCK || NPC[A].Type == NPCID_FLY_CANNON) // fly block
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].HoldingPlayer > 0)
            NPC[A].FrameCount += 1;
        if(NPC[A].Location.SpeedY != 0)
            NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount <= 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 30)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 36)
            NPC[A].Frame = 1;
        else
        {
            NPC[A].Frame = 0;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Type == NPCID_FLY_CANNON && NPC[A].Direction == 1)
            NPC[A].Frame += 4;
    }
    else if(NPC[A].Type == NPCID_QUAD_SPITTER) // fire plant thing
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 2;
            else
            {
                NPC[A].Frame = 0;
                NPC[A].FrameCount = 0;
            }
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 1;
            else
            {
                NPC[A].Frame = 0;
                NPC[A].FrameCount = 0;
            }
        }
        else
            NPC[A].Frame = 3;
    }
    else if(NPC[A].Type == NPCID_DOOR_MAKER) // potion
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].Frame += 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Frame >= 4)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_ITEM_BUBBLE) // bubble
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount < 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 18)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 24)
            NPC[A].Frame = 2;
        else
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_VINE_BUG) // spider
    {
        if(NPC[A].Projectile || NPC[A].Location.SpeedY >= 0 || NPC[A].HoldingPlayer > 0)
            NPC[A].Frame = 0;
        else
            NPC[A].Frame = 2;
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount > 15)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount >= 8)
            NPC[A].Frame += 1;
    }
    else if(NPC[A].Type == NPCID_BAT) // bat thing
    {
        if(NPC[A].Special == 0)
            NPC[A].Frame = 0;
        else
        {
            NPC[A].Frame = 1;
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount > 15)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 8)
                NPC[A].Frame = 2;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 3;


    }
    else if(NPC[A].Type == NPCID_JUMP_PLANT) // jumping plant
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].Frame += 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Frame >= 4)
            NPC[A].Frame = 0;

    }
    else if(NPC[A].Type == NPCID_FIRE_BOSS) // ludwig koopa
    {
        if(NPC[A].Location.SpeedY != 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 10;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 11;
            else
            {
                NPC[A].Frame = 10;
                NPC[A].FrameCount = 0;
            }
        }
        else
        {
            if(NPC[A].Special == 0)
            {
                NPC[A].FrameCount += 1;
                if(NPC[A].Location.SpeedX == 0)
                    NPC[A].FrameCount = 10;
                if(NPC[A].FrameCount < 4)
                    NPC[A].Frame = 0;
                else if(NPC[A].FrameCount < 8)
                    NPC[A].Frame = 1;
                else if(NPC[A].FrameCount < 12)
                    NPC[A].Frame = 2;
                else
                {
                    NPC[A].Frame = 0;
                    NPC[A].FrameCount = 0;
                }
            }
            else if(NPC[A].Special == 1)
                NPC[A].Frame = 3;
            else if(NPC[A].Special == 2)
                NPC[A].Frame = 4;
            if(NPC[A].Direction == 1)
                NPC[A].Frame += 5;
        }



    }
    else if(NPC[A].Type == NPCID_FIRE_BOSS_SHELL) // ludwig shell
    {
        if(NPC[A].Location.SpeedX == 0)
        {
            if(NPC[A].Frame > 2)
                NPC[A].Frame = 0;
        }
        else
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].Frame += NPC[A].Direction;
                NPC[A].FrameCount = 0;
            }
            if(NPC[A].Frame < 0)
                NPC[A].Frame = 2;
            if(NPC[A].Frame > 2)
                NPC[A].Frame = 0;
        }

    }
    else if(NPC[A].Type == NPCID_FIRE_BOSS_FIRE) // ludwig fire
    {
        NPC[A].FrameCount += 1;
        NPC[A].Frame = 0;
        if(NPC[A].FrameCount > 8)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount >= 4)
            NPC[A].Frame = 1;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 2;

    }
    else if(NPC[A].Type == NPCID_MAGIC_BOSS_BALL) // larry magic
    {
        if(NPC[A].Special == 0)
            NPC[A].Frame = 2;
        else if(NPC[A].Special == 1)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_MAGIC_BOSS_SHELL) // larry shell
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].Frame += NPC[A].Direction;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Frame < 0)
            NPC[A].Frame = 5;
        if(NPC[A].Frame > 5)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_MAGIC_BOSS) // larry koopa
    {

        NPC[A].Frame = 0;
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedY == 0)
            {
                if(NPC[A].Location.SpeedX == 0)
                    NPC[A].Frame = 0;
                else
                {
                    NPC[A].FrameCount += 1;
                    if(NPC[A].FrameCount < 8)
                        NPC[A].Frame = 0;
                    else if(NPC[A].FrameCount < 16)
                        NPC[A].Frame = 1;
                    else
                    {
                        NPC[A].Frame = 0;
                        NPC[A].FrameCount = 0;
                    }
                }
            }
            else
                NPC[A].Frame = 1;
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 2)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 3;
            else if(NPC[A].FrameCount < 6)
                NPC[A].Frame = 4;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 5;
            else
            {
                NPC[A].Frame = 2;
                NPC[A].FrameCount = 0;
            }
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 2)
                NPC[A].Frame = 6;
            else if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 7;
            else if(NPC[A].FrameCount < 6)
                NPC[A].Frame = 8;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 9;
            else
            {
                NPC[A].Frame = 6;
                NPC[A].FrameCount = 0;
            }
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 10;


    }
    else if(NPC[A].Type == NPCID_SWORDBEAM) // sword beam
    {
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 4;
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount < 2)
        {
        }
        else if(NPC[A].FrameCount < 4)
            NPC[A].Frame += 1;
        else if(NPC[A].FrameCount < 6)
            NPC[A].Frame += 2;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame += 3;
        else
            NPC[A].FrameCount = 0;


    }
    else if(NPC[A].Type == NPCID_BOMBER_BOSS) // mouser
    {
        if(NPC[A].Immune > 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 3;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 4;
            else if(NPC[A].FrameCount < 12)
                NPC[A].Frame = 5;
            else if(NPC[A].FrameCount < 15)
                NPC[A].Frame = 6;
            else
            {
                NPC[A].Frame = 6;
                NPC[A].FrameCount = 0;
            }
            if(NPC[A].Direction == 1)
                NPC[A].Frame += 7;
        }
        else if(NPC[A].Special <= 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount < 15)
                NPC[A].Frame = 2;
            else
            {
                NPC[A].Frame = 2;
                NPC[A].FrameCount = 0;
            }
            if(NPC[A].Direction == 1)
                NPC[A].Frame += 7;
        }
        else
        {
            NPC[A].Frame = 0;
            if(NPC[A].Direction == 1)
                NPC[A].Frame += 7;
        }

    }
    else if(NPC[A].Type == NPCID_WALK_PLANT)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 15)
            NPC[A].Frame = 1;
        else
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 1;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 4;
        if(NPC[A].Special > 0 && NPC[A].Location.SpeedY <= 0)
            NPC[A].Frame += 2;

    }
    else if(NPC[A].Type == NPCID_FIRE_CHAIN)
    {
        if(NPC[A].Direction == 1)
            NPC[A].Frame = SpecialFrame[2];
        else
            NPC[A].Frame = 3 - SpecialFrame[2];
    }
    else if(NPC[A].Type == NPCID_LOCK_DOOR)
    {
        // NPC has no frames so do nothing
    }
    else if(NPC[A].Type == NPCID_FIRE_DISK)
    {
        NPC[A].Frame += 1;
        if(NPC[A].Frame >= 5)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_GEM_1 || NPC[A].Type == NPCID_GEM_5 || NPC[A].Type == NPCID_GEM_20)
        NPC[A].Frame = SpecialFrame[8];
    else if(NPC[A].Type == NPCID_TIME_SWITCH)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame += 1;
        }
        if(NPC[A].Frame >= 3)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_STACKER)
    {
        // Special less than zero - body, zero - head
        if(NPC[A].Special < 0 && NPC[A].Location.SpeedY == 0)
            NPC[A].Special += 1;
        if(NPC[A].Projectile || NPC[A].HoldingPlayer > 0)
            NPC[A].Frame = 4;
        else
        {
            if(NPC[A].Special < 0)
                NPC[A].Frame = 1;
            else
                NPC[A].Frame = 0;
        }
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount > 8)
        {
            if(NPC[A].Projectile || NPC[A].HoldingPlayer > 0)
                NPC[A].Frame += 1;
            else
                NPC[A].Frame += 2;
        }
    }
    else if(NPC[A].Type == NPCID_FIRE_PLANT)
    {
        NPC[A].Frame = 0;
        if(Player[NPC[A].Special4].Location.X + Player[NPC[A].Special4].Location.Width / 2.0 > NPC[A].Location.X + NPC[A].Location.Width / 2.0)
            NPC[A].Frame = 2;
        if(Player[NPC[A].Special4].Location.Y + Player[NPC[A].Special4].Location.Height / 2.0 < NPC[A].Location.Y + 16)
            NPC[A].Frame += 1;
    }
    else if(NPC[A].Type == NPCID_FLY_FODDER_S5)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame += 1;
            if(NPC[A].Frame >= 2)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_EARTHQUAKE_BLOCK) // POW block
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame += 1;
            if(NPC[A].Frame >= 7)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_SLANT_WOOD_L) // 1 frame left or right
    {
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_HOMING_BALL_GEN)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount <= 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 30)
            NPC[A].Frame = 4;
        else if(NPC[A].FrameCount <= 36)
            NPC[A].Frame = 5;
        else if(NPC[A].FrameCount <= 42)
            NPC[A].Frame = 4;
        else if(NPC[A].FrameCount <= 48)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 54)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 60)
            NPC[A].Frame = 1;
        else
            NPC[A].FrameCount = 0;
    }
    else if(NPC[A].Type == NPCID_HOMING_BALL)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount <= 8)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 16)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 32)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 38)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 46)
            NPC[A].Frame = 1;
        else
            NPC[A].FrameCount = 0;
    }
    else if(NPC[A].Type == NPCID_BOSS_FRAGILE)
    {
        NPC[A].Frame = 0;
        if(NPC[A].Special > 0 && NPC[A].Special < 15)
            NPC[A].Frame = 1;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 2;
    }
    else if(NPC[A].Type == NPCID_BOSS_CASE)
    {
        if(NPC[A].Damage < 3)
            NPC[A].Frame = 0;
        else if(NPC[A].Damage < 6)
            NPC[A].Frame = 1;
        else if(NPC[A].Damage < 9)
            NPC[A].Frame = 2;
        else if(NPC[A].Damage < 12)
            NPC[A].Frame = 3;
        else
            NPC[A].Frame = 4;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 5;
    }
    else if(NPC[A].Type == NPCID_WALL_TURTLE)
    {
        NPC[A].FrameCount += 1;
        NPC[A].Frame = 0;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount > 8)
            NPC[A].Frame = 1;
        if(NPC[A].Special == 4)
            NPC[A].Frame += 4;
        else if(NPC[A].Special == 3)
            NPC[A].Frame += 8;
        else if(NPC[A].Special == 2)
            NPC[A].Frame += 12;
        if(NPC[A].Special2 == 1)
            NPC[A].Frame += 2;


    }
    else if(NPC[A].Type == NPCID_WALL_BUG)
    {
        NPC[A].FrameCount += 1;
        NPC[A].Frame = 0;
        if(NPC[A].FrameCount <= 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount <= 30)
            NPC[A].Frame = 4;
        else
            NPC[A].FrameCount = 0;
        if(NPC[A].Special == 4)
            NPC[A].Frame += 5;
        else if(NPC[A].Special == 3)
            NPC[A].Frame += 10;
        else if(NPC[A].Special == 2)
            NPC[A].Frame += 15;


    }
    else if(NPC[A].Type == NPCID_FLIER || NPC[A].Type == NPCID_ROCKET_FLIER)
    {
        NPC[A].FrameCount += 1;
        NPC[A].Frame = 0;
        if(NPC[A].FrameCount <= 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount <= 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount <= 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount <= 24)
            NPC[A].Frame = 1;
        else
            NPC[A].FrameCount = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 4;
    }
    else if(NPC[A].Type == NPCID_SICK_BOSS)
    {
        NPC[A].Frame = 0;
        if(NPC[A].Special == 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 1;
            else
                NPC[A].FrameCount = 0;
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 2;

        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 3;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 4;
            else
            {
                NPC[A].Frame = 3;
                NPC[A].FrameCount = 0;
            }
        }

        if(NPC[A].Special == 3 || NPC[A].Special == 2)
        {
            NPC[A].Frame = 0;
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 5;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 6;
            else if(NPC[A].FrameCount < 12)
                NPC[A].Frame = 7;
            else
            {
                NPC[A].Frame = 7;
                NPC[A].FrameCount = 0;
            }
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 8;
    }
    else if(NPC[A].Type == NPCID_VILLAIN_S1) // King Koopa
    {
        NPC[A].Frame = 0;
        if(NPC[A].Special == 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount <= 8)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount <= 16)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount <= 24)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount <= 32)
                NPC[A].Frame = 0;
            else
                NPC[A].FrameCount = 0;
        }
        else if(NPC[A].Special == 1)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 3;
        }
        else if(NPC[A].Special == 2)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 4;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 5;
    }
    else if(NPC[A].Type == NPCID_STAR_COLLECT)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame += 1;
            if(NPC[A].Frame >= 2)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_STONE_S4)
    {
        NPC[A].Frame = 0;
        C = 0;
        for(B = 1; B <= numPlayers; ++B)
        {
            if(!CanComeOut(NPC[A].Location, Player[B].Location) && Player[B].Location.Y >= NPC[A].Location.Y)
                C = B;
        }
        if(C > 0)
            NPC[A].Frame = 2;
        else
        {
            for(B = 1; B <= numPlayers; ++B)
            {
                tempLocation = NPC[A].Location;
                tempLocation.Width = NPC[A].Location.Width * 2;
                tempLocation.X = NPC[A].Location.X - NPC[A].Location.Width / 2.0;
                if(!CanComeOut(tempLocation, Player[B].Location) && Player[B].Location.Y >= NPC[A].Location.Y)
                    C = B;
            }
            if(C > 0)
                NPC[A].Frame = 1;
        }
        if(NPC[A].Special == 1)
            NPC[A].Frame = 2;

    }
    else if(NPC[A].Type == NPCID_CHAR4_HEAVY) // toad boomerang
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 6)
        {
            NPC[A].FrameCount = 0;

            if(NPC[A].Location.SpeedX > 0)
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame == 1)
                    NPC[A].Location.X += 4;
                else if(NPC[A].Frame == 3)
                    NPC[A].Location.X -= 4;
                else if(NPC[A].Frame == 2)
                    NPC[A].Location.Y += 4;
                else
                    NPC[A].Location.Y -= 4;
            }
            else
            {
                NPC[A].Frame -= 1;
                if(NPC[A].Frame == 0)
                    NPC[A].Location.X -= 4;
                else if(NPC[A].Frame == 1)
                    NPC[A].Location.Y -= 4;
                else if(NPC[A].Frame == 2)
                    NPC[A].Location.X += 4;
                else
                    NPC[A].Location.Y += 4;
            }

            if(NPC[A].Frame > 3)
                NPC[A].Frame = 0;
            else if(NPC[A].Frame < 0)
                NPC[A].Frame = 3;

            treeNPCUpdate(A);
            if(NPC[A].tempBlock > 0)
                treeNPCSplitTempBlock(A);
        }

        if(iRand(4) == 0)
        {
            NewEffect(EFFID_SPARKLE, newLoc(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 4, NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 4), 1, 0, NPC[A].Shadow);
            Effect[numEffects].Location.SpeedX = dRand() * 1 - 0.5;
            Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5;
        }



    }
    else if(NPC[A].Type == NPCID_PLR_HEAVY) // Mario Hammer
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            if(NPC[A].Location.SpeedX > 0)
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame == 1)
                    NPC[A].Location.X += 8;
                else if(NPC[A].Frame == 3)
                    NPC[A].Location.X -= 8;
                else if(NPC[A].Frame == 2)
                    NPC[A].Location.Y += 12;
                else
                    NPC[A].Location.Y -= 12;
            }
            else
            {
                NPC[A].Frame -= 1;
                if(NPC[A].Frame == 0)
                    NPC[A].Location.X -= 8;
                else if(NPC[A].Frame == 1)
                    NPC[A].Location.Y -= 12;
                else if(NPC[A].Frame == 2)
                    NPC[A].Location.X += 8;
                else
                    NPC[A].Location.Y += 12;
            }
            if(NPC[A].Frame > 3)
                NPC[A].Frame = 0;
            else if(NPC[A].Frame < 0)
                NPC[A].Frame = 3;
            NewEffect(EFFID_SPARKLE, newLoc(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 8, NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 8), 1, 0, NPC[A].Shadow);
            Effect[numEffects].Location.SpeedX = dRand() * 1 - 0.5;
            Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5;
        }

        treeNPCUpdate(A);
        if(NPC[A].tempBlock > 0)
            treeNPCSplitTempBlock(A);
    }
    else if(NPC[A].Type == NPCID_FLY_CARRY_FODDER) // smw paragoomba
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 4;
        else
            NPC[A].Frame = 0;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
        else if(NPC[A].FrameCount >= 8)
            NPC[A].Frame += 1;

        if(NPC[A].Effect == 0)
        {
            if(NPC[A].Special == 0)
                NPC[A].Special2 += 2;
            else if(NPC[A].Special <= 60)
                NPC[A].Special2 = 0;
            else if(NPC[A].Special < 65)
                NPC[A].Special2 += 1;
            else
                NPC[A].Special2 += 2;
            if(NPC[A].Special2 >= 16)
                NPC[A].Special2 = 0;
            else if(NPC[A].Special2 >= 8)
                NPC[A].Frame += 2;
        }
    }
    else if(NPC[A].Type == NPCID_RED_FLY_FODDER || NPC[A].Type == NPCID_FLY_FODDER_S3) // Flying Goomba
    {
        if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 8)
            {
                NPC[A].FrameCount = 0;
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
            }
        }
        else
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Frame == 0)
                    NPC[A].Frame = 2;
                else if(NPC[A].Frame == 1)
                    NPC[A].Frame = 3;
                else if(NPC[A].Frame == 2)
                    NPC[A].Frame = 1;
                else if(NPC[A].Frame == 3)
                    NPC[A].Frame = 0;
            }
        }
    }
    else if(NPC[A].Type == NPCID_BOMB) // bomb
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount < 4)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 11)
            NPC[A].Frame = 2;
        else
            NPC[A].FrameCount = 0;
        if(NPC[A].Special2 == 1)
        {
            NPC[A].Special3 += 1;
            if(NPC[A].Special3 < 4)
            {
            }
            else if(NPC[A].Special3 < 8)
                NPC[A].Frame += 9;
            else if(NPC[A].Special3 < 12)
                NPC[A].Frame += 3;
            else if(NPC[A].Special3 < 15)
                NPC[A].Frame += 6;
            else
                NPC[A].Special3 = 0;
        }
    }
    else if(NPC[A].Type == NPCID_CHAR3_HEAVY) // heart bomb
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount < 4)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 11)
            NPC[A].Frame = 2;
        else
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 0;
        }
        NPC[A].Special3 += 1;
        if(NPC[A].Special3 < 4)
        {}
        else if(NPC[A].Special3 < 8)
            NPC[A].Frame += 3;
        else if(NPC[A].Special3 < 12)
            NPC[A].Frame += 6;
        else // If .Special3 >= 16 Then
            NPC[A].Special3 = 0;
        if(iRand(100) >= 92)
        {
            NewEffect(EFFID_SPARKLE, newLoc(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - 4, NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 6), 1, 0, NPC[A].Shadow);
            Effect[numEffects].Location.SpeedX = dRand() * 1 - 0.5;
            Effect[numEffects].Location.SpeedY = dRand() * 1 - 0.5;
        }
    }
    else if(NPC[A].Type == NPCID_ITEM_BURIED)
        NPC[A].Frame = SpecialFrame[5];
    else if(NPC[A].Type == NPCID_ITEM_POD)
    {
        NPC[A].Frame = 0;
        if(NPC[A].Special == 98)
            NPC[A].Frame = 1;
        else if(NPC[A].Special == 99)
            NPC[A].Frame = 2;
        else if(NPC[A].Special == 100)
            NPC[A].Frame = 3;
        else if(NPC[A].Special == 148)
            NPC[A].Frame = 4;
        else if(NPC[A].Special == 149)
            NPC[A].Frame = 5;
        else if(NPC[A].Special == 150)
            NPC[A].Frame = 6;
        else if(NPC[A].Special == 228)
            NPC[A].Frame = 7;
    }
    else if(NPC[A].Type == NPCID_RAINBOW_SHELL || NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL) // Glowy Shell
    {
        NPC[A].Special5 += 1;
        if(NPC[A].Special5 >= 16)
            NPC[A].Special5 = 0;
        if(NPC[A].Location.SpeedX > 0)
        {
            if(NPC[A].Type == NPCID_RAINBOW_SHELL)
                NPC[A].FrameCount += 1;
            else
                NPC[A].FrameCount -= 1;
        }
        else if(NPC[A].Location.SpeedX < 0)
        {
            if(NPC[A].Type == NPCID_RAINBOW_SHELL)
                NPC[A].FrameCount -= 1;
            else
                NPC[A].FrameCount += 1;
        }
        else
        {
            if(NPC[A].Type == NPCID_RAINBOW_SHELL)
                NPC[A].FrameCount = 0;
            else
                NPC[A].FrameCount = 12;
        }
        if(NPC[A].FrameCount < 0)
            NPC[A].FrameCount = 15;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
        if(NPC[A].FrameCount < 4)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 12)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount < 16)
            NPC[A].Frame = 3;
        if(NPC[A].Special5 < 4)
        {
        }
        else if(NPC[A].Special5 < 8)
            NPC[A].Frame += 4;
        else if(NPC[A].Special5 < 12)
            NPC[A].Frame += 8;
        else if(NPC[A].Special5 < 16)
            NPC[A].Frame += 12;
    }
    else if(NPC[A]->IsAShell) // Turtle shell
    {
        if(NPC[A].Location.SpeedX == 0)
            NPC[A].Frame = 0;
        else
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 0;
            }
        }
    }
    else if(NPC[A].Type == NPCID_JUMPER_S4) // black ninja
    {
        if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
        {
            NPC[A].Frame = 0;
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 12)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 6)
                NPC[A].Frame = 1;
        }
        else if(NPC[A].Location.SpeedY < 0)
        {
            NPC[A].Frame = 0;
            NPC[A].FrameCount = 6;
        }
        else
        {
            NPC[A].Frame = 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 2;
    }
    else if(NPC[A].Type == NPCID_CONVEYOR) // smb3 belt
    {
        if(NPC[A].Direction == -1)
            NPC[A].Frame = SpecialFrame[4];
        else
            NPC[A].Frame = 3 - SpecialFrame[4];
    }
    else if(NPC[A].Type == NPCID_YEL_PLATFORM || NPC[A].Type == NPCID_BLU_PLATFORM || NPC[A].Type == NPCID_GRN_PLATFORM || NPC[A].Type == NPCID_RED_PLATFORM)
    {
        NPC[A].Frame = 1;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_BULLY) // Bully
    {
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 3;

        if(NPC[A].Projectile || NPC[A].Special2 != 0)
        {
            NPC[A].Frame += 2;
            NPC[A].FrameCount = 0;
        }
        else
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 16)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 8)
                NPC[A].Frame += 1;
        }


    }
    else if(NPC[A].Type == NPCID_TANK_TREADS) // tank treads
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount >= 4)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
        if(NPC[A].FrameCount > 12)
            NPC[A].FrameCount = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 3;
    }
    else if(NPC[A].Type == NPCID_EXT_TURTLE) // nekkid koopa
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].Frame = 0;
            if(NPC[A].Direction == 1)
                NPC[A].Frame = 3;
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 15)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 8)
                NPC[A].Frame += 1;
        }
        else
        {
            if(NPC[A].Direction == -1)
                NPC[A].Frame = 2;
            else
                NPC[A].Frame = 5;
        }
    }
    else if(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4) // beach koopa
    {
        if(NPC[A].Projectile)
        {
            if(NPC[A].Location.SpeedX < -0.5 || NPC[A].Location.SpeedX > 0.5)
                NPC[A].Frame = 3;
            else
            {
                NPC[A].Frame = 3;
                NPC[A].FrameCount += 1;
                if(NPC[A].FrameCount >= 15)
                    NPC[A].FrameCount = 0;
                else if(NPC[A].FrameCount >= 8)
                    NPC[A].Frame = 4;
            }
        }
        else
        {
            if(NPC[A].Special == 0)
            {
                NPC[A].Frame = 0;
                NPC[A].FrameCount += 1;
                if(NPC[A].FrameCount >= 15)
                    NPC[A].FrameCount = 0;
                else if(NPC[A].FrameCount >= 8)
                    NPC[A].Frame = 1;
            }
            else
                NPC[A].Frame = 2;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 5;
    }
    else if(NPC[A].Type == NPCID_FLY) // bouncy bee
    {
        if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 0;
        }
        else
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 3)
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
                NPC[A].FrameCount = 0;
            }
        }
    }
    else if(NPC[A].Type == NPCID_VEHICLE)
    {
        NPC[A].Frame = SpecialFrame[2];
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 4;
    }
    else if(NPC[A].Type == NPCID_SLIDE_BLOCK) // ice block
    {
        if(NPC[A].Special == 0)
            NPC[A].Frame = BlockFrame[4];
        else
        {
            if(NPC[A].Frame < 4)
                NPC[A].Frame = 4;
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 6)
                    NPC[A].Frame = 4;
            }
        }
        // bowser fireball
    }
    else if(NPC[A].Type == NPCID_VILLAIN_FIRE)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 20)
            NPC[A].FrameCount = 0;
        NPC[A].Frame = static_cast<int>(floor(static_cast<double>(NPC[A].FrameCount / 5)));
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 4;
        // statue fireball
    }
    else if(NPC[A].Type == NPCID_STATUE_FIRE)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
            NPC[A].FrameCount = 0;
        NPC[A].Frame = static_cast<int>(floor(static_cast<double>(NPC[A].FrameCount / 2)));
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 4;
        // winged koopa
    }
    else if(NPC[A].Type == NPCID_GRN_FLY_TURTLE_S3 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S3)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].Direction == -1 && NPC[A].Frame >= 4)
            NPC[A].Frame = 0;
        else if(NPC[A].Direction == 1 && NPC[A].Frame < 4)
            NPC[A].Frame = 4;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            if(NPC[A].Direction == -1)
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 0;
            }
            else
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 8)
                    NPC[A].Frame = 4;
            }
        }
    }
    else if(NPC[A].Type == NPCID_LIT_BOMB_S3) // SMB3 Bomb
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 15)
            NPC[A].Frame = 1;
        else
        {
            NPC[A].Frame = 1;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 6;
        if(NPC[A].Special2 == 1)
        {
            NPC[A].Special3 += 1;
            if(NPC[A].Special3 < 4)
            {
            }
            else if(NPC[A].Special3 < 8)
                NPC[A].Frame += 2;
            else if(NPC[A].Special3 < 11)
                NPC[A].Frame += 4;
            else
            {
                NPC[A].Frame += 4;
                NPC[A].Special3 = 0;
            }
        }
    }
    else if(NPC[A].Type == NPCID_ROCKET_WOOD) // Airship Jet
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].Direction == -1 && NPC[A].Frame >= 4)
            NPC[A].Frame = 0;
        else if(NPC[A].Direction == 1 && NPC[A].Frame < 4)
            NPC[A].Frame = 8;
        if(NPC[A].FrameCount >= 2)
        {
            NPC[A].FrameCount = 0;
            if(NPC[A].Direction == -1)
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 0;
            }
            else
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 8)
                    NPC[A].Frame = 4;
            }
        }
    }
    else if(NPC[A].Type == NPCID_AXE)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame += 1;
            if(NPC[A].Frame >= 3)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_GRN_TURTLE_S3 || NPC[A].Type == NPCID_RED_TURTLE_S3 || NPC[A].Type == NPCID_GLASS_TURTLE || NPC[A].Type == NPCID_SPIKY_S3 || NPC[A].Type == NPCID_SPIKY_S4 || NPC[A].Type == NPCID_GHOST_FAST || NPC[A].Type == NPCID_SIDE_PLANT || NPC[A].Type == NPCID_BIG_TURTLE || (NPC[A].Type >= NPCID_GRN_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_TURTLE_S4) || (NPC[A].Type >= NPCID_GRN_FLY_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_FLY_TURTLE_S4) || NPC[A].Type == NPCID_WALK_BOMB_S3 || NPC[A].Type == NPCID_LIFT_SAND || NPC[A].Type == NPCID_BRUTE || NPC[A].Type == NPCID_BRUTE_SQUISHED || NPC[A].Type == NPCID_BIG_MOLE || NPC[A].Type == NPCID_CARRY_FODDER || NPC[A].Type == NPCID_HIT_CARRY_FODDER || NPC[A].Type == NPCID_GRN_TURTLE_S1 || NPC[A].Type == NPCID_RED_TURTLE_S1 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S1 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S1 || NPC[A].Type == NPCID_LAVA_MONSTER || NPC[A].Type == NPCID_GRN_FISH_S3 || NPC[A].Type == NPCID_FISH_S4 || NPC[A].Type == NPCID_RED_FISH_S3 || NPC[A].Type == NPCID_GOGGLE_FISH || NPC[A].Type == NPCID_GRN_FISH_S1) // Walking koopa troopa / hard thing / spiney
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].Type == NPCID_HIT_CARRY_FODDER && NPC[A].Special > 360)
            NPC[A].FrameCount += 1;
        if(NPC[A].Direction == -1 && NPC[A].Frame >= 2)
            NPC[A].Frame = 0;
        else if(NPC[A].Direction == 1 && NPC[A].Frame < 2)
            NPC[A].Frame = 2;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 0;
            if(NPC[A].Direction == -1)
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
            }
            else
            {
                NPC[A].Frame += 1;
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 2;
            }
        }

    }
    else if(NPC[A].Type == NPCID_BONE_FISH)
    {
        NPC[A].FrameCount += 1;
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 3;

        if(NPC[A].FrameCount > 8)
            NPC[A].Frame += 1;

        if(NPC[A].FrameCount > 16)
            NPC[A].Frame += 1;
        if(NPC[A].FrameCount > 24)
            NPC[A].Frame -= 1;
        if(NPC[A].FrameCount > 32)
            NPC[A].FrameCount = 0;

    }
    else if(NPC[A].Type == NPCID_SKELETON) // dry bones
    {
        if(NPC[A].Special == 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].Type == NPCID_HIT_CARRY_FODDER && NPC[A].Special > 360)
                NPC[A].FrameCount += 1;
            if(NPC[A].Direction == -1 && NPC[A].Frame >= 2)
                NPC[A].Frame = 0;
            else if(NPC[A].Direction == 1 && NPC[A].Frame < 2)
                NPC[A].Frame = 2;
            if(NPC[A].FrameCount >= 8)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Direction == -1)
                {
                    NPC[A].Frame += 1;
                    if(NPC[A].Frame >= 2)
                        NPC[A].Frame = 0;
                }
                else
                {
                    NPC[A].Frame += 1;
                    if(NPC[A].Frame >= 4)
                        NPC[A].Frame = 2;
                }
            }
        }
        else
        {
            if(NPC[A].Special2 < 10 || NPC[A].Special2 > 400 - 10)
                NPC[A].Frame = 4;
            else
                NPC[A].Frame = 5;
            if(NPC[A].Direction == 1)
                NPC[A].Frame += 2;
        }
    }
    else if(NPC[A].Type == NPCID_MEDAL) // dragon coin
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount < 6)
            NPC[A].Frame = 0;
        else if(NPC[A].FrameCount < 12)
            NPC[A].Frame = 1;
        else if(NPC[A].FrameCount < 18)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount < 24)
            NPC[A].Frame = 3;
        else if(NPC[A].FrameCount < 30)
            NPC[A].Frame = 2;
        else if(NPC[A].FrameCount < 36)
            NPC[A].Frame = 1;
        else
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame = 0;
        }
    }
    else if(NPC[A]->IsACoin) // Coin
    {
        NPC[A].Frame = CoinFrame[3];
        if(NPC[A].Type == NPCID_COIN_S2)
            NPC[A].Frame = CoinFrame[2];
        if(NPC[A].Type == NPCID_RING)
            NPC[A].Frame = CoinFrame[3];
    }
    else if(NPC[A].Type == NPCID_ITEMGOAL) // Frame finder for Star/Flower/Mushroom Exit
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame += 1;
            if(NPC[A].Frame == 3)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_TOOTHY) // killer plant
    {
        // .vehiclePlr = A
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 2;
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
            NPC[A].Frame += 1;
        if(NPC[A].FrameCount >= 16)
            NPC[A].FrameCount = 0;
    }
    else if(NPC[A].Type == NPCID_TOOTHYPIPE) // killer pipe
    {
        if(NPC[A].HoldingPlayer == 0 && !Player[NPC[A].vehiclePlr].Controls.Run && !NPC[A].Projectile)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].Frame += 1;
                NPC[A].FrameCount = 0;
            }
            if(NPC[A].Frame >= 5)
                NPC[A].Frame = 0;
        }
        else
        {
            if(NPC[A].Direction == -1)
            {
                NPC[A].FrameCount += 1;
                if(NPC[A].FrameCount >= 4)
                {
                    NPC[A].Frame += 1;
                    NPC[A].FrameCount = 0;
                }
                if(NPC[A].Frame >= 10 || NPC[A].Frame < 5)
                    NPC[A].Frame = 5;
            }
            else
            {
                NPC[A].FrameCount += 1;
                if(NPC[A].FrameCount >= 4)
                {
                    NPC[A].Frame += 1;
                    NPC[A].FrameCount = 0;
                }
                if(NPC[A].Frame >= 15 || NPC[A].Frame < 10)
                    NPC[A].Frame = 10;
            }
        }
    }
    else if(NPC[A].Type == NPCID_LAVABUBBLE) // Frame finder for big fireball
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame += 1;
            if(NPC[A].Location.SpeedY < 0)
            {
                if(NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
            }
            else
            {
                if(NPC[A].Frame >= 4)
                    NPC[A].Frame = 2;
            }
        }
    }
    else if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_HEAVY_THROWN || NPC[A].Type == NPCID_PLANT_FIRE || NPC[A].Type == NPCID_PLR_ICEBALL) // Frame finder for Fireball / Hammer
    {
        if((NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL) && NPC[A].Quicksand == 0)
        {
            if(NPC[A].Wet > 0)
            {
                if(iRand(20) == 0)
                {
                    tempLocation = newLoc(NPC[A].Location.X + 4, NPC[A].Location.Y + 4, 8, 8);
                    if(!UnderWater[NPC[A].Section])
                        NewEffect(EFFID_AIR_BUBBLE, tempLocation, 1, 0, NPC[A].Shadow);
                    else
                        NewEffect(EFFID_AIR_BUBBLE, tempLocation, 1, 1, NPC[A].Shadow);
                }

                if(iRand(100) >= 85)
                {
                    if(NPC[A].Type == NPCID_PLR_ICEBALL)
                    {
                        if(NPC[A].Special == 5)
                        {
                            NewEffect(EFFID_PLR_ICEBALL_TRAIL, NPC[A].Location, 1, 0, NPC[A].Shadow);
                            if(iRand(5) == 0)
                            {
                                tempLocation.Height = EffectHeight[80];
                                tempLocation.Width = EffectWidth[80];
                                tempLocation.SpeedX = 0;
                                tempLocation.SpeedY = 0;
                                tempLocation.X = NPC[A].Location.X + dRand() * 16 - EffectWidth[80] / 2.0 - 4 - NPC[A].Location.SpeedX * 3;
                                tempLocation.Y = NPC[A].Location.Y + dRand() * 16 - EffectHeight[80] / 2.0 - 4;
                                NewEffect(EFFID_SPARKLE, tempLocation);
                                Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.5;
                                Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY * 0.5;
                                Effect[numEffects].Frame = iRand(3);
                            }
                        }
                        else if(iRand(5) >= 3)
                        {
                            tempLocation.Height = EffectHeight[80];
                            tempLocation.Width = EffectWidth[80];
                            tempLocation.SpeedX = 0;
                            tempLocation.SpeedY = 0;
                            tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
                            tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
                            NewEffect(EFFID_SPARKLE, tempLocation, 1, 0, NPC[A].Shadow);
                            Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.25;
                            Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY * 0.25;
                            Effect[numEffects].Frame = iRand(3);
                        }
                    }
                    else
                        NewEffect(EFFID_PLR_FIREBALL_TRAIL, NPC[A].Location, static_cast<float>(NPC[A].Special), 0, NPC[A].Shadow);
                }
            }
            else
            {
                if(NPC[A].Type == NPCID_PLR_ICEBALL)
                {
                    if(NPC[A].Special == 5)
                    {
                        NewEffect(EFFID_PLR_ICEBALL_TRAIL, NPC[A].Location, 1, 0, NPC[A].Shadow);
                        if(iRand(5) == 0)
                        {
                            tempLocation.Height = EffectHeight[80];
                            tempLocation.Width = EffectWidth[80];
                            tempLocation.SpeedX = 0;
                            tempLocation.SpeedY = 0;
                            tempLocation.X = NPC[A].Location.X + dRand() * 16 - EffectWidth[80] / 2.0 - 4 - NPC[A].Location.SpeedX * 3;
                            tempLocation.Y = NPC[A].Location.Y + dRand() * 16 - EffectHeight[80] / 2.0 - 4;
                            NewEffect(EFFID_SPARKLE, tempLocation);
                            Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.5;
                            Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY * 0.5;
                            Effect[numEffects].Frame = iRand(3);
                        }
                    }
                    else if(iRand(5) >= 3)
                    {
                        tempLocation.Height = EffectHeight[80];
                        tempLocation.Width = EffectWidth[80];
                        tempLocation.SpeedX = 0;
                        tempLocation.SpeedY = 0;
                        tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
                        tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
                        NewEffect(EFFID_SPARKLE, tempLocation, 1, 0, NPC[A].Shadow);
                        Effect[numEffects].Location.SpeedX = NPC[A].Location.SpeedX * 0.25;
                        Effect[numEffects].Location.SpeedY = NPC[A].Location.SpeedY * 0.25;
                        Effect[numEffects].Frame = iRand(3);
                    }
                }
                else
                    NewEffect(EFFID_PLR_FIREBALL_TRAIL, NPC[A].Location, static_cast<float>(NPC[A].Special), 0, NPC[A].Shadow);
            }
        }
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 0;
            NPC[A].Frame += -NPC[A].Direction;
        }
        if(NPC[A].Special < 2 || (NPC[A].Type == NPCID_PLR_ICEBALL && NPC[A].Special != 5))
        {
            if(NPC[A].Frame >= 4)
                NPC[A].Frame = 0;
            if(NPC[A].Frame < 0)
                NPC[A].Frame = 3;
        }
        else if(NPC[A].Special == 2 || (NPC[A].Type == NPCID_PLR_ICEBALL && NPC[A].Special == 5))
        {
            if(NPC[A].Frame >= 7)
                NPC[A].Frame = 4;
            if(NPC[A].Frame < 4)
                NPC[A].Frame = 6;
        }
        else if(NPC[A].Special == 3)
        {
            if(NPC[A].Frame >= 11)
                NPC[A].Frame = 8;
            if(NPC[A].Frame < 8)
                NPC[A].Frame = 10;
        }
        else if(NPC[A].Special == 4)
        {
            if(NPC[A].Frame >= 15)
                NPC[A].Frame = 12;
            if(NPC[A].Frame < 12)
                NPC[A].Frame = 14;
        }
        else if(NPC[A].Special == 5)
        {
            if(NPC[A].Frame >= 19)
                NPC[A].Frame = 16;
            if(NPC[A].Frame < 16)
                NPC[A].Frame = 18;
        }
    }
    else if(NPC[A].Type == NPCID_MINIBOSS) // Frame finder for Big Koopa
    {
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedY != 0)
                NPC[A].Frame = 0;
            else
            {
                if(NPC[A].FrameCount >= 0)
                    NPC[A].FrameCount += 1;
                else
                    NPC[A].FrameCount -= 1;
                if(NPC[A].FrameCount >= 5 || NPC[A].FrameCount <= -5)
                {
                    if(NPC[A].FrameCount >= 0)
                    {
                        NPC[A].Frame += 1;
                        NPC[A].FrameCount = 1;
                    }
                    else
                    {
                        NPC[A].Frame -= 1;
                        NPC[A].FrameCount = -1;
                    }
                    if(NPC[A].Frame >= 5)
                    {
                        NPC[A].Frame = 3;
                        NPC[A].FrameCount = -1;
                    }
                    else if(NPC[A].Frame <= 0)
                    {
                        NPC[A].Frame = 2;
                        NPC[A].FrameCount = 1;
                    }
                }
            }
        }
        else if(NPC[A].Special == 1)
            NPC[A].Frame = 6;
        else if(NPC[A].Special == 4)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].Frame < 7)
                NPC[A].Frame = 7;
            if(NPC[A].FrameCount >= 8)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Frame == 7)
                    NPC[A].Frame = 8;
                else
                    NPC[A].Frame = 7;
            }
        }
        else
            NPC[A].Frame = 5;
    }
    else if(NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4) // Thwomp
    {
        // Bullet Bills / Key / ONLY DIRECTION FRAMES
    }
    else if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET || NPC[A].Type == NPCID_KEY || NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_CIVILIAN || NPC[A].Type == NPCID_CHAR3 || NPCIsYoshi(NPC[A]) || NPC[A].Type == NPCID_CHAR2 || NPC[A].Type == NPCID_CHAR5 || NPC[A].Type == NPCID_STATUE_S4)
    {
        if(NPC[A].Direction == -1)
            NPC[A].Frame = 0;
        else
            NPC[A].Frame = 1;
        // Leaf
    }
    else if(NPC[A].Type == NPCID_LEAF_POWER)
    {
        if(NPC[A].Direction == -1)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
    }
    else if(NPC[A].Type == NPCID_WALK_BOMB_S2 && NPC[A].Special2 == 1)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount < 4)
            NPC[A].Frame = 8;
        else if(NPC[A].FrameCount < 8)
            NPC[A].Frame = 9;
        else if(NPC[A].FrameCount < 11)
            NPC[A].Frame = 10;
        else
        {
            NPC[A].Frame = 10;
            NPC[A].FrameCount = 0;
        }
        if(NPC[A].HoldingPlayer > 0 || NPC[A].Projectile)
            NPC[A].Frame += 6;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 3;
    }
    else if(NPC[A].Type == NPCID_BLU_GUY || NPC[A].Type == NPCID_RED_GUY || NPC[A].Type == NPCID_RED_FISH_S1 || (NPC[A].Type >= NPCID_BIRD && NPC[A].Type <= NPCID_GRY_SPIT_GUY) || NPC[A].Type == NPCID_WALK_BOMB_S2 || NPC[A].Type == NPCID_SATURN) // Shy guys / Jumping Fish
    {
        if(NPC[A].HoldingPlayer == 0 && !NPC[A].Projectile)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].Direction == -1 && NPC[A].Frame >= 2)
                NPC[A].Frame = 0;
            else if(NPC[A].Direction == 1 && NPC[A].Frame < 2)
                NPC[A].Frame = 2;
            if(NPC[A].FrameCount >= 8)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Direction == -1)
                {
                    NPC[A].Frame += 1;
                    if(NPC[A].Frame >= 2)
                        NPC[A].Frame = 0;
                }
                else
                {
                    NPC[A].Frame += 1;
                    if(NPC[A].Frame >= 4)
                        NPC[A].Frame = 2;
                }
            }
        }
        else
        {
            if(NPC[A].Frame < 4)
                NPC[A].Frame = 4;
            NPC[A].FrameCount += 1;
            if(NPC[A].Direction == -1 && NPC[A].Frame >= 6)
                NPC[A].Frame = 4;
            else if(NPC[A].Direction == 1 && NPC[A].Frame < 6)
                NPC[A].Frame = 6;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Direction == -1)
                {
                    NPC[A].Frame += 1;
                    if(NPC[A].Frame >= 6)
                        NPC[A].Frame = 4;
                }
                else
                {
                    NPC[A].Frame += 1;
                    if(NPC[A].Frame >= 8)
                        NPC[A].Frame = 6;
                }
            }
        }
    }
    else if(NPC[A].Type == NPCID_JUMPER_S3) // Bouncy Star things
    {
        if(NPC[A].HoldingPlayer == 0 && !NPC[A].Projectile)
        {
            if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Frame = 0;
                else if(NPC[A].Direction == 1)
                    NPC[A].Frame = 2;
            }
            else
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Frame = 1;
                else if(NPC[A].Direction == 1)
                    NPC[A].Frame = 3;
            }
        }
        else
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].Direction == -1 && NPC[A].Frame >= 6)
                NPC[A].Frame = 4;
            else if(NPC[A].Direction == 1 && NPC[A].Frame < 6)
                NPC[A].Frame = 6;
            if(NPC[A].FrameCount >= 4)
            {
                NPC[A].FrameCount = 0;
                if(NPC[A].Direction == -1)
                {
                    NPC[A].Frame += 1;
                    if(NPC[A].Frame >= 6)
                        NPC[A].Frame = 4;
                }
                else
                {
                    NPC[A].Frame += 1;
                    if(NPC[A].Frame >= 8)
                        NPC[A].Frame = 6;
                }
            }
        }
    }
    else if(NPC[A].Type == NPCID_CANNONITEM) // Bullet bill Gun
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame += 1;
            if(NPC[A].Frame == 5)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_PINK_CIVILIAN)
    {
        if(NPC[A].Location.SpeedX == 0)
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 7)
                NPC[A].Frame = 1;
            else
            {
                NPC[A].Frame = 1;
                NPC[A].FrameCount = 0;
            }
        }
        else
        {
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount < 4)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount < 12)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount < 15)
                NPC[A].Frame = 3;
            else
            {
                NPC[A].Frame = 3;
                NPC[A].FrameCount = 0;
            }
        }
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 4;
    }
    else if(NPC[A].Type == NPCID_SPRING) // Spring thing
    {
        if(!LevelEditor)
        {
            if(NPC[A].Location.Height == 32)
            {
                NPC[A].Location.Height = 16;
                NPC[A].Location.Y += 16;
            }
            if(NPC[A].HoldingPlayer > 0)
                NPC[A].Frame = 0;
            else
            {
                C = 0;
                tempLocation = NPC[A].Location;
                tempLocation.Height = 24;
                tempLocation.Y -= 8;
                for(B = 1; B <= numPlayers; ++B)
                {
                    if(CheckCollision(tempLocation, Player[B].Location) && Player[B].Mount != 2 && (Player[B].Location.SpeedY > 0 || Player[B].Location.SpeedY < Physics.PlayerJumpVelocity))
                    {
                        C = 2;
                        break;
                    }
                }
                if(C == 0)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.Height = 32;
                    tempLocation.Y -= 16;
                    for(B = 1; B <= numPlayers; ++B)
                    {
                        if(CheckCollision(tempLocation, Player[B].Location) && Player[B].Mount != 2 && (Player[B].Location.SpeedY > 0 || Player[B].Location.SpeedY < Physics.PlayerJumpVelocity))
                        {
                            C = 1;
                            break;
                        }
                    }
                }
                NPC[A].Frame = C;
            }
        }
    }
    else if(NPC[A].Type == NPCID_SPIT_BOSS) // birdo
    {
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 5;
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedX != 0)
            {
                NPC[A].FrameCount += 1;
                if(NPC[A].FrameCount > 12)
                    NPC[A].FrameCount = 0;
                else if(NPC[A].FrameCount >= 6)
                    NPC[A].Frame += 1;
            }
        }
        else if(NPC[A].Special < 0)
        {
            NPC[A].Frame += 3;
            NPC[A].FrameCount += 1;
            if(NPC[A].FrameCount > 8)
                NPC[A].FrameCount = 0;
            else if(NPC[A].FrameCount >= 4)
                NPC[A].Frame += 1;
        }
        else
            NPC[A].Frame += 2;
    }
    else if(NPC[A].Type == NPCID_KNIGHT) // Rat Head
    {
        NPC[A].Frame = NPC[A].FrameCount;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 2;
    }
    else if(NPC[A].Type == NPCID_HEAVY_THROWER) // SMB Hammer Bro
    {
        if(NPC[A].Special3 >= 0)
        {
            if((NPC[A].Location.SpeedY < 1 && NPC[A].Location.SpeedY >= 0) || NPC[A].Slope > 0 || NPC[A].HoldingPlayer > 0)
            {
                NPC[A].FrameCount += 1;
                if(NPC[A].Direction == -1 && NPC[A].Frame >= 2)
                    NPC[A].Frame = 0;
                else if(NPC[A].Direction == 1 && NPC[A].Frame < 3)
                    NPC[A].Frame = 3;
                if(NPC[A].FrameCount >= 8)
                {
                    NPC[A].FrameCount = 0;
                    if(NPC[A].Direction == -1)
                    {
                        NPC[A].Frame += 1;
                        if(NPC[A].Frame >= 2)
                            NPC[A].Frame = 0;
                    }
                    else
                    {
                        NPC[A].Frame += 1;
                        if(NPC[A].Frame >= 5)
                            NPC[A].Frame = 3;
                    }
                }
            }
            else
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Frame = 0;
                else
                    NPC[A].Frame = 3;
            }
        }
        else
        {
            if(NPC[A].Direction == -1)
                NPC[A].Frame = 2;
            else
                NPC[A].Frame = 5;
        }
    }
    else if(NPC[A].Type == NPCID_PET_FIRE) // Yoshi Fireball
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].Frame = 1;
            NPC[A].FrameCount = 0;
        }
        else if(NPC[A].FrameCount > 4)
            NPC[A].Frame = 1;
        else
            NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame += 2;
    }
    else if(NPC[A].Type == NPCID_GRN_BOOT || NPC[A].Type == NPCID_RED_BOOT || NPC[A].Type == NPCID_BLU_BOOT) // Goombas Shoe
    {
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 2 + SpecialFrame[1];
        else
            NPC[A].Frame = 0 + SpecialFrame[1];
    }
    else if(NPC[A].Type == NPCID_GHOST_S3 || NPC[A].Type == NPCID_GHOST_S4 || NPC[A].Type == NPCID_BIG_GHOST) // Boo
    {
        NPC[A].Frame = 0;
        if(NPC[A].Direction == 1)
            NPC[A].Frame = 2;
        if(NPC[A].Special == 1 || NPC[A].HoldingPlayer > 0)
            NPC[A].Frame += 1;
    }
    else if(NPC[A].Type == NPCID_GOALORB_S2) // smb2 birdo exit
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame += 1;
            if(NPC[A].Frame == 8)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_STAR_EXIT) // SMB3 Star
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].Special == 0)
        {
            if(NPC[A].FrameCount < 8)
                NPC[A].Frame = 0;
            else if(NPC[A].FrameCount < 12)
                NPC[A].Frame = 1;
            else if(NPC[A].FrameCount < 16)
                NPC[A].Frame = 2;
            else if(NPC[A].FrameCount < 20)
                NPC[A].Frame = 1;
            else
                NPC[A].FrameCount = 0;
        }
        else
        {
            if(NPC[A].FrameCount < 60)
                NPC[A].Frame = 2;
            // ElseIf .FrameCount < 64 Then
            // .Frame = 1
            else
                NPC[A].FrameCount = 0;
        }
    }
    else if(!(NPC[A]->IsABonus || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_COIN_SWITCH)) // Frame finder for everything else
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].Type == NPCID_SPIKY_BALL_S3 || NPC[A].Type == NPCID_WALL_SPARK)
            NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 8)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame += 1;
            if(NPC[A].Frame == 2)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_FIRE_POWER_S4 || NPC[A].Type == NPCID_ICE_POWER_S4)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 12)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame += 1;
            if(NPC[A].Frame == 2)
                NPC[A].Frame = 0;
        }
    }
    else if(NPC[A].Type == NPCID_FIRE_POWER_S1)
    {
        NPC[A].FrameCount += 1;
        if(NPC[A].FrameCount >= 4)
        {
            NPC[A].FrameCount = 1;
            NPC[A].Frame += 1;
            if(NPC[A].Frame == 4)
                NPC[A].Frame = 0;
        }
    }
    else
    {
        if(A == 0)
            NPC[A].Frame = 0;
    }
}
