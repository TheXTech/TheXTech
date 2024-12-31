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

static inline bool s_use_default_movement(int A)
{
    return (NPCDefaultMovement(NPC[A]) || (NPC[A]->IsFish && NPC[A].Special != 2)) && !((NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4) && NPC[A].Special > 0);
}

void NPCMovementLogic(int A, float& speedVar)
{
    // POSSIBLE SUBROUTINE: setSpeed

    // Default Movement Code
    if(s_use_default_movement(A) && NPC[A].Type != NPCID_ITEM_BURIED)
    {
        if(NPC[A].Direction == 0)
        {
            if(iRand(2) == 0)
                NPC[A].Direction = -1;
            else
                NPC[A].Direction = 1;
        }

        if(NPC[A]->CanWalkOn)
        {
            if(NPC[A].Location.SpeedX < Physics.NPCWalkingOnSpeed && NPC[A].Location.SpeedX > -Physics.NPCWalkingOnSpeed)
            {
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX = Physics.NPCWalkingOnSpeed * NPC[A].Direction;
            }

            if(NPC[A].Location.SpeedX > Physics.NPCWalkingOnSpeed)
            {
                NPC[A].Location.SpeedX -= 0.05;
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX -= 0.1;
            }
            else if(NPC[A].Location.SpeedX < -Physics.NPCWalkingOnSpeed)
            {
                NPC[A].Location.SpeedX += 0.05;
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX += 0.1;
            }
        }
        else if(NPC[A].Type == NPCID_KNIGHT)
        {
            if(NPC[A].Location.SpeedX < 2 && NPC[A].Location.SpeedX > -2)
            {
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
            }

            if(NPC[A].Location.SpeedX > 2)
                NPC[A].Location.SpeedX -= 0.05;
            else if(NPC[A].Location.SpeedX < -2)
                NPC[A].Location.SpeedX += 0.05;
        }
        else if(!(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4 && NPC[A].Projectile))
        {
            if(NPC[A].Location.SpeedX < Physics.NPCWalkingSpeed && NPC[A].Location.SpeedX > -Physics.NPCWalkingSpeed)
            {
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX = Physics.NPCWalkingSpeed * NPC[A].Direction;
            }

            if(NPC[A].Location.SpeedX > Physics.NPCWalkingSpeed)
                NPC[A].Location.SpeedX -= 0.05;
            else if(NPC[A].Location.SpeedX < -Physics.NPCWalkingSpeed)
                NPC[A].Location.SpeedX += 0.05;
        }
    }
    else if(NPC[A].Type == NPCID_FLIER)
    {
        if(NPC[A].Location.SpeedX > -2 && NPC[A].Location.SpeedX < 2)
            NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
    }
    else if(NPC[A].Type == NPCID_ROCKET_FLIER)
    {
        if(NPC[A].Location.SpeedX > -2.5 && NPC[A].Location.SpeedX < 2.5)
            NPC[A].Location.SpeedX = 2.5 * NPC[A].Direction;
    }
    // Slow things down that shouldnt move
    else if(NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_JUMPER_S3 || NPC[A].Type == NPCID_SPRING ||
            NPC[A].Type == NPCID_KEY || NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_TNT ||
            NPC[A].Type == NPCID_GRN_BOOT || NPC[A].Type == NPCID_RED_BOOT || NPC[A].Type == NPCID_BLU_BOOT ||
            (NPC[A].Type == NPCID_SPIT_BOSS_BALL && NPC[A].Projectile) || NPC[A].Type == NPCID_TOOTHYPIPE || NPC[A].Type == NPCID_METALBARREL ||
            NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG ||
            (NPCIsVeggie(NPC[A]) && !NPC[A].Projectile) ||
            (NPC[A].Type == NPCID_HEAVY_THROWER && NPC[A].Projectile) ||
            /*(NPC[A].Projectile && (NPC[A].Type == NPCID_FLY && NPC[A].Type == NPCID_MINIBOSS)) ||*/ // FIXME: This segment is always false (type equal both 54 and 15, impossible!) [PVS Studio]
            NPC[A].Type == NPCID_CIVILIAN_SCARED || NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_STATUE_S4 || NPC[A].Type == NPCID_CIVILIAN ||
            NPC[A].Type == NPCID_CHAR3 || NPC[A].Type == NPCID_ITEM_POD || NPC[A].Type == NPCID_BOMB || NPC[A].Type == NPCID_LIT_BOMB_S3 ||
            NPC[A].Type == NPCID_CHAR2 || NPC[A].Type == NPCID_CHAR5 || (NPCIsYoshi(NPC[A]) && NPC[A].Special == 0) ||
            (NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D) || NPC[A].Type == NPCID_HIT_CARRY_FODDER || (NPC[A].Type == NPCID_SPIT_BOSS && NPC[A].Projectile) ||
            NPC[A].Type == NPCID_HEAVY_POWER || NPC[A].Type == NPCID_STATUE_POWER || NPC[A].Type == NPCID_FIRE_POWER_S4 || NPC[A].Type == NPCID_3_LIFE ||
            NPC[A].Type == NPCID_STAR_EXIT || NPC[A].Type == NPCID_STAR_COLLECT || NPC[A].Type == NPCID_FIRE_POWER_S1 || NPC[A].Type == NPCID_TIMER_S2 ||
            NPC[A].Type == NPCID_EARTHQUAKE_BLOCK || NPC[A].Type == NPCID_POWER_S2 || NPC[A].Type == NPCID_POWER_S5 || NPC[A].Type == NPCID_FLY_POWER ||
            NPC[A].Type == NPCID_LOCK_DOOR || NPC[A].Type == NPCID_FLY_BLOCK || NPC[A].Type == NPCID_FLY_CANNON || NPC[A].Type == NPCID_ICE_POWER_S4 ||
            NPC[A].Type == NPCID_ICE_POWER_S3 || NPC[A].Type == NPCID_DOOR_MAKER || NPC[A].Type == NPCID_QUAD_SPITTER)
    {
        if(NPC[A].Location.SpeedX > 0)
            NPC[A].Location.SpeedX -= 0.05;
        else if(NPC[A].Location.SpeedX < 0)
            NPC[A].Location.SpeedX += 0.05;

        if(NPC[A].Location.SpeedX >= -0.05 && NPC[A].Location.SpeedX <= 0.05)
            NPC[A].Location.SpeedX = 0;

        if(NPC[A].Location.SpeedY >= -Physics.NPCGravity && NPC[A].Location.SpeedY <= Physics.NPCGravity)
        {
            if(NPC[A].Location.SpeedX > 0)
                NPC[A].Location.SpeedX -= 0.3;
            else if(NPC[A].Location.SpeedX < 0)
                NPC[A].Location.SpeedX += 0.3;

            if(NPC[A].Location.SpeedX >= -0.3 && NPC[A].Location.SpeedX <= 0.3)
                NPC[A].Location.SpeedX = 0;
        }
    }
#if 0
    // dead code because NPCDefaultMovement(NPCID_TANK_TREADS) is true
    else if(NPC[A].Type == NPCID_TANK_TREADS)
    {
        NPC[A].Projectile = true;
        NPC[A].Direction = NPC[A].DefaultDirection;
        NPC[A].Location.SpeedX = 1 * NPC[A].DefaultDirection;

        // the conditions here are outdated
        for(int B = 1; B <= numPlayers; B++)
        {
            if(!(Player[B].Effect == PLREFF_NORMAL || Player[B].Effect == PLREFF_WARP_PIPE))
            {
                NPC[A].Location.SpeedX = 0;
                NPC[A].Location.SpeedY = 0;
            }
        }
    }
#endif
    // Mushroom Movement Code
    else if(NPC[A].Type == NPCID_POWER_S3 || NPC[A].Type == NPCID_SWAP_POWER || NPC[A].Type == NPCID_LIFE_S3 ||
            NPC[A].Type == NPCID_POISON || NPC[A].Type == NPCID_POWER_S1 || NPC[A].Type == NPCID_POWER_S4 ||
            NPC[A].Type == NPCID_LIFE_S1 || NPC[A].Type == NPCID_LIFE_S4 || NPC[A].Type == NPCID_BRUTE_SQUISHED ||
            NPC[A].Type == NPCID_BIG_MOLE)
    {
        if(NPC[A].Direction == 0.0f) // Move toward the closest player
        {
            double C = 0;
            for(int B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                {
                    double dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                    if(C == 0.0 || dist < C)
                    {
                        C = dist;
                        NPC[A].Direction = -Player[B].Direction;
                    }
                }
            }
        }

        if(NPC[A].Location.SpeedX < Physics.NPCMushroomSpeed && NPC[A].Location.SpeedX > -Physics.NPCMushroomSpeed)
        {
            if(!NPC[A].Projectile)
                NPC[A].Location.SpeedX = Physics.NPCMushroomSpeed * NPC[A].Direction;
        }

        if(NPC[A].Location.SpeedX > Physics.NPCMushroomSpeed)
            NPC[A].Location.SpeedX -= 0.05;
        else if(NPC[A].Location.SpeedX < -Physics.NPCMushroomSpeed)
            NPC[A].Location.SpeedX += 0.05;
    }
    else if(NPC[A].Type == NPCID_RAINBOW_SHELL)
    {
        NPC[A].Projectile = true;

        double C = 0;
        for(int B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
            {
                double dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                if(C == 0.0 || dist < C)
                {
                    C = dist;
                    if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                        NPC[A].Direction = -1;
                    else
                        NPC[A].Direction = 1;
                }
            }
        }

        NPC[A].Location.SpeedX += 0.1 * double(NPC[A].Direction);

        if(NPC[A].Location.SpeedX < -4)
            NPC[A].Location.SpeedX = -4;

        if(NPC[A].Location.SpeedX > 4)
            NPC[A].Location.SpeedX = 4;
    }
    // Yoshi Fireball
    else if(NPC[A].Type == NPCID_PET_FIRE)
    {
        NPC[A].Projectile = true;
        if(NPC[A].Location.SpeedX == 0)
            NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
    }
    // bully
    else if(NPC[A].Type == NPCID_BULLY)
    {
        if(!NPC[A].Projectile && NPC[A].Special2 == 0)
        {
            double C = 0;
            for(int B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                {
                    double dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                    if(C == 0.0 || dist < C)
                    {
                        C = dist;
                        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                            NPC[A].Direction = -1;
                        else
                            NPC[A].Direction = 1;
                    }
                }
            }

            NPC[A].Location.SpeedX += 0.05 * double(NPC[A].Direction);

            if(NPC[A].Location.SpeedX >= 3)
                NPC[A].Location.SpeedX = 3;

            if(NPC[A].Location.SpeedX <= -3)
                NPC[A].Location.SpeedX = -3;
        }
        else
        {
            if(NPC[A].Location.SpeedX > 0.1)
                NPC[A].Location.SpeedX -= 0.075;
            else if(NPC[A].Location.SpeedX < -0.1)
                NPC[A].Location.SpeedX += 0.075;

            if(NPC[A].Location.SpeedX >= -0.1 && NPC[A].Location.SpeedX <= 0.1)
                NPC[A].Special2 = 0;
        }
    }
    else if(NPC[A].Type == NPCID_RAFT)
    {
        if(NPC[A].Special == 1)
            NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
    }
    // Big Koopa Movement Code
    else if(NPC[A].Type == NPCID_MINIBOSS)
    {
        if(NPC[A].Location.SpeedX < 0)
            NPC[A].Direction = -1;
        else
            NPC[A].Direction = 1;

        if(NPC[A].Special == 0 || NPC[A].Special == 3)
        {
            if(NPC[A].Location.SpeedX < 3.5 && NPC[A].Location.SpeedX > -3.5)
                NPC[A].Location.SpeedX += (0.1 * NPC[A].Direction);

            if(NPC[A].Location.SpeedX > 3.5)
                NPC[A].Location.SpeedX -= 0.05;
            else if(NPC[A].Location.SpeedX < -3.5)
                NPC[A].Location.SpeedX += 0.05;

            if(NPC[A].Special == 3)
                NPC[A].Location.SpeedY = -6;
        }
        else if(NPC[A].Special == 2)
            NPC[A].Location.SpeedX += (0.2 * NPC[A].Direction);
        else if(NPC[A].Special == 3)
            NPC[A].Location.SpeedY = -6;
        else
        {
            if(NPC[A].Location.SpeedX > 0)
                NPC[A].Location.SpeedX -= 0.05;
            else if(NPC[A].Location.SpeedX < 0)
                NPC[A].Location.SpeedX += 0.05;

            if(NPC[A].Location.SpeedX > -0.5 && NPC[A].Location.SpeedX < 0.5)
                NPC[A].Location.SpeedX = 0.0001 * NPC[A].Direction;
        }
    }
    // spiney eggs
    else if(NPC[A].Type == NPCID_SPIKY_BALL_S3)
    {
        if(NPC[A].CantHurt > 0)
        {
            NPC[A].Projectile = true;
            NPC[A].CantHurt = 100;
        }
        else
        {
            double C = 0;
            for(int B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                {
                    double dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                    if(C == 0.0 || dist < C)
                    {
                        C = dist;
                        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                            NPC[A].Direction = -1;
                        else
                            NPC[A].Direction = 1;
                    }
                }
            }

            if(NPC[A].Direction == 1 && NPC[A].Location.SpeedX < 4)
                NPC[A].Location.SpeedX += 0.04;
            if(NPC[A].Direction == -1 && NPC[A].Location.SpeedX > -4)
                NPC[A].Location.SpeedX -= 0.04;
        }
    }
    else if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET)
    {
        if(NPC[A].CantHurt < 1000)
            NPC[A].Location.SpeedX = 4 * NPC[A].Direction;
    }
    else if(NPC[A].Type == NPCID_GHOST_FAST)
        NPC[A].Location.SpeedX = 2 * double(NPC[A].Direction);

    // yoshi
    if(NPCIsYoshi(NPC[A]))
    {
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
            {
                if(NPC[A].Wet == 0)
                    NPC[A].Location.SpeedY = -2.1;
                else
                    NPC[A].Location.SpeedY = -1.1;
            }
        }
        else
        {
            if(NPC[A].Location.SpeedX < 3 && NPC[A].Location.SpeedX > -3)
            {
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX = 3 * NPC[A].Direction;
            }
        }
    }

    if(NPC[A].Type != NPCID_SPIT_BOSS && NPC[A].Type != NPCID_FALL_BLOCK_RED && NPC[A].Type != NPCID_FALL_BLOCK_BROWN && NPC[A].Type != NPCID_VEHICLE &&
       NPC[A].Type != NPCID_CONVEYOR && NPC[A].Type != NPCID_YEL_PLATFORM && NPC[A].Type != NPCID_BLU_PLATFORM && NPC[A].Type != NPCID_GRN_PLATFORM &&
       NPC[A].Type != NPCID_RED_PLATFORM && NPC[A].Type != NPCID_STATUE_S3 && NPC[A].Type != NPCID_STATUE_S4 && NPC[A].Type != NPCID_STATUE_FIRE &&
       NPC[A].Type != NPCID_CANNONITEM && NPC[A].Type != NPCID_TOOTHYPIPE && NPC[A].Type != NPCID_TOOTHY && !(NPC[A].Type >= NPCID_PLATFORM_S3 &&
       NPC[A].Type <= NPCID_PLATFORM_S1))
    {
        if(NPC[A].Location.SpeedX < 0) // Find the NPCs direction
            NPC[A].Direction = -1;
        else if(NPC[A].Location.SpeedX > 0)
            NPC[A].Direction = 1;
    }

    // Reset Speed when no longer a projectile
    // If Not (NPCIsAShell(.Type) Or .Type = 8 Or .Type = 93 Or .Type = 74 Or .Type = 51 Or .Type = 52 Or .Type = 12 Or .Type = 14 Or .Type = 13 Or .Type = 15 Or NPCIsABonus(.Type) Or .Type = 17 Or .Type = 18 Or .Type = 21 Or .Type = 22 Or .Type = 25 Or .Type = 26 Or .Type = 29 Or .Type = 30 Or .Type = 31 Or .Type = 32 Or .Type = 35 Or .Type = 37 Or .Type = 38 Or .Type = 39 Or .Type = 40 Or .Type = 42 Or .Type = 43 Or .Type = 44 Or .Type = 45 Or .Type = 46 Or .Type = 47 Or .Type = 48 Or .Type = 76 Or .Type = 49 Or .Type = 54 Or .Type = 56 Or .Type = 57 Or .Type = 58 Or .Type = 60 Or .Type = 62 Or .Type = 64 Or .Type = 66 Or .Type = 67 Or .Type = 68 Or .Type = 69 Or .Type = 70 Or .Type = 78 Or .Type = 84 Or .Type = 85 Or .Type = 87 Or (.Type = 55 And .Special > 0) Or (.Type >= 79 And .Type <= 83) Or .Type = 86 Or .Type = 92 Or .Type = 94 Or NPCIsYoshi(.Type) Or .Type = 96 Or .Type = 101 Or .Type = 102) And .Projectile = False Then
    if(s_use_default_movement(A) && !NPC[A].Projectile)
    {
        if(!NPC[A]->CanWalkOn)
        {
            if(NPC[A]->CanWalkOn)
                NPC[A].Location.SpeedX = Physics.NPCWalkingOnSpeed * NPC[A].Direction;
            else if(NPC[A].Type == NPCID_KNIGHT)
                NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
            else
                NPC[A].Location.SpeedX = Physics.NPCWalkingSpeed * NPC[A].Direction;


            if((NPC[A]->IsFish && NPC[A].Special != 1) && !NPC[A].Projectile)
            {
                // NOTE: SpeedX was previously stored in Special3. That value was only read here (fish with Special == 1 use Special3 in a different way).
                if(NPC[A].Wet == 0)
                {
                    if(NPC[A].Special5 >= 0)
                        NPC[A].Special2 -= 1;
                }
                else
                {
                    NPC[A].Special2 = 6;
                    NPC[A].SpecialX = NPC[A].Location.SpeedX;
                }

                if(NPC[A].Special2 <= 0)
                {
                    NPC[A].SpecialX = NPC[A].SpecialX * 0.99;
                    if(NPC[A].SpecialX > -0.1 && NPC[A].SpecialX < 0.1)
                        NPC[A].SpecialX = 0;
                    NPC[A].Location.SpeedX = NPC[A].SpecialX;
                }
            }

            if(NPC[A]->IsFish && NPC[A].Special == 1 && !NPC[A].Projectile)
                NPC[A].Location.SpeedX = Physics.NPCWalkingOnSpeed * 2 * NPC[A].Direction;
        }
    }

    if(NPC[A].Type == NPCID_WALK_BOMB_S2 && !NPC[A].Projectile && NPC[A].Special2 == 1)
        NPC[A].Location.SpeedX = 0;



    // NPC Gravity
    if(!NPC[A]->NoGravity)
    {
        // POSSIBLE SUBROUTINE: calcGravity

        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL)
        {
            NPC[A].CantHurt = 100;
            if(NPC[A].Special < 2)
                NPC[A].Location.SpeedY += Physics.NPCGravity * 1.5;
            else if(NPC[A].Special == 3)
            {
                // peach fireball changes
                NPC[A].Location.SpeedY += Physics.NPCGravity * 0.9;
                if(NPC[A].Location.SpeedX > 3)
                    NPC[A].Location.SpeedX -= 0.04;
                else if(NPC[A].Location.SpeedX < -3)
                    NPC[A].Location.SpeedX += 0.04;
            }
            else if(NPC[A].Special == 4)
            {

                // toad fireball changes
                NPC[A].Location.SpeedY += Physics.NPCGravity * 1.3;
                if(NPC[A].Location.SpeedX < 8 && NPC[A].Location.SpeedX > 0)
                    NPC[A].Location.SpeedX += 0.03;
                else if(NPC[A].Location.SpeedX > -8 && NPC[A].Location.SpeedX < 0)
                    NPC[A].Location.SpeedX -= 0.03;
            }
            else if(NPC[A].Special == 5) // link fireballs float
            {
            }
            else
                NPC[A].Location.SpeedY += Physics.NPCGravity * 1.3;

        }
        else if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET)
            NPC[A].Location.SpeedY = 0;

        else if((NPC[A]->IsFish && NPC[A].Special == 2) && !NPC[A].Projectile)
        {
            if(NPC[A].Special5 == 1)
            {
                if(NPC[A].Location.Y > NPC[A].DefaultLocationY)
                    NPC[A].Location.SpeedY = -4 - (NPC[A].Location.Y - NPC[A].DefaultLocationY) * 0.02;
                else
                    NPC[A].Special5 = 0;
            }
            else
            {
                // If .Location.SpeedY < 2 + (.Location.Y - .DefaultLocationY) * 0.02 Then
                NPC[A].Location.SpeedY += Physics.NPCGravity * 0.4;
                // End If
            }
        }
        else if(NPC[A].Type != NPCID_RED_VINE_TOP_S3 && NPC[A].Type != NPCID_GRN_VINE_TOP_S3 && NPC[A].Type != NPCID_GRN_VINE_TOP_S4 &&
                !(NPC[A]->IsFish && NPC[A].Special == 2) && NPC[A].Type != NPCID_HOMING_BALL &&
                NPC[A].Type != NPCID_HOMING_BALL_GEN && NPC[A].Type != NPCID_SPIT_GUY_BALL && NPC[A].Type != NPCID_STAR_EXIT && NPC[A].Type != NPCID_STAR_COLLECT &&
                NPC[A].Type != NPCID_VILLAIN_FIRE && NPC[A].Type != NPCID_PLANT_S3 && NPC[A].Type != NPCID_FIRE_PLANT && NPC[A].Type != NPCID_PLANT_FIRE &&
                NPC[A].Type != NPCID_PLANT_S1 && NPC[A].Type != NPCID_BIG_PLANT && NPC[A].Type != NPCID_LONG_PLANT_UP && NPC[A].Type != NPCID_LONG_PLANT_DOWN &&
                !NPCIsAParaTroopa(NPC[A]) && NPC[A].Type != NPCID_BOTTOM_PLANT && NPC[A].Type != NPCID_SIDE_PLANT &&
                NPC[A].Type != NPCID_LEAF_POWER && NPC[A].Type != NPCID_STONE_S3 && NPC[A].Type != NPCID_STONE_S4 && NPC[A].Type != NPCID_GHOST_S3 &&
                NPC[A].Type != NPCID_GHOST_FAST && NPC[A].Type != NPCID_GHOST_S4 && NPC[A].Type != NPCID_BIG_GHOST && NPC[A].Type != NPCID_SPIKY_THROWER &&
                NPC[A].Type != NPCID_VEHICLE && NPC[A].Type != NPCID_CONVEYOR && NPC[A].Type != NPCID_YEL_PLATFORM &&
                NPC[A].Type != NPCID_BLU_PLATFORM && NPC[A].Type != NPCID_GRN_PLATFORM && NPC[A].Type != NPCID_RED_PLATFORM &&
                NPC[A].Type != NPCID_STATUE_FIRE && !(NPC[A]->IsACoin && NPC[A].Special == 0) &&
                NPC[A].Type != NPCID_CHECKER_PLATFORM && NPC[A].Type != NPCID_PLATFORM_S1 && NPC[A].Type != NPCID_PET_FIRE &&
                NPC[A].Type != NPCID_GOALTAPE && NPC[A].Type != NPCID_LAVA_MONSTER && NPC[A].Type != NPCID_FLIER &&
                NPC[A].Type != NPCID_ROCKET_FLIER &&
                ((NPC[A].Type != NPCID_WALL_BUG && NPC[A].Type != NPCID_WALL_SPARK && NPC[A].Type != NPCID_WALL_TURTLE)) &&
                NPC[A].Type != NPCID_BOSS_FRAGILE && NPC[A].Type != NPCID_ITEM_BURIED &&
                NPC[A].Type != NPCID_MAGIC_BOSS_BALL && NPC[A].Type != NPCID_JUMP_PLANT && NPC[A].Type != NPCID_LOCK_DOOR)
        {
            if(NPC[A].Type != NPCID_BAT && NPC[A].Type != NPCID_VINE_BUG && NPC[A].Type != NPCID_QUAD_BALL && NPC[A].Type != NPCID_FIRE_BOSS_FIRE && NPC[A].Type != NPCID_ITEM_BUBBLE && NPC[A].Type != NPCID_ITEM_THROWER && NPC[A].Type != NPCID_MAGIC_DOOR && NPC[A].Type != NPCID_COCKPIT && NPC[A].Type != NPCID_CHAR3_HEAVY && NPC[A].Type != NPCID_CHAR4_HEAVY) // no gravity
            {
                if(NPC[A]->IsFish && NPC[A].Special == 4 && !NPC[A].Projectile)
                    NPC[A].Location.SpeedX = 0;
                if(NPC[A].Wet == 2 && (NPC[A].Type == NPCID_RAFT))
                    NPC[A].Location.SpeedY += -Physics.NPCGravity * 0.5;
                else if(NPC[A].Wet == 2 && NPC[A]->IsFish && NPC[A].Special != 2 && !NPC[A].Projectile) // Fish cheep
                {
                    if((NPC[A].Location.X < NPC[A].DefaultLocationX - 100 && NPC[A].Direction == -1) || (NPC[A].Location.X > NPC[A].DefaultLocationX + 100 && NPC[A].Direction == 1))
                    {
                        if(NPC[A].Special == 3)
                            NPC[A].TurnAround = true;
                    }

                    if(NPC[A].Special == 4)
                    {


                        if(NPC[A].Location.SpeedY == 0)
                            NPC[A].Special4 = 1;
                        if(NPC[A].Location.SpeedY == 0.01)
                            NPC[A].Special4 = 0;

                        NPC[A].Location.SpeedX = 0;
                        if(NPC[A].Location.SpeedY > 2)
                            NPC[A].Location.SpeedY = 2;
                        if(NPC[A].Location.SpeedY < -2)
                            NPC[A].Location.SpeedY = -2;
                        if(NPC[A].Location.Y > NPC[A].DefaultLocationY + 25)
                            NPC[A].Special4 = 1;
                        else if(NPC[A].Location.Y < NPC[A].DefaultLocationY - 25)
                            NPC[A].Special4 = 0;
                        if(NPC[A].Special4 == 0)
                            NPC[A].Location.SpeedY += 0.05;
                        else
                            NPC[A].Location.SpeedY -= 0.05;
                    }
                    else
                    {
                        if(NPC[A].Special4 == 0)
                        {
                            NPC[A].Location.SpeedY -= 0.025;
                            if(NPC[A].Location.SpeedY <= -1)
                                NPC[A].Special4 = 1;
                            if(NPC[A].Special == 3 && NPC[A].Location.SpeedY <= -0.5)
                                NPC[A].Special4 = 1;
                        }
                        else
                        {
                            NPC[A].Location.SpeedY += 0.025;
                            if(NPC[A].Location.SpeedY >= 1)
                                NPC[A].Special4 = 0;
                            if(NPC[A].Special == 3 && NPC[A].Location.SpeedY >= 0.5)
                                NPC[A].Special4 = 0;
                        }
                    }
                }
                else if(NPC[A]->IsFish && NPC[A].Special == 1 && NPC[A].Special5 == 1)
                    NPC[A].Location.SpeedY += Physics.NPCGravity * 0.6;
                else if(NPC[A].Type == NPCID_FLY_BLOCK || (g_config.fix_flamethrower_gravity && NPC[A].Type == NPCID_FLY_CANNON))
                {
                    NPC[A].Location.SpeedY += Physics.NPCGravity * 0.75;
                    if(NPC[A].Location.SpeedY > Physics.NPCGravity * 15)
                        NPC[A].Location.SpeedY = Physics.NPCGravity * 15;
                }
                else if(NPC[A].Type != NPCID_FIRE_DISK && NPC[A].Type != NPCID_FIRE_CHAIN)
                    NPC[A].Location.SpeedY += Physics.NPCGravity;
            }
        }


        if(NPC[A].Type == NPCID_CHAR3_HEAVY)
        {
            NPC[A].Location.SpeedY += Physics.NPCGravity * 0.8;
            // If .Location.SpeedY >= 5 Then .Location.SpeedY = 5
            if(NPC[A].Location.SpeedX < -0.005)
                NPC[A].Location.SpeedX += 0.02;
            else if(NPC[A].Location.SpeedX > 0.005)
                NPC[A].Location.SpeedX -= 0.02;
            else
                NPC[A].Location.SpeedX = 0;
        }
    }
    else if(NPC[A].Projectile)
    {
        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95;
        if(NPC[A].Location.SpeedY > -0.1 && NPC[A].Location.SpeedY < 0.1)
        {
            NPC[A].Projectile = false;
            NPC[A].Location.SpeedY = 0;
        }
    }

    if(NPC[A].Location.SpeedY >= 8 && NPC[A].Type != NPCID_FIRE_DISK && NPC[A].Type != NPCID_FIRE_CHAIN)
        NPC[A].Location.SpeedY = 8;

    // POSSIBLE SUBROUTINE: preMovement

    if(NPC[A].Type == NPCID_SPIT_BOSS_BALL)
    {
        if(!NPC[A].Projectile)
        {
            NPC[A].Location.SpeedY = 0; // egg code
            if(NPC[A].Location.SpeedX == 0)
                NPC[A].Projectile = true;
        }
    }

    if((NPC[A].Type == NPCID_SLIDE_BLOCK || NPC[A].Type == NPCID_FALL_BLOCK_RED || NPC[A].Type == NPCID_FALL_BLOCK_BROWN) && NPC[A].Special == 0)
        NPC[A].Location.SpeedY = 0;

    if(NPC[A].Type == NPCID_TOOTHY || NPC[A].Type == NPCID_HOMING_BALL_GEN)
    {
        NPC[A].Location.SpeedX = 0;
        NPC[A].Location.SpeedY = 0;
    }

    NPCSpecial(A);

    // lots of speed cancel code (and some TheXTech logic for the Raft NPC); fine to move into NPCSpecial
    if(NPC[A].Type == NPCID_TANK_TREADS)
    {
        if(!AllPlayersNormal())
            NPC[A].Location.SpeedX = 0;
    }

    if(NPC[A].Type == NPCID_ICE_CUBE)
    {
        if(NPC[A].Projectile || NPC[A].Wet > 0 || NPC[A].HoldingPlayer > 0)
            NPC[A].Special3 = 0;
        else if(NPC[A].Special3 == 1)
        {
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
        }
    }

    if((NPC[A].Type == NPCID_ITEM_POD && NPC[A].Special2 == 1) || NPC[A].Type == NPCID_SIGN || NPC[A].Type == NPCID_LIFT_SAND)
    {
        NPC[A].Location.SpeedX = 0;
        NPC[A].Location.SpeedY = 0;
    }
    else if(NPC[A].Type == NPCID_ROCKET_WOOD || NPC[A].Type == NPCID_3_LIFE)
        NPC[A].Location.SpeedY = 0;
    if(NPC[A].Type == NPCID_CHECKPOINT)
    {
        NPC[A].Projectile = false;
        NPC[A].Location.SpeedX = 0;
        NPC[A].Location.SpeedY = 0;
    }

    if(NPC[A].Type == NPCID_RAFT) // Skull raft
    {
        if(!AllPlayersNormal())
        {
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
        }

        // the following is all new code!

        if((NPC[A].Special == 2 || NPC[A].Special == 3) && (NPC[A].SpecialX != 0))
        {
            NPC[A].Location.X = NPC[A].SpecialX; // Finish alignment
            NPC[A].SpecialX = 0;
        }

        if(NPC[A].Special == 3) // Watch for wall collisions. If one got dissappear (hidden layer, toggled switch), resume a ride
        {
            auto loc = NPC[A].Location;
            loc.X += 1 * NPC[A].Direction;
            loc.SpeedX += 2 * NPC[A].Direction;

            // int64_t fBlock;// = FirstBlock[static_cast<int>(floor(static_cast<double>(loc.X / 32))) - 1];
            // int64_t lBlock;// = LastBlock[floor((loc.X + loc.Width) / 32.0) + 1];
            // blockTileGet(loc, fBlock, lBlock);
            bool stillCollide = false;

            for(BlockRef_t block : treeBlockQuery(loc, SORTMODE_NONE))
            {
                int B = block;
                if(!CheckCollision(loc, Block[B].Location))
                    continue;

                if(NPC[A].tempBlock == B || Block[B].tempBlockNoProjClipping() ||
                   BlockOnlyHitspot1[Block[B].Type] || BlockIsSizable[Block[B].Type] ||
                   BlockNoClipping[Block[B].Type] || Block[B].Hidden)
                {
                    continue;
                }

                int hs = NPCFindCollision(loc, Block[B].Location);
                if(Block[B].tempBlockNpcType > 0)
                    hs = 0;
                if(hs == 2 || hs == 4)
                    stillCollide = true;
            }

            if(!npcHasFloor(NPC[A]) || !stillCollide)
            {
                NPC[A].Special = 2;
                SkullRide(A, true);
            }
        }
    }

    if(NPC[A].Type == NPCID_STACKER && !NPC[A].Projectile)
    {
        speedVar = (float)(speedVar * 0.7);
        if(NPC[A].Special2 < 2)
        {
            speedVar = (float)(speedVar * 0.7);
            NPC[A].Special2 += 1;
        }
    }


    // POSSIBLE SUBROUTINE: applyMovement

    // Dont move
    if(NPC[A].Stuck && !NPC[A].Projectile && NPC[A].Type != NPCID_LEAF_POWER) // face closest player
    {
        NPC[A].Location.SpeedX = 0;
        if(!(NPC[A].Type == NPCID_SKELETON && NPC[A].Special > 0))
        {
            double C = 0;
            for(int B = 1; B <= numPlayers; B++)
            {
                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                {
                    double dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                    if(C == 0.0 || dist < C)
                    {
                        C = dist;
                        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                            NPC[A].Direction = -1;
                        else
                            NPC[A].Direction = 1;
                    }
                }
            }
        }
    }

    // Actual Movement (SpeedX / SpeedY application code)
    if((!NPCIsAnExit(NPC[A]) || NPC[A].Type == NPCID_STAR_EXIT || NPC[A].Type == NPCID_STAR_COLLECT) &&
        NPC[A].Type != NPCID_FIRE_POWER_S3 && NPC[A].Type != NPCID_CONVEYOR)
    {
        // ParaTroopa speed application happens in SpecialNPC, buried item can't move at all
        if(!NPCIsAParaTroopa(NPC[A]) && NPC[A].Type != NPCID_ITEM_BURIED)
        {
            NPC[A].Location.X += NPC[A].Location.SpeedX * speedVar;
            NPC[A].Location.Y += NPC[A].Location.SpeedY;
        }
    }
    else
    {
        if(!(NPC[A].Location.X == NPC[A].DefaultLocationX && NPC[A].Location.Y == NPC[A].DefaultLocationY) || NPC[A].Type == NPCID_FIRE_POWER_S3)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.99;
            NPC[A].Location.X += NPC[A].Location.SpeedX;
            NPC[A].Location.Y += NPC[A].Location.SpeedY;
            if(!NPC[A].Projectile)
                NPC[A].Location.SpeedX = 0;
        }
    }
    // End If 'end of freezenpcs


    // POSSIBLE SUBROUTINE: postMovement

    if(NPC[A].Type == NPCID_ICE_CUBE && NPC[A].Special == 3)
        NPC[A].BeltSpeed = 0;

    // projectile check
    if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PET_FIRE || NPC[A].Type == NPCID_PLR_ICEBALL || NPC[A].Type == NPCID_SWORDBEAM || NPC[A].Type == NPCID_PLR_HEAVY || NPC[A].Type == NPCID_CHAR4_HEAVY)
        NPC[A].Projectile = true;

    // make things projectiles
    if(NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG || (NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M))
    {
        if(NPC[A].Location.SpeedY > Physics.NPCGravity * 20)
            NPC[A].Projectile = true;
        else
            NPC[A].Projectile = false;
    }

    if(NPC[A].Type == NPCID_TANK_TREADS)
        NPC[A].Projectile = true;
    if(NPC[A].Type == NPCID_EARTHQUAKE_BLOCK && (NPC[A].Location.SpeedY > 2 || NPC[A].Location.SpeedY < -2))
        NPC[A].Projectile = true;

    // Special NPCs code
    SpecialNPC(A);

    // only the top half of the saw collides with blocks (gets restored after block collisions)
    if(NPC[A].Type == NPCID_SAW)
        NPC[A].Location.Height = 24;
}
