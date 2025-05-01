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

#include "npc/npc_update/npc_update_priv.h"

#include "main/trees.h"

static inline bool s_use_default_movement(int A)
{
    return (NPCDefaultMovement(NPC[A]) || (NPC[A]->IsFish && NPC[A].Special != 2)) && !((NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4) && NPC[A].Special > 0);
}

void NPCMovementLogic(int A, tempf_t& speedVar)
{
    num_t Wings_SpeedX = NPC[A].Location.SpeedX;
    num_t Wings_SpeedY = NPC[A].Location.SpeedY;

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
                NPC[A].Location.SpeedX -= 0.05_n;
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX -= 0.1_n;
            }
            else if(NPC[A].Location.SpeedX < -Physics.NPCWalkingOnSpeed)
            {
                NPC[A].Location.SpeedX += 0.05_n;
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX += 0.1_n;
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
                NPC[A].Location.SpeedX -= 0.05_n;
            else if(NPC[A].Location.SpeedX < -2)
                NPC[A].Location.SpeedX += 0.05_n;
        }
        else if(!(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4 && NPC[A].Projectile))
        {
            if(NPC[A].Location.SpeedX < Physics.NPCWalkingSpeed && NPC[A].Location.SpeedX > -Physics.NPCWalkingSpeed)
            {
                if(!NPC[A].Projectile)
                    NPC[A].Location.SpeedX = Physics.NPCWalkingSpeed * NPC[A].Direction;
            }

            if(NPC[A].Location.SpeedX > Physics.NPCWalkingSpeed)
                NPC[A].Location.SpeedX -= 0.05_n;
            else if(NPC[A].Location.SpeedX < -Physics.NPCWalkingSpeed)
                NPC[A].Location.SpeedX += 0.05_n;
        }
    }
    else if(NPC[A].Type == NPCID_FLIER)
    {
        if(NPC[A].Location.SpeedX > -2 && NPC[A].Location.SpeedX < 2)
            NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
    }
    else if(NPC[A].Type == NPCID_ROCKET_FLIER)
    {
        if(NPC[A].Location.SpeedX > -2.5_n && NPC[A].Location.SpeedX < 2.5_n)
            NPC[A].Location.SpeedX = 2.5_n * NPC[A].Direction;
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
            NPC[A].Type == NPCID_ICE_POWER_S3 || NPC[A].Type == NPCID_DOOR_MAKER || NPC[A].Type == NPCID_QUAD_SPITTER || NPC[A].Type == NPCID_AQUATIC_POWER ||
            NPC[A].Type == NPCID_POLAR_POWER || NPC[A].Type == NPCID_SHELL_POWER)
    {
        if(NPC[A].Location.SpeedX > 0)
            NPC[A].Location.SpeedX -= 0.05_n;
        else if(NPC[A].Location.SpeedX < 0)
            NPC[A].Location.SpeedX += 0.05_n;

        if(NPC[A].Location.SpeedX >= -0.05_n && NPC[A].Location.SpeedX <= 0.05_n)
            NPC[A].Location.SpeedX = 0;

        if(NPC[A].Location.SpeedY >= -Physics.NPCGravity && NPC[A].Location.SpeedY <= Physics.NPCGravity)
        {
            if(NPC[A].Location.SpeedX > 0)
                NPC[A].Location.SpeedX -= 0.3_n;
            else if(NPC[A].Location.SpeedX < 0)
                NPC[A].Location.SpeedX += 0.3_n;

            if(NPC[A].Location.SpeedX >= -0.3_n && NPC[A].Location.SpeedX <= 0.3_n)
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
            NPC[A].Type == NPCID_BIG_GUY || NPC[A].Type == NPCID_INVINCIBILITY_POWER || NPC[A].Type == NPCID_CYCLONE_POWER)
    {
        if(NPC[A].Direction == 0) // Move toward the closest player
        {
            int target_plr = NPCTargetPlayer(NPC[A]);

            if(target_plr)
                NPC[A].Direction = -Player[target_plr].Direction;
        }

        if(NPC[A].Location.SpeedX < Physics.NPCMushroomSpeed && NPC[A].Location.SpeedX > -Physics.NPCMushroomSpeed)
        {
            if(!NPC[A].Projectile)
                NPC[A].Location.SpeedX = Physics.NPCMushroomSpeed * NPC[A].Direction;
        }

        if(NPC[A].Location.SpeedX > Physics.NPCMushroomSpeed)
            NPC[A].Location.SpeedX -= 0.05_n;
        else if(NPC[A].Location.SpeedX < -Physics.NPCMushroomSpeed)
            NPC[A].Location.SpeedX += 0.05_n;
    }
    else if(NPC[A].Type == NPCID_RAINBOW_SHELL)
    {
        NPC[A].Projectile = true;

        NPCFaceNearestPlayer(NPC[A]);

        NPC[A].Location.SpeedX += 0.1_n * NPC[A].Direction;

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
    else if(NPC[A].Type == NPCID_CHASER)
    {
        if(!NPC[A].Projectile && NPC[A].Special2 == 0)
        {
            NPCFaceNearestPlayer(NPC[A]);

            NPC[A].Location.SpeedX += 0.05_n * NPC[A].Direction;

            if(NPC[A].Location.SpeedX >= 3)
                NPC[A].Location.SpeedX = 3;

            if(NPC[A].Location.SpeedX <= -3)
                NPC[A].Location.SpeedX = -3;
        }
        else
        {
            if(NPC[A].Location.SpeedX > 0.1_n)
                NPC[A].Location.SpeedX -= 0.075_n;
            else if(NPC[A].Location.SpeedX < -0.1_n)
                NPC[A].Location.SpeedX += 0.075_n;

            if(NPC[A].Location.SpeedX >= -0.1_n && NPC[A].Location.SpeedX <= 0.1_n)
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
            if(NPC[A].Location.SpeedX < 3.5_n && NPC[A].Location.SpeedX > -3.5_n)
                NPC[A].Location.SpeedX += (0.1_n * NPC[A].Direction);

            if(NPC[A].Location.SpeedX > 3.5_n)
                NPC[A].Location.SpeedX -= 0.05_n;
            else if(NPC[A].Location.SpeedX < -3.5_n)
                NPC[A].Location.SpeedX += 0.05_n;

            if(NPC[A].Special == 3)
                NPC[A].Location.SpeedY = -6;
        }
        else if(NPC[A].Special == 2)
            NPC[A].Location.SpeedX += (0.2_n * NPC[A].Direction);
        else if(NPC[A].Special == 3)
            NPC[A].Location.SpeedY = -6;
        else
        {
            if(NPC[A].Location.SpeedX > 0)
                NPC[A].Location.SpeedX -= 0.05_n;
            else if(NPC[A].Location.SpeedX < 0)
                NPC[A].Location.SpeedX += 0.05_n;

            if(NPC[A].Location.SpeedX > -0.5_n && NPC[A].Location.SpeedX < 0.5_n)
                NPC[A].Location.SpeedX = 0.0001_n * NPC[A].Direction;
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
            NPCFaceNearestPlayer(NPC[A]);

            if(NPC[A].Direction == 1 && NPC[A].Location.SpeedX < 4)
                NPC[A].Location.SpeedX += 0.04_n;
            if(NPC[A].Direction == -1 && NPC[A].Location.SpeedX > -4)
                NPC[A].Location.SpeedX -= 0.04_n;
        }
    }
    else if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET)
    {
        if(NPC[A].CantHurt < 1000)
            NPC[A].Location.SpeedX = 4 * NPC[A].Direction;
    }
    else if(NPC[A].Type == NPCID_GHOST_FAST)
        NPC[A].Location.SpeedX = 2 * NPC[A].Direction;

    // yoshi
    if(NPCIsYoshi(NPC[A]))
    {
        if(NPC[A].Special == 0)
        {
            if(NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0)
            {
                if(NPC[A].Wet == 0)
                    NPC[A].Location.SpeedY = -2.1_n;
                else
                    NPC[A].Location.SpeedY = -1.1_n;
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
                    NPC[A].SpecialX = NPC[A].SpecialX * 0.99_r;
                    if(NPC[A].SpecialX > -0.1_n && NPC[A].SpecialX < 0.1_n)
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
                NPC[A].Location.SpeedY += Physics.NPCGravity * 1.5_rb;
            else if(NPC[A].Special == 3)
            {
                // peach fireball changes
                NPC[A].Location.SpeedY += Physics.NPCGravity * 0.9_r;
                if(NPC[A].Location.SpeedX > 3)
                    NPC[A].Location.SpeedX -= 0.04_n;
                else if(NPC[A].Location.SpeedX < -3)
                    NPC[A].Location.SpeedX += 0.04_n;
            }
            else if(NPC[A].Special == 4)
            {

                // toad fireball changes
                NPC[A].Location.SpeedY += Physics.NPCGravity * 1.3_r;
                if(NPC[A].Location.SpeedX < 8 && NPC[A].Location.SpeedX > 0)
                    NPC[A].Location.SpeedX += 0.03_n;
                else if(NPC[A].Location.SpeedX > -8 && NPC[A].Location.SpeedX < 0)
                    NPC[A].Location.SpeedX -= 0.03_n;
            }
            else if(NPC[A].Special == 5) // link fireballs float
            {
            }
            else
                NPC[A].Location.SpeedY += Physics.NPCGravity * 1.3_r;

        }
        else if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET)
            NPC[A].Location.SpeedY = 0;

        else if((NPC[A]->IsFish && NPC[A].Special == 2) && !NPC[A].Projectile)
        {
            if(NPC[A].Special5 == 1)
            {
                if(NPC[A].Location.Y > NPC[A].DefaultLocationY)
                    NPC[A].Location.SpeedY = -4 - (NPC[A].Location.Y - NPC[A].DefaultLocationY) / 50;
                else
                    NPC[A].Special5 = 0;
            }
            else
            {
                // If .Location.SpeedY < 2 + (.Location.Y - .DefaultLocationY) * 0.02 Then
                NPC[A].Location.SpeedY += Physics.NPCGravity * 0.4_r;
                // End If
            }
        }
        else if(NPC[A].Type != NPCID_RED_VINE_TOP_S3 && NPC[A].Type != NPCID_GRN_VINE_TOP_S3 && NPC[A].Type != NPCID_GRN_VINE_TOP_S4 &&
                !(NPC[A]->IsFish && NPC[A].Special == 2) && NPC[A].Type != NPCID_HOMING_BALL &&
                NPC[A].Type != NPCID_HOMING_BALL_GEN && NPC[A].Type != NPCID_SPIT_GUY_BALL && NPC[A].Type != NPCID_STAR_EXIT && NPC[A].Type != NPCID_STAR_COLLECT &&
                NPC[A].Type != NPCID_VILLAIN_FIRE && NPC[A].Type != NPCID_PLANT_S3 && NPC[A].Type != NPCID_FIRE_PLANT && NPC[A].Type != NPCID_PLANT_FIREBALL &&
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
                    NPC[A].Location.SpeedY += -Physics.NPCGravity / 2;
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
                        if(NPC[A].Location.SpeedY == 0.01_n)
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
                            NPC[A].Location.SpeedY += 0.05_n;
                        else
                            NPC[A].Location.SpeedY -= 0.05_n;
                    }
                    else
                    {
                        if(NPC[A].Special4 == 0)
                        {
                            NPC[A].Location.SpeedY -= 0.025_n;
                            if(NPC[A].Location.SpeedY <= -1)
                                NPC[A].Special4 = 1;
                            if(NPC[A].Special == 3 && NPC[A].Location.SpeedY <= -0.5_n)
                                NPC[A].Special4 = 1;
                        }
                        else
                        {
                            NPC[A].Location.SpeedY += 0.025_n;
                            if(NPC[A].Location.SpeedY >= 1)
                                NPC[A].Special4 = 0;
                            if(NPC[A].Special == 3 && NPC[A].Location.SpeedY >= 0.5_n)
                                NPC[A].Special4 = 0;
                        }
                    }
                }
                else if(NPC[A]->IsFish && NPC[A].Special == 1 && NPC[A].Special5 == 1)
                    NPC[A].Location.SpeedY += Physics.NPCGravity * 0.6_r;
                else if(NPC[A].Type == NPCID_FLY_BLOCK || (g_config.fix_flamethrower_gravity && NPC[A].Type == NPCID_FLY_CANNON))
                {
                    NPC[A].Location.SpeedY += Physics.NPCGravity * 0.75_rb;
                    if(NPC[A].Location.SpeedY > Physics.NPCGravity * 15)
                        NPC[A].Location.SpeedY = Physics.NPCGravity * 15;
                }
                else if(NPC[A].Type != NPCID_FIRE_DISK && NPC[A].Type != NPCID_FIRE_CHAIN)
                    NPC[A].Location.SpeedY += Physics.NPCGravity;
            }
        }


        if(NPC[A].Type == NPCID_CHAR3_HEAVY)
        {
            NPC[A].Location.SpeedY += Physics.NPCGravity * 0.8_r;
            // If .Location.SpeedY >= 5 Then .Location.SpeedY = 5
            if(NPC[A].Location.SpeedX < -0.005_n)
                NPC[A].Location.SpeedX += 0.02_n;
            else if(NPC[A].Location.SpeedX > 0.005_n)
                NPC[A].Location.SpeedX -= 0.02_n;
            else
                NPC[A].Location.SpeedX = 0;
        }
    }
    else if(NPC[A].Projectile)
    {
        NPC[A].Location.SpeedY = NPC[A].Location.SpeedY * 0.95_r;
        if(NPC[A].Location.SpeedY > -0.1_n && NPC[A].Location.SpeedY < 0.1_n)
        {
            NPC[A].Projectile = false;
            NPC[A].Location.SpeedY = 0;
        }
    }

    if(NPC[A].Location.SpeedY >= 8 && NPC[A].Type != NPCID_FIRE_DISK && NPC[A].Type != NPCID_FIRE_CHAIN)
        NPC[A].Location.SpeedY = 8;

    // POSSIBLE SUBROUTINE: preMovement

    NPCSpecial(A);

    // there was lots of speed cancel code (and some TheXTech logic for the Raft NPC) here; moved into NPCSpecial

    if(NPC[A].Type == NPCID_STACKER && !NPC[A].Projectile)
    {
        speedVar = speedVar * 7 / 10;
        if(NPC[A].Special2 < 2)
        {
            speedVar = speedVar * 7 / 10;
            NPC[A].Special2 += 1;
        }
    }

    // POSSIBLE SUBROUTINE: applyMovement

    // Dont move
    if(NPC[A].Stuck && !NPC[A].Projectile && NPC[A].Type != NPCID_LEAF_POWER) // face closest player
    {
        NPC[A].Location.SpeedX = 0;
        if(!(NPC[A].Type == NPCID_SKELETON && NPC[A].Special > 0))
            NPCFaceNearestPlayer(NPC[A]);
    }

    // Actual Movement (SpeedX / SpeedY application code)
    if(NPC[A].Wings)
    {
        // don't do anything here, the movement will be applied after SpecialNPC
    }
    else if((!NPCIsAnExit(NPC[A]) || NPC[A].Type == NPCID_STAR_EXIT || NPC[A].Type == NPCID_STAR_COLLECT) &&
        NPC[A].Type != NPCID_FIRE_POWER_S3 && NPC[A].Type != NPCID_CONVEYOR)
    {
        // ParaTroopa speed application happens in SpecialNPC, buried item can't move at all
        if(!NPCIsAParaTroopa(NPC[A]) && NPC[A].Type != NPCID_ITEM_BURIED)
        {
            NPC[A].Location.X += NPC[A].Location.SpeedX.times((num_t)speedVar);
            NPC[A].Location.Y += NPC[A].Location.SpeedY;
        }
    }
    else
    {
        if(!(NPC[A].Location.X == NPC[A].DefaultLocationX && NPC[A].Location.Y == NPC[A].DefaultLocationY) || NPC[A].Type == NPCID_FIRE_POWER_S3)
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.99_r;
            NPC[A].Location.X += NPC[A].Location.SpeedX;
            NPC[A].Location.Y += NPC[A].Location.SpeedY;
            if(!NPC[A].Projectile)
                NPC[A].Location.SpeedX = 0;
        }
    }
    // End If 'end of freezenpcs


    // POSSIBLE SUBROUTINE: postMovement

    // projectile checks moved into SpecialNPC

    // Special NPCs code
    SpecialNPC(A);

    // Wings movement code
    if(NPC[A].Wings)
    {
        // reset speed to its old value
        if(NPC[A].Wings != WING_OVERRIDE)
        {
            NPC[A].Location.SpeedX = Wings_SpeedX;
            NPC[A].Location.SpeedY = Wings_SpeedY;
        }

        // do wings movement!
        NPCMovementLogic_Wings(A, (num_t)speedVar);
    }
}

