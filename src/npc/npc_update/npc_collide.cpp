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
#include "npc.h"
#include "npc_traits.h"
#include "npc/npc_queues.h"
#include "config.h"
#include "collision.h"
#include "layers.h"
#include "effect.h"
#include "eff_id.h"
#include "editor.h"

#include "main/trees.h"

void NPCCollide(int A)
{
    // first exclusion condition
    // if(!(!NPC[A].Inert && NPC[A].Type != NPCID_LIFT_SAND && NPC[A].Type != NPCID_CANNONITEM && NPC[A].Type != NPCID_SPRING &&
    //         !(NPC[A].Type == NPCID_HEAVY_THROWN && !NPC[A].Projectile) && NPC[A].Type != NPCID_COIN_SWITCH && NPC[A].Type != NPCID_GRN_BOOT &&
    //         !(NPC[A].Type == NPCID_SPIT_BOSS_BALL && !NPC[A].Projectile) &&
    //         !((NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_YEL_PLATFORM || NPC[A].Type == NPCID_BLU_PLATFORM || NPC[A].Type == NPCID_GRN_PLATFORM ||
    //    NPC[A].Type == NPCID_RED_PLATFORM || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT ||
    //    NPC[A].Type == NPCID_VPIPE_LONG || NPC[A].Type == NPCID_CANNONENEMY) && !NPC[A].Projectile) &&
    //         !(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 0) && !(NPC[A].Type == NPCID_SPIKY_BALL_S3 && !NPC[A].Projectile) &&
    //    NPC[A].Type != NPCID_TOOTHYPIPE && NPC[A].Type != NPCID_FALL_BLOCK_RED && NPC[A].Type != NPCID_VEHICLE && NPC[A].Type != NPCID_CONVEYOR &&
    //         !NPCIsYoshi(NPC[A]) && !(NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M) &&
    //         !(NPC[A].Type == NPCID_ITEM_POD && !NPC[A].Projectile) && !(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 &&
    //    NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4 && NPC[A].Projectile && NPC[A].CantHurt > 0) &&
    //         !(NPC[A]->IsAShell && !NPC[A].Projectile) &&
    //         !(NPC[A].Projectile && NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4) &&
    //    NPC[A].Type != NPCID_SPIT_GUY_BALL && !(!NPCIsToad(NPC[A]) && !NPC[A].Projectile &&
    //    NPC[A].Location.SpeedX == 0 && (NPC[A].Location.SpeedY == 0 || NPC[A].Location.SpeedY == Physics.NPCGravity))))
    // {
    //     return;
    // }

    // second exclusion condition
    // if(!(!NPC[A]->IsACoin && NPC[A].Type != NPCID_TIMER_S2 && NPC[A].Type != NPCID_FALL_BLOCK_BROWN &&
    //     NPC[A].Type != NPCID_WALL_BUG && NPC[A].Type != NPCID_WALL_SPARK && NPC[A].Type != NPCID_WALL_TURTLE && NPC[A].Type != NPCID_RED_BOOT &&
    //     NPC[A].Type != NPCID_BLU_BOOT && !(NPC[A]->IsFish && NPC[A].Special == 2) &&
    //    !NPC[A].Generator && NPC[A].Type != NPCID_PLANT_FIRE && NPC[A].Type != NPCID_FIRE_CHAIN &&
    //     NPC[A].Type != NPCID_QUAD_BALL && NPC[A].Type != NPCID_FLY_BLOCK && NPC[A].Type != NPCID_FLY_CANNON &&
    //     NPC[A].Type != NPCID_FIRE_BOSS_FIRE && NPC[A].Type != NPCID_DOOR_MAKER && NPC[A].Type != NPCID_MAGIC_DOOR))
    // {
    //     return;
    // }

    // These NPC types prevent all physics updates and make it so that NPCCollide can never be called. They are removed now.
    // NPC[A].Type == NPCID_LIFT_SAND || NPC[A].Type == NPCID_PLANT_FIRE || NPC[A].Type == NPCID_FIRE_CHAIN

    // NPC properties that block collision query
    if(NPC[A].Inert || NPC[A].Generator)
        return;

    // NPC traits that prevent collision query
    if(NPC[A]->IsACoin || NPCIsYoshi(NPC[A]))
        return;

    // NPC types that prevent collision query
    if(NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_SPRING || NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_GRN_BOOT
        || NPC[A].Type == NPCID_TOOTHYPIPE || NPC[A].Type == NPCID_FALL_BLOCK_RED || NPC[A].Type == NPCID_VEHICLE || NPC[A].Type == NPCID_CONVEYOR
        || (NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M) || NPC[A].Type == NPCID_SPIT_GUY_BALL
        || NPC[A].Type == NPCID_TIMER_S2 || NPC[A].Type == NPCID_FALL_BLOCK_BROWN
        || NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_TURTLE
        || NPC[A].Type == NPCID_RED_BOOT || NPC[A].Type == NPCID_BLU_BOOT || NPC[A].Type == NPCID_QUAD_BALL
        || NPC[A].Type == NPCID_FLY_BLOCK || NPC[A].Type == NPCID_FLY_CANNON || NPC[A].Type == NPCID_FIRE_BOSS_FIRE
        || NPC[A].Type == NPCID_DOOR_MAKER || NPC[A].Type == NPCID_MAGIC_DOOR)
    {
        return;
    }

    // NPC types that can only query collisions as projectiles
    if(!NPC[A].Projectile && (NPC[A].Type == NPCID_HEAVY_THROWN || NPC[A].Type == NPCID_SPIT_BOSS_BALL || NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_YEL_PLATFORM || NPC[A].Type == NPCID_BLU_PLATFORM || NPC[A].Type == NPCID_GRN_PLATFORM ||
       NPC[A].Type == NPCID_RED_PLATFORM || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT ||
       NPC[A].Type == NPCID_VPIPE_LONG || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_SPIKY_BALL_S3 || NPC[A].Type == NPCID_ITEM_POD || NPC[A]->IsAShell))
    {
        return;
    }

    // most NPCs cannot query collisions if static and not projectile
    if(!NPCIsToad(NPC[A]) && !NPC[A].Projectile && NPC[A].Location.SpeedX == 0 && (NPC[A].Location.SpeedY == 0 || NPC[A].Location.SpeedY == Physics.NPCGravity))
        return;

    // slide blocks only query collisions after activation
    if(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 0)
        return;

    // newly spawned hit turtles do not query collisions
    if(NPC[A].Projectile && NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4)
        return;

    // duplicate condition
    // if(NPC[A].Projectile && NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4 && NPC[A].CantHurt > 0)
    //     return;

    if(NPC[A]->IsFish && NPC[A].Special == 2)
        return;

    for(int B : treeNPCQuery(NPC[A].Location, SORTMODE_ID))
    {
        if(B == A || !NPC[B].Active || NPC[B]->IsACoin)
            continue;

        if(!CheckCollision(NPC[A].Location, NPC[B].Location))
            continue;

        // first exclusion condition
        // if(!(!(NPC[B].Type == NPCID_MINIBOSS && NPC[B].Special == 4) && !(NPCIsToad(NPC[B])) &&
        //    !(NPC[B].Type >= NPCID_PLATFORM_S3 && NPC[B].Type <= NPCID_PLATFORM_S1) && !(NPC[B].Type >= NPCID_CARRY_BLOCK_A && NPC[B].Type <= NPCID_CARRY_BLOCK_D) &&
        //    NPC[B].Type != NPCID_LIFT_SAND && NPC[B].Type != NPCID_SICK_BOSS_BALL && !NPC[B]->IsAVine &&
        //    NPC[B].Type != NPCID_PLR_ICEBALL && NPC[B].Type != NPCID_FIRE_CHAIN && NPC[B].Type != NPCID_CHAR3_HEAVY))
        // {
        //     continue;
        // }

        // second exclusion condition
        // If Not (NPC(B).Type = 133) And NPC(B).HoldingPlayer = 0 And .Killed = 0 And NPC(B).JustActivated = 0 And NPC(B).Inert = False And NPC(B).Killed = 0 Then
        // if(!(NPC[B].Type != NPCID_SPIT_GUY_BALL && !(NPCIsVeggie(NPC[B]) && NPCIsVeggie(NPC[A])) &&
        //    NPC[B].HoldingPlayer == 0 && NPC[A].Killed == 0 &&
        //    NPC[B].JustActivated == 0 && !NPC[B].Inert && NPC[B].Killed == 0))
        // {
        //     continue;
        // }

        // third exclusion condition
        // if(!(NPC[B].Type != NPCID_CANNONITEM && NPC[B].Type != NPCID_SWORDBEAM && NPC[B].Type != NPCID_TOOTHYPIPE && NPC[B].Type != NPCID_SPRING &&
        //    NPC[B].Type != NPCID_HEAVY_THROWN && NPC[B].Type != NPCID_KEY && NPC[B].Type != NPCID_COIN_SWITCH && NPC[B].Type != NPCID_GRN_BOOT &&
        //    NPC[B].Type != NPCID_VEHICLE && NPC[B].Type != NPCID_TOOTHY && NPC[B].Type != NPCID_CONVEYOR && NPC[B].Type != NPCID_METALBARREL &&
        //    NPC[B].Type != NPCID_RED_BOOT && NPC[B].Type != NPCID_BLU_BOOT && !NPC[B].Generator &&
        //    !((NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL) && NPC[B].Type == NPCID_FLIPPED_RAINBOW_SHELL) &&
        //    NPC[B].Type != NPCID_TIMER_S2 && NPC[B].Type != NPCID_FLY_BLOCK && NPC[B].Type != NPCID_FLY_CANNON && NPC[B].Type != NPCID_DOOR_MAKER &&
        //    NPC[B].Type != NPCID_MAGIC_DOOR && NPC[B].Type != NPCID_CHAR3_HEAVY && NPC[B].Type != NPCID_PLR_HEAVY && NPC[B].Type != NPCID_CHAR4_HEAVY))
        // {
        //     continue;
        // }

        // fourth exclusion condition
        // if(!(!(NPC[B].Type == NPCID_HPIPE_SHORT || NPC[B].Type == NPCID_YEL_PLATFORM || NPC[B].Type == NPCID_BLU_PLATFORM ||
        //    NPC[B].Type == NPCID_GRN_PLATFORM || NPC[B].Type == NPCID_RED_PLATFORM || NPC[B].Type == NPCID_HPIPE_LONG ||
        //    NPC[B].Type == NPCID_VPIPE_SHORT || NPC[B].Type == NPCID_VPIPE_LONG) && !(!NPC[A].Projectile &&
        //    NPC[B].Type == NPCID_SPIKY_BALL_S3) && !NPCIsYoshi(NPC[B]) && NPC[B].Type != NPCID_FALL_BLOCK_RED &&
        //    NPC[B].Type != NPCID_FALL_BLOCK_BROWN && !(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Special == 0) &&
        //    NPC[B].Type != NPCID_CONVEYOR && !(NPC[B].Type >= NPCID_TANK_TREADS && NPC[B].Type <= NPCID_SLANT_WOOD_M) &&
        //    NPC[B].Type != NPCID_STATUE_S3 && NPC[B].Type != NPCID_STATUE_FIRE && !(NPC[B].Type == NPCID_BULLET &&
        //    NPC[B].CantHurt > 0) && NPC[B].Type != NPCID_ITEM_BURIED && !(NPC[A].CantHurtPlayer == NPC[B].CantHurtPlayer &&
        //    NPC[A].CantHurtPlayer > 0) && !(NPC[B].Type == NPCID_ITEM_POD && !NPC[B].Projectile) &&
        //    NPC[B].Type != NPCID_PET_FIRE && NPC[B].Type != NPCID_PLANT_FIRE && NPC[B].Type != NPCID_QUAD_BALL &&
        //    NPC[B].Type != NPCID_FIRE_BOSS_FIRE && NPC[B].Type != NPCID_RED_VINE_TOP_S3 && NPC[B].Type != NPCID_GRN_VINE_TOP_S3 && NPC[B].Type != NPCID_GRN_VINE_TOP_S4))
        // {
        //     continue;
        // }

        // don't allow vines to be collision targets
        if(NPC[B]->IsAVine)
            continue;

        // don't allow killed NPCs to collide, and don't allow held, just-activated, or friendly NPCs to be collision targets
        if(NPC[A].Killed != 0 || NPC[B].Killed != 0 || NPC[B].HoldingPlayer != 0 || NPC[B].JustActivated != 0 || NPC[B].Inert)
            continue;

        // don't allow generators to be collision targets
        if(NPC[B].Generator)
            continue;

        // don't allow friendly fire
        if(NPC[A].CantHurtPlayer == NPC[B].CantHurtPlayer && NPC[A].CantHurtPlayer > 0)
            continue;

        // these types can't be collision targets
        if((NPC[B].Type >= NPCID_PLATFORM_S3 && NPC[B].Type <= NPCID_PLATFORM_S1) || (NPC[B].Type >= NPCID_CARRY_BLOCK_A && NPC[B].Type <= NPCID_CARRY_BLOCK_D) ||
            NPC[B].Type == NPCID_LIFT_SAND || NPC[B].Type == NPCID_SICK_BOSS_BALL || NPC[B].Type == NPCID_PLR_ICEBALL || NPC[B].Type == NPCID_FIRE_CHAIN || NPC[B].Type == NPCID_CHAR3_HEAVY ||
            NPC[B].Type == NPCID_FALL_BLOCK_RED || NPC[B].Type == NPCID_FALL_BLOCK_BROWN ||
            NPC[B].Type == NPCID_SPIT_GUY_BALL || NPC[B].Type == NPCID_CANNONITEM || NPC[B].Type == NPCID_SWORDBEAM || NPC[B].Type == NPCID_TOOTHYPIPE || NPC[B].Type == NPCID_SPRING ||
            NPC[B].Type == NPCID_HEAVY_THROWN || NPC[B].Type == NPCID_KEY || NPC[B].Type == NPCID_COIN_SWITCH || NPC[B].Type == NPCID_GRN_BOOT ||
            NPC[B].Type == NPCID_VEHICLE || NPC[B].Type == NPCID_TOOTHY || NPC[B].Type == NPCID_CONVEYOR || NPC[B].Type == NPCID_METALBARREL ||
            NPC[B].Type == NPCID_RED_BOOT || NPC[B].Type == NPCID_BLU_BOOT ||
            NPC[B].Type == NPCID_TIMER_S2 || NPC[B].Type == NPCID_FLY_BLOCK || NPC[B].Type == NPCID_FLY_CANNON || NPC[B].Type == NPCID_DOOR_MAKER ||
            NPC[B].Type == NPCID_MAGIC_DOOR || NPC[B].Type == NPCID_CHAR3_HEAVY || NPC[B].Type == NPCID_PLR_HEAVY || NPC[B].Type == NPCID_CHAR4_HEAVY ||
            NPC[B].Type == NPCID_HPIPE_SHORT || NPC[B].Type == NPCID_YEL_PLATFORM || NPC[B].Type == NPCID_BLU_PLATFORM ||
            NPC[B].Type == NPCID_GRN_PLATFORM || NPC[B].Type == NPCID_RED_PLATFORM || NPC[B].Type == NPCID_HPIPE_LONG ||
            NPC[B].Type == NPCID_VPIPE_SHORT || NPC[B].Type == NPCID_VPIPE_LONG || NPC[B].Type == NPCID_CONVEYOR || (NPC[B].Type >= NPCID_TANK_TREADS && NPC[B].Type <= NPCID_SLANT_WOOD_M) ||
            NPC[B].Type == NPCID_STATUE_S3 || NPC[B].Type == NPCID_STATUE_FIRE || NPC[B].Type == NPCID_ITEM_BURIED || NPC[B].Type == NPCID_PET_FIRE || NPC[B].Type == NPCID_PLANT_FIRE || NPC[B].Type == NPCID_QUAD_BALL ||
            NPC[B].Type == NPCID_FIRE_BOSS_FIRE || NPC[B].Type == NPCID_RED_VINE_TOP_S3 || NPC[B].Type == NPCID_GRN_VINE_TOP_S3 || NPC[B].Type == NPCID_GRN_VINE_TOP_S4 || NPCIsYoshi(NPC[B].Type) || NPCIsToad(NPC[B].Type))
        {
            continue;
        }
        // miniboss can't be collision target if in guard state
        else if(NPC[B].Type == NPCID_MINIBOSS)
        {
            if(NPC[B].Special == 4)
                continue;
        }
        // bullet can't be collision target if just player-shot
        else if(NPC[B].Type == NPCID_BULLET)
        {
            if(NPC[B].CantHurt > 0)
                continue;
        }
        // item pod can't be collision target until thrown
        else if(NPC[B].Type == NPCID_ITEM_POD)
        {
            if(!NPC[B].Projectile)
                continue;
        }
        // slide block can't be collision target until activation
        else if(NPC[B].Type == NPCID_SLIDE_BLOCK)
        {
            if(NPC[B].Special == 0)
                continue;
        }
        // spiky balls can't be collision target unless hit by thrown item
        else if(NPC[B].Type == NPCID_SPIKY_BALL_S3)
        {
            if(!NPC[A].Projectile)
                continue;
        }
        // flipped rainbow shells can't be collision target of fireball / iceball
        else if(NPC[B].Type == NPCID_FLIPPED_RAINBOW_SHELL)
        {
            if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL)
                continue;
        }
        // veggies can't be collision target of other veggies
        else if(NPCIsVeggie(NPC[B].Type))
        {
            if(NPCIsVeggie(NPC[A].Type))
                continue;
        }

        // NPC-NPC collisions must be handled a function pointer defined by NPC A, but also shouldn't be hardcoded based on NPC B's type
        //   this logic will be quite difficult (but necessary) to convert

        // NOTE: There are a number of assignments to HitSpot here, but they are never read from (in the entire UpdateNPCs routine).
        // All reads are preceded by other writes.
        // I am commenting out these assignments.

        // if(NPC[A].Type == NPCID_MAGIC_BOSS_BALL || NPC[B].Type == NPCID_MAGIC_BOSS_BALL || NPC[A].Type == NPCID_FIRE_BOSS_FIRE || NPC[B].Type == NPCID_FIRE_BOSS_FIRE)
        //     HitSpot = 0;

        if(NPC[A].Type == NPCID_ITEM_BUBBLE)
        {
            NPCHit(A, 3, B);
            // HitSpot = 0;
        }
        else if(NPC[B].Type == NPCID_ITEM_BUBBLE)
            NPCHit(B, 3, A);


        if(NPC[A].Type == NPCID_SWORDBEAM)
        {
            if(!NPC[B]->IsABonus)
                NPCHit(B, 10, NPC[A].CantHurtPlayer);
            // HitSpot = 0;
        }

        // toad code
        if(NPCIsToad(NPC[A]))
        {
            if(!(NPC[B]->WontHurt && !NPC[B].Projectile) && !NPC[B]->IsABonus &&
               NPC[B].Type != NPCID_PLR_FIREBALL && /* NPC[B].Type != NPCID_PLR_ICEBALL && !(NPC[B].Type == NPCID_BULLET && NPC[B].CantHurt > 0) &&
               NPC[B].Type != NPCID_TOOTHY && NPC[B].Type != NPCID_PLR_HEAVY && NPC[B].Type != NPCID_CHAR4_HEAVY && */ NPC[B].Type != NPCID_FLIPPED_RAINBOW_SHELL)
            {
                NPCHit(A, 3, B);
                // HitSpot = 0;
            }
        }

        // turtle enters a shell
        if((NPC[A].Type == NPCID_GRN_HIT_TURTLE_S4 || NPC[A].Type == NPCID_RED_HIT_TURTLE_S4 || NPC[A].Type == NPCID_YEL_HIT_TURTLE_S4) && /* !NPC[A].Projectile && */
           (!NPC[B].Projectile && NPC[B].Type >= NPCID_GRN_SHELL_S4 && NPC[B].Type <= NPCID_YEL_SHELL_S4))
        {
            Location_t tempLocation = NPC[A].Location;
            Location_t tempLocation2 = NPC[B].Location;
            tempLocation.Width = 8;
            tempLocation.X += 12;
            tempLocation2.Width = 8;
            tempLocation2.X += 12;

            if(CheckCollision(tempLocation, tempLocation2))
            {
                NPC[B].Type = NPCID(NPC[B].Type - 4);
                if(NPC[B].Type == NPCID_YEL_TURTLE_S4)
                    NPC[B].Type = NPCID_RAINBOW_SHELL;
                NPC[A].Killed = 9;
                NPCQueues::Killed.push_back(A);
                NPC[B].Direction = NPC[A].Direction;
                NPC[B].Frame = EditorNPCFrame(NPC[B].Type, NPC[B].Direction);
            }
        }
        // NPC is a projectile
        else if(NPC[A].Projectile && /*!(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Special == 0.0) &&*/ NPC[A].Type != NPCID_SWORDBEAM)
        {
            if(!(NPC[A].Projectile && NPC[B].Projectile && NPC[A].Type == NPCID_BULLET && NPC[B].Type == NPCID_BULLET && NPC[A].CantHurtPlayer != NPC[B].CantHurtPlayer))
            {
                if(!((NPC[A].Type == NPCID_PLR_FIREBALL && NPC[B]->IsABonus) || NPC[B].Type == NPCID_PLR_FIREBALL || NPC[B].Type == NPCID_VILLAIN_FIRE))
                {
                    // allow turtle (B) to kick shell (A) if it is facing the shell
                    if(NPC[A]->IsAShell &&
                            (NPC[B].Type == NPCID_EXT_TURTLE || NPC[B].Type == NPCID_BLU_HIT_TURTLE_S4) &&
                            (NPC[A].Direction != NPC[B].Direction || NPC[A].Special > 0) && !NPC[B].Projectile)
                    {
                        if(NPC[A].Direction == -1)
                        {
                            NPC[B].Frame = 3;
                            if(NPC[B].Type == NPCID_BLU_HIT_TURTLE_S4)
                                NPC[B].Frame = 5;
                            NPC[B].FrameCount = 0;
                        }
                        else
                        {
                            NPC[B].Frame = 0;
                            NPC[B].FrameCount = 0;
                        }

                        if(NPC[A].CantHurt < 25)
                            NPC[A].Special = 1;
                        if(NPC[A].Location.to_right_of(NPC[B].Location))
                        {
                            NPC[B].Location.X = NPC[A].Location.X - NPC[B].Location.Width - 1;
                            NPC[B].Direction = 1;
                        }
                        else
                        {
                            NPC[B].Location.X = NPC[A].Location.X + NPC[A].Location.Width + 1;
                            NPC[B].Direction = -1;
                        }

                        if(NPC[A].Location.SpeedY < NPC[B].Location.SpeedY)
                            NPC[A].Location.SpeedY = NPC[B].Location.SpeedY;
                        NPC[A].Frame = 0;
                        NPC[A].FrameCount = 0;
                        if(NPC[A].CantHurt < 25)
                            NPC[A].Special = 2;
                        NPC[B].Special = 0;
                        Location_t tempLocation = NPC[B].Location;
                        tempLocation.Y += 1;
                        tempLocation.Height -= 2;

                        for(int bCheck2 = 1; bCheck2 <= 2; bCheck2++)
                        {
                            // if(bCheck2 == 1)
                            // {
                            //     // fBlock2 = FirstBlock[(NPC[B].Location.X / 32) - 1];
                            //     // lBlock2 = LastBlock[((NPC[B].Location.X + NPC[B].Location.Width) / 32.0) + 1];
                            //     blockTileGet(NPC[B].Location, fBlock2, lBlock2);
                            // }
                            // else
                            // {
                                   // ds-sloth comment: this was a "bug",
                                   // but it never affected the result so wasn't fixed
                                   // should be numBlock - numTempBlock + 1,
                                   // this will double-count numBlock - numTempBlock.
                                   // not a problem because it is the last of the non-temp blocks
                                   // and the first of the temp blocks in the original check,
                                   // so order is the same if we exclusively count it is a non-temp block,
                                   // which is what the new code does
                            //     fBlock2 = numBlock - numTempBlock;
                            //     lBlock2 = numBlock;
                            // }

                            auto collBlockSentinel2 = (bCheck2 == 1)
                                ? treeFLBlockQuery(NPC[B].Location, SORTMODE_COMPAT)
                                : treeTempBlockQuery(NPC[B].Location, SORTMODE_LOC);

                            for(BlockRef_t block2 : collBlockSentinel2)
                            {
                                int C = block2;

                                if(!BlockIsSizable[Block[C].Type] && !BlockOnlyHitspot1[Block[C].Type] && !Block[C].Hidden && BlockSlope[Block[C].Type] == 0)
                                {
                                    if(CheckCollision(tempLocation, Block[C].Location))
                                    {
                                        if(int(NPC[A].Direction) == -1)
                                        {
                                            NPC[B].Location.X = Block[C].Location.X + Block[C].Location.Width + 0.1;
                                            NPC[A].Location.X = NPC[B].Location.X + NPC[B].Location.Width + 0.1;
                                        }
                                        else
                                        {
                                            NPC[B].Location.X = Block[C].Location.X - NPC[B].Location.Width - 0.1;
                                            NPC[A].Location.X = NPC[B].Location.X - NPC[A].Location.Width - 0.1;
                                        }
                                    }
                                }
                            }
                        }

                        treeNPCUpdate(B);
                        if(NPC[B].tempBlock > 0)
                            treeNPCSplitTempBlock(B);
                    }
                    else if(NPC[A].Type == NPCID_TANK_TREADS)
                        NPCHit(B, 8, A);
                    else
                    {
                        if(!NPC[B]->IsABonus)
                        {
                            if(NPC[A].Type == NPCID_CANNONENEMY && NPC[B].Type == NPCID_BULLET)
                                NPC[B].Projectile = true;
                            else
                            {
                                bool tempBool = false; // This whole cluster stops friendly projectiles form killing riddin shells

                                if(NPC[A]->IsAShell)
                                {
                                    for(auto C = 1; C <= numPlayers; C++)
                                    {
                                        if(Player[C].StandingOnNPC == A && NPC[B].CantHurtPlayer == C)
                                        {
                                            tempBool = true;
                                            break;
                                        }
                                    }
                                }

                                if(NPC[B]->IsAShell)
                                {
                                    for(auto C = 1; C <= numPlayers; C++)
                                    {
                                        if(Player[C].StandingOnNPC == B && NPC[A].CantHurtPlayer == C)
                                        {
                                            tempBool = true;
                                            break;
                                        }
                                    }
                                }

                                if(!(NPC[A].Type == NPCID_BULLET && NPC[A].Projectile))
                                {
                                    if(NPC[B]->IsAShell && NPC[B].Projectile)
                                    {
                                        if(!tempBool)
                                            NPCHit(A, 3, B);
                                    }
                                    else
                                    {
                                        if(!tempBool)
                                            NPCHit(A, 4, B);
                                    }
                                }

                                if(!tempBool) // end cluster
                                    NPCHit(B, 3, A);

                                if(NPC[A].Type == NPCID_BULLET)
                                {
                                    if(NPC[B].Type == NPCID_MINIBOSS)
                                    {
                                        NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                        NPCHit(A, 4, B);
                                    }
                                    else if(NPC[B].Type == NPCID_CANNONENEMY)
                                    {
                                        NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                        PlaySoundSpatial(SFX_BlockHit, NPC[A].Location);
                                        NPCHit(A, 4, A);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if(!(NPC[B].Type == NPCID_SPIT_BOSS_BALL && !NPC[B].Projectile))
        {
            int HitSpot = FindCollision(NPC[A].Location, NPC[B].Location);

            if(NPCIsToad(NPC[A]) && NPC[A].Killed > 0)
                HitSpot = 0;

            if(NPCIsAParaTroopa(NPC[A]) && NPCIsAParaTroopa(NPC[B]))
            {
                if(NPC[A].Location.to_right_of(NPC[B].Location))
                    NPC[A].Location.SpeedX += 0.05;
                else
                    NPC[A].Location.SpeedX -= 0.05;

                if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 > NPC[B].Location.Y + NPC[B].Location.Height / 2.0)
                    NPC[A].Location.SpeedY += 0.05;
                else
                    NPC[A].Location.SpeedY -= 0.05;

                HitSpot = 0;
            }

            if(!NPC[B].Projectile && !NPC[A]->NoClipping && !NPC[B]->NoClipping)
            {
                // NPC A can't be a shell in this condition, because A is not a projectile, and shells only query collisions as projectiles
                if(((NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4) && NPC[B]->IsAShell) /* || ((NPC[B].Type == NPCID_EXT_TURTLE || NPC[B].Type == NPCID_BLU_HIT_TURTLE_S4) && NPC[A]->IsAShell)*/) // Nekkid koopa kicking a shell
                {
                    // if(NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4)
                    if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
                    {
                        // If .Direction = 1 And .Location.X + .Location.Width < NPC(B).Location.X + 3 Or (.Direction = -1 And .Location.X > NPC(B).Location.X + NPC(B).Location.Width - 3) Then
                        if((NPC[A].Direction == 1  && NPC[A].Location.X + NPC[A].Location.Width < NPC[B].Location.X + 4) ||
                           (NPC[A].Direction == -1 && NPC[A].Location.X > NPC[B].Location.X + NPC[B].Location.Width - 4))
                        {
                            if(NPC[B].Location.SpeedX == 0 && NPC[B].Effect == NPCEFF_NORMAL)
                            {
                                NPC[A].Special = 10;
                                Player[numPlayers + 1].Direction = NPC[A].Direction;
                                NPC[A].Location.X += -NPC[A].Direction;
                                NPCHit(B, 1, numPlayers + 1);
                            }
                        }
                    }
                }
                else if(NPC[A].Effect == NPCEFF_MAZE && NPC[B].Effect == NPCEFF_MAZE && HitSpot != 5)
                {
                    NPC[A].onWall = true;

                    if(!NPC[B].TurnAround)
                        NPC[A].TurnAround = true;

                    if(NPC[A].Effect3 != NPC[B].Effect3)
                        NPC[B].TurnAround = true;
                }
                else if((HitSpot == 2 || HitSpot == 4) && NPC[A].Type != NPCID_SAW && NPC[B].Type != NPCID_SAW)
                {
                    NPC[A].onWall = true;
                    if(NPC[A].Direction == NPC[B].Direction)
                    {
                        if(NPC[A].Location.SpeedX * NPC[A].Direction > NPC[B].Location.SpeedX * NPC[B].Direction)
                        {
                            if(NPC[A].Type != NPCID_BULLET && NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_PLR_ICEBALL)
                                NPC[A].TurnAround = true;
                        }
                        else if(NPC[A].Location.SpeedX * NPC[A].Direction < NPC[B].Location.SpeedX * NPC[B].Direction)
                            NPC[B].TurnAround = true;
                        else
                        {
                            NPC[A].TurnAround = true;
                            NPC[B].TurnAround = true;
                        }
                    }
                    else
                    {
                        if(NPC[A].Type != NPCID_BULLET && NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_PLR_ICEBALL)
                            NPC[A].TurnAround = true;
                        NPC[B].TurnAround = true;
                    }
                }
            }
        }
    }
}

void NPCCollideHeld(int A)
{
    // these types can't collide while held
    if(NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_SPRING || NPC[A].Type == NPCID_COIN_SWITCH ||
         NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_TNT || NPC[A].Type == NPCID_BLU_BOOT || NPC[A].Type == NPCID_GRN_BOOT || NPC[A].Type == NPCID_RED_BOOT ||
         NPC[A].Type == NPCID_TOOTHYPIPE || NPC[A].Type == NPCID_BOMB || (NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D) ||
         NPC[A].Type == NPCID_KEY || NPC[A].Type == NPCID_TIMER_S2 || NPC[A].Type == NPCID_FLY_BLOCK || NPC[A].Type == NPCID_FLY_CANNON || NPC[A].Type == NPCID_CHAR4_HEAVY)
    {
        return;
    }

    for(int B : treeNPCQuery(NPC[A].Location, SORTMODE_ID))
    {
        // original exclusion condition
        // if(!(B != A && NPC[B].Active &&
        //    (NPC[B].HoldingPlayer == 0 || (BattleMode && NPC[B].HoldingPlayer != NPC[A].HoldingPlayer)) &&
        //    !NPC[B]->IsABonus &&
        //    (NPC[B].Type != NPCID_PLR_FIREBALL  || (BattleMode && NPC[B].CantHurtPlayer != NPC[A].HoldingPlayer)) &&
        //    (NPC[B].Type != NPCID_PLR_ICEBALL || (BattleMode && NPC[B].CantHurtPlayer != NPC[A].HoldingPlayer)) &&
        //     NPC[B].Type != NPCID_CANNONENEMY && NPC[B].Type != NPCID_CANNONITEM &&  NPC[B].Type != NPCID_SPRING && NPC[B].Type != NPCID_KEY &&
        //     NPC[B].Type != NPCID_COIN_SWITCH && NPC[B].Type != NPCID_TIME_SWITCH && NPC[B].Type != NPCID_TNT && NPC[B].Type != NPCID_RED_BOOT &&
        //     NPC[B].Type != NPCID_GRN_BOOT && !(NPC[B].Type == NPCID_BLU_BOOT && NPC[A].Type == NPCID_BLU_BOOT) &&
        //     NPC[B].Type != NPCID_STONE_S3 && NPC[B].Type != NPCID_STONE_S4 && NPC[B].Type != NPCID_GHOST_S3 &&
        //     NPC[B].Type != NPCID_SPIT_BOSS && !(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Special == 0.0) &&
        //     NPC[B].Type != NPCID_ITEM_BURIED && NPC[B].Type != NPCID_LIFT_SAND && NPC[B].Type != NPCID_FLIPPED_RAINBOW_SHELL &&
        //    !(NPC[B].Type == NPCID_HEAVY_THROWN && NPC[B].Projectile) && NPC[B].Type != NPCID_EARTHQUAKE_BLOCK && NPC[B].Type != NPCID_ICE_CUBE && NPC[B].Type != NPCID_CHAR3_HEAVY))
        // {
        //     continue;
        // }

        if(B == A || !NPC[B].Active)
            continue;

        // don't kill dead or friendly NPC
        if(NPC[B].Killed != 0 || NPC[B].Inert)
            continue;

        // don't kill powerups
        if(NPC[B]->IsABonus)
            continue;

        // don't kill other players' NPCs except in Battle Mode
        if(NPC[B].HoldingPlayer != 0 && (!BattleMode || NPC[B].HoldingPlayer == NPC[A].HoldingPlayer))
            continue;

        // no friendly fire
        if(NPC[A].CantHurtPlayer == NPC[B].CantHurtPlayer)
            continue;

        // don't kill NPC player is standing on
        if(Player[NPC[A].HoldingPlayer].StandingOnNPC == B)
            continue;

        // these types can't be collision targets
        if(NPC[B].Type == NPCID_CANNONENEMY || NPC[B].Type == NPCID_CANNONITEM || NPC[B].Type == NPCID_SPRING || NPC[B].Type == NPCID_KEY ||
            NPC[B].Type == NPCID_COIN_SWITCH || NPC[B].Type == NPCID_TIME_SWITCH || NPC[B].Type == NPCID_TNT || NPC[B].Type == NPCID_RED_BOOT ||
            NPC[B].Type == NPCID_GRN_BOOT || NPC[B].Type == NPCID_STONE_S3 || NPC[B].Type == NPCID_STONE_S4 || NPC[B].Type == NPCID_GHOST_S3 ||
            NPC[B].Type == NPCID_SPIT_BOSS || NPC[B].Type == NPCID_ITEM_BURIED || NPC[B].Type == NPCID_LIFT_SAND || NPC[B].Type == NPCID_FLIPPED_RAINBOW_SHELL ||
            NPC[B].Type == NPCID_EARTHQUAKE_BLOCK || NPC[B].Type == NPCID_ICE_CUBE || NPC[B].Type == NPCID_CHAR3_HEAVY)
        {
            continue;
        }
        // slide block can't be collision target until activation
        else if(NPC[B].Type == NPCID_SLIDE_BLOCK)
        {
            if(NPC[B].Special == 0)
                continue;
        }
        else if(NPC[B].Type == NPCID_HEAVY_THROWN)
        {
            if(NPC[B].Projectile)
                continue;
        }
        // probably redundant check (see above friendly fire note), but need to confirm when CantHurtPlayer and HoldingPlayer may be out of sync
        else if(NPC[B].Type == NPCID_PLR_FIREBALL || NPC[B].Type == NPCID_PLR_ICEBALL)
        {
            if(!BattleMode || NPC[B].CantHurtPlayer == NPC[A].HoldingPlayer)
                continue;
        }

#if 0
        // impossible
        if(NPC[B].Type == NPCID_BLU_BOOT && NPC[A].Type == NPCID_BLU_BOOT)
            continue;
#endif

        if(!CheckCollision(NPC[A].Location, NPC[B].Location))
            continue;

        NPCHit(B, 3, A);

        if(NPC[B].Killed > 0)
        {
            NPC[B].Location.SpeedX = Physics.NPCShellSpeed / 2 * -Player[NPC[A].HoldingPlayer].Direction;
            NPCHit(A, 5, B);
        }

        if(NPC[A].Killed > 0)
            NPC[A].Location.SpeedX = Physics.NPCShellSpeed / 2 * Player[NPC[A].HoldingPlayer].Direction;

        if(!g_config.fix_held_item_cancel || NPC[A].Killed || NPC[B].Killed)
            break;
    }
}