void NPCSectionWrap(NPC_t& npc)
{
    if((LevelWrap[npc.Section] || LevelVWrap[npc.Section]) && npc.Type != NPCID_HEAVY_THROWN && npc.Type != NPCID_PET_FIRE) // Level wraparound
    {
        if(LevelWrap[npc.Section])
        {
            if(npc.Location.X + npc.Location.Width < level[npc.Section].X)
                npc.Location.X = level[npc.Section].Width - 1;
            else if(npc.Location.X > level[npc.Section].Width)
                npc.Location.X = level[npc.Section].X - npc.Location.Width + 1;
        }

        if(LevelVWrap[npc.Section])
        {
            if(npc.Location.Y + npc.Location.Height < level[npc.Section].Y)
                npc.Location.Y = level[npc.Section].Height - 1;
            else if(npc.Location.Y > level[npc.Section].Height)
                npc.Location.Y = level[npc.Section].Y - npc.Location.Height + 1;
        }
    }
}

void NPCMovementLogic_Wings(int A, const num_t speedVar)
{
    WingBehaviors behavior = (NPC[A].Wings) ? NPC[A].Wings : (WingBehaviors)NPC[A].Special;

    if(NPC[A].Wings && NPC[A].Projectile && !NPC[A]->IsAShell)
    {
        NPC[A].Location.SpeedY += Physics.NPCGravity;

        if(num_t::abs(NPC[A].Location.SpeedY) < 0.5_n)
            NPC[A].Projectile = false;
    }
    else if(behavior == WING_CHASE || behavior == WING_PARA_CHASE) // chase
    {
        if(NPC[A].CantHurt > 0)
            NPC[A].CantHurt = 100;

        NPC[A].Projectile = false;

        int target_plr = 0;
        num_t min_dist = 0;
        for(int B = 1; B <= numPlayers; B++)
        {
            if(!Player[B].Dead && Player[B].Section == NPC[A].Section && Player[B].TimeToLive == 0 && NPC[A].CantHurtPlayer != B)
            {
                num_t dist = NPCPlayerTargetDist(NPC[A], Player[B]);
                if(min_dist == 0 || dist < min_dist)
                {
                    min_dist = dist;
                    target_plr = B;
                }
            }
        }

        if(NPC[A].Wings && target_plr == 0 && NPC[A].CantHurtPlayer)
            target_plr = NPC[A].CantHurtPlayer;

        if(target_plr > 0)
        {
            int D;
            if(NPC[A].Location.to_right_of(Player[target_plr].Location))
                D = -1;
            else
                D = 1;

            NPC[A].Direction = D;
            int E = 0; // X
            int F_div = -1; // Y

            if(NPC[A].Location.Y > Player[target_plr].Location.Y)
                F_div = -1;
            else if(NPC[A].Location.Y < Player[target_plr].Location.Y - 128)
                F_div = 1;

            if(NPC[A].Location.X > Player[target_plr].Location.X + Player[target_plr].Location.Width + 64)
                E = -1;
            else if(NPC[A].Location.X + NPC[A].Location.Width + 64 < Player[target_plr].Location.X)
                E = 1;

            if(NPC[A].Location.X + NPC[A].Location.Width + 150 > Player[target_plr].Location.X && NPC[A].Location.X - 150 < Player[target_plr].Location.X + Player[target_plr].Location.Width)
            {
                if(NPC[A].Location.Y > Player[target_plr].Location.Y + Player[target_plr].Location.Height)
                {

                    // If Player(C).Location.SpeedX + NPC(Player(C).StandingOnNPC).Location.SpeedX > 0 And .Location.X + .Location.Width / 2 > Player(C).Location.X + Player(C).Location.Width / 2 Then
                        // E = -D
                    // ElseIf Player(C).Location.SpeedX + NPC(Player(C).StandingOnNPC).Location.SpeedX <= 0 And .Location.X + .Location.Width / 2 < Player(C).Location.X + Player(C).Location.Width / 2 Then
                        E = -D;
                    // End If
                    if(NPC[A].Location.Y < Player[target_plr].Location.Y + Player[target_plr].Location.Height + 160)
                    {
                        if(NPC[A].Location.X + NPC[A].Location.Width + 100 > Player[target_plr].Location.X && NPC[A].Location.X - 100 < Player[target_plr].Location.X + Player[target_plr].Location.Width)
                            F_div = 5;
                    }
                }
                else
                {
                    E = D;
                    F_div = 1;
                }
            }

            if(NPC[A].Wet == 2)
            {
                NPC[A].Location.SpeedX += 0.025_n * E;
                NPC[A].Location.SpeedY += 0.025_n / F_div;
            }
            else
            {
                NPC[A].Location.SpeedX += 0.05_n * E;
                NPC[A].Location.SpeedY += 0.05_n / F_div;
            }

            if(NPC[A].Location.SpeedX > 4)
                NPC[A].Location.SpeedX = 4;
            else if(NPC[A].Location.SpeedX < -4)
                NPC[A].Location.SpeedX = -4;

            if(NPC[A].Location.SpeedY > 3)
                NPC[A].Location.SpeedY = 3;
            else if(NPC[A].Location.SpeedY < -3)
                NPC[A].Location.SpeedY = -3;
        }
    }
    else if(behavior == WING_JUMP)
    {
        NPC[A].Location.SpeedY += Physics.NPCGravity;

        if(NPC[A].Wings && NPC[A].Location.SpeedX)
        {
            if(NPC[A].Location.SpeedX < 0)
                NPC[A].Location.SpeedX = -Physics.NPCWalkingSpeed;
            else
                NPC[A].Location.SpeedX = Physics.NPCWalkingSpeed;
        }
        else
            NPC[A].Location.SpeedX = Physics.NPCWalkingSpeed * NPC[A].Direction;
    }
    else if(behavior == WING_LEFTRIGHT)
    {
        if(NPC[A].Wings)
        {
            if(NPC[A].Location.Y == NPC[A].DefaultLocationY && NPC[A].Location.SpeedY == 0)
                NPC[A].Location.SpeedY = 1;

            if(NPC[A].Location.Y > NPC[A].DefaultLocationY)
                NPC[A].Location.SpeedY -= 0.02_n;
            else if(NPC[A].Location.Y < NPC[A].DefaultLocationY)
                NPC[A].Location.SpeedY += 0.02_n;

            if(NPC[A].Location.SpeedY < -2)
                NPC[A].Location.SpeedY += Physics.NPCGravity;
            else if(num_t::abs(NPC[A].Location.SpeedY) >= 1)
                NPC[A].Location.SpeedY *= 0.9921875_rb;
        }
        else
        {
            if(NPC[A].Special3 == 0)
            {
                NPC[A].Location.SpeedY += 0.05_n;
                if(NPC[A].Location.SpeedY > 1)
                    NPC[A].Special3 = 1;
            }
            else
            {
                NPC[A].Location.SpeedY -= 0.05_n;
                if(NPC[A].Location.SpeedY < -1)
                    NPC[A].Special3 = 0;
            }
        }

        if(NPC[A].Location.X == NPC[A].DefaultLocationX && NPC[A].Location.SpeedX == 0)
            NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
        if(NPC[A].Location.X < NPC[A].DefaultLocationX - 64)
            NPC[A].Location.SpeedX += 0.02_n;
        else if(NPC[A].Location.X > NPC[A].DefaultLocationX + 64)
            NPC[A].Location.SpeedX -= 0.02_n;
        else if(NPC[A].Direction == -1)
            NPC[A].Location.SpeedX -= 0.02_n;
        else if(NPC[A].Direction == 1)
            NPC[A].Location.SpeedX += 0.02_n;

        if(NPC[A].Location.SpeedX > 2)
            NPC[A].Location.SpeedX = 2;
        if(NPC[A].Location.SpeedX < -2)
            NPC[A].Location.SpeedX = -2;
    }
    else if(behavior == WING_UPDOWN)
    {
        NPC[A].Location.SpeedX = 0;

        if(NPC[A].Location.Y == NPC[A].DefaultLocationY && NPC[A].Location.SpeedY == 0)
            NPC[A].Location.SpeedY = 2 * NPC[A].Direction;
        if(NPC[A].Location.Y < NPC[A].DefaultLocationY - 64)
            NPC[A].Location.SpeedY += 0.02_n;
        else if(NPC[A].Location.Y > NPC[A].DefaultLocationY + 64)
            NPC[A].Location.SpeedY -= 0.02_n;
        else if(NPC[A].Location.SpeedY < 0)
            NPC[A].Location.SpeedY -= 0.02_n;
        else
            NPC[A].Location.SpeedY += 0.02_n;

        if(NPC[A].Location.SpeedY > 2)
            NPC[A].Location.SpeedY = 2;
        if(NPC[A].Location.SpeedY < -2)
            NPC[A].Location.SpeedY = -2;

        NPCFaceNearestPlayer(NPC[A], true);
    }

    if(NPC[A].Stuck && !NPC[A].Projectile)
        NPC[A].Location.SpeedX = 0;

    // apply speed
    NPC[A].Location.X += NPC[A].Location.SpeedX.times(speedVar);
    NPC[A].Location.Y += NPC[A].Location.SpeedY;
    // deferring tree update to end of the NPC physics update
}
