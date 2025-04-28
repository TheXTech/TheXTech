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
#include "blocks.h"
#include "npc.h"
#include "npc_traits.h"
#include "npc/npc_queues.h"
#include "config.h"
#include "collision.h"
#include "layers.h"
#include "effect.h"
#include "eff_id.h"
#include "blk_id.h"

#include "main/trees.h"

void NPCBlockLogic(int A, num_t& tempHit, int& tempHitBlock, tempf_t& tempSpeedA, const int numTempBlock, const tempf_t speedVar)
{
    bool resetBeltSpeed = false;
    bool beltClear = false; // "stops belt movement when on a wall" (Redigit)
    bool onWall = false; // temporary variable, moved from NPC_t struct
    tempf_t beltCount = 0;
    tempf_t addBelt = 0;

    bool SlopeTurn = false;

    // int tempBlockHit[3] = {0}; // keeps track of up to two blocks hit from below
    int tempBlockHit1 = 0;
    int tempBlockHit2 = 0;
    int tempHitIsSlope = 0;

    int oldSlope = NPC[A].Slope; // previous sloped block the npc was on
    tempf_t oldBeltSpeed = (tempf_t)NPC[A].BeltSpeed;

    NPC[A].Slope = 0;
    // now, use tempf_t to do all BeltSpeed computations (accumulation, etc) in full precision in fixed-point builds
    // NPC[A].BeltSpeed = 0;
    tempf_t beltSpeed = 0;

    bool is_winged = (NPCIsAParaTroopa(NPC[A]) || (NPC[A].Wings && NPC[A].WingBehavior != WING_JUMP));

    if((!NPC[A]->NoClipping || NPC[A].Projectile || NPC[A].Wings) &&
       !(NPC[A].Type == NPCID_SPIT_BOSS_BALL && NPC[A].Projectile) && NPC[A].Type != NPCID_TOOTHY &&
        NPC[A].vehiclePlr == 0 && !(NPCIsVeggie(NPC[A]) && NPC[A].Projectile) &&
       NPC[A].Type != NPCID_HEAVY_THROWN && NPC[A].Type != NPCID_BIG_BULLET && NPC[A].Type != NPCID_PET_FIRE &&
       !(NPC[A]->IsFish && NPC[A].Special == 2) && NPC[A].Type != NPCID_VINE_BUG)
    {
        for(int bCheck = 1; bCheck <= 2; bCheck++)
        {
            // if(bCheck == 1)
            // {
            //     // fBlock = FirstBlock[(int)SDL_floor(NPC[A].Location.X / 32) - 1];
            //     // lBlock = LastBlock[(int)SDL_floor((NPC[A].Location.X + NPC[A].Location.Width) / 32.0) + 1];
            //     blockTileGet(NPC[A].Location, fBlock, lBlock);
            // }
            // else
            // {
            //     fBlock = numBlock + 1 - numTempBlock;
            //     lBlock = numBlock;
            // }
            auto collBlockSentinel = (bCheck == 1)
                ? treeFLBlockQuery(NPC[A].Location, SORTMODE_COMPAT)
                : treeTempBlockQuery(NPC[A].Location, SORTMODE_LOC);

            for(BlockRef_t block : collBlockSentinel)
            {
                int B = block;
                // If Not .Block = B And Not .tempBlock = B And Not (.Projectile = True And Block(B).noProjClipping = True) And BlockNoClipping(Block(B).Type) = False And Block(B).Hidden = False And Block(B).Hidden = False Then




                if(NPC[A].Location.X + NPC[A].Location.Width >= Block[B].Location.X)
                {
                    if(NPC[A].Location.X <= Block[B].Location.X + Block[B].Location.Width)
                    {
                        if(NPC[A].Location.Y + NPC[A].Location.Height >= Block[B].Location.Y)
                        {
                            if(NPC[A].Location.Y <= Block[B].Location.Y + Block[B].Location.Height)
                            {

                                // If CheckCollision(.Location, Block(B).Location) = True Then



                                // the coinSwitchBlockType != B check is an SMBX 1.3 bug, probably because the field was called "Block"
                                if(NPC[A].coinSwitchBlockType() != B && NPC[A].tempBlock != B &&
                                   !(NPC[A].Projectile && Block[B].tempBlockNoProjClipping()) &&
                                   !BlockNoClipping[Block[B].Type] && !Block[B].Hidden)
                                {
                                    if(Block[B].tempBlockNpcType == NPCID_TANK_TREADS && !NPC[A]->NoClipping && NPC[A].Type != NPCID_BULLET)
                                        NPCHit(A, 8);

                                    // traits of the block's NPC (if it is actually an NPC)
                                    const NPCTraits_t& blk_npc_tr = NPCTraits[Block[B].tempBlockNpcType];

                                    bool block_is_not_conveyor = (Block[B].tempBlockNpcType != NPCID_CONVEYOR) && !(Block[B].Type >= BLKID_CONVEYOR_L_START && Block[B].Type <= BLKID_CONVEYOR_R_END);

                                    int HitSpot;
                                    if(block_is_not_conveyor && (blk_npc_tr.IsABlock || blk_npc_tr.IsAHit1Block || blk_npc_tr.CanWalkOn))
                                        HitSpot = NPCFindCollision(NPC[A].Location, Block[B].Location);
                                    else
                                        HitSpot = FindCollisionBelt(NPC[A].Location, Block[B].Location, oldBeltSpeed);

                                    // block-based cancellation logic (plus some type-based cancellation logic that was placed between these in SMBX 1.3)
                                    if(Block[B].tempBlockVehiclePlr > 0 && ((!NPC[A]->StandsOnPlayer && NPC[A].Type != NPCID_PLR_FIREBALL) || NPC[A].Inert))
                                        HitSpot = 0;

                                    if(Block[B].Invis)
                                    {
                                        if(HitSpot != 3)
                                            HitSpot = 0;
                                    }

                                    if(HitSpot == 5)
                                    {
                                        if(CheckHitSpot1(NPC[A].Location, Block[B].Location))
                                            HitSpot = 1;
                                        else if(Block[B].tempBlockNpcType == NPCID_CONVEYOR) // && HitSpot == 5
                                        {
                                            if(Block[B].Location.to_right_of(NPC[A].Location))
                                                HitSpot = 4;
                                            else
                                                HitSpot = 2;
                                        }
                                        // after CheckHitSpot1 and NPCID_CONVEYOR check, before sizable check
                                        else if(NPC[A].Type == NPCID_MINIBOSS) // && HitSpot == 5
                                        {
                                            if(NPC[A].WallDeath >= 5)
                                            {
                                                NPC[A].Killed = 3;
                                                NPCQueues::Killed.push_back(A);
                                            }
                                            else
                                                HitSpot = 3;
                                        }
                                    }

                                    // after CheckHitSpot1, before sizable check
                                    if(NPC[A].Type == NPCID_VILLAIN_S3)
                                    {
                                        if(HitSpot != 1 && NPC[A].Special > 0)
                                        {
                                            if(Block[B].Location.X < level[NPC[A].Section].X + 48 || Block[B].Location.X > level[NPC[A].Section].Width - 80)
                                                NPC[A].Special = 0;
                                            else
                                            {
                                                HitSpot = 0;
                                                resetBeltSpeed = true;
                                            }
                                        }
                                        else if(HitSpot == 3)
                                        {
                                            if(NPC[A].Special4 == 3)
                                            {
                                                NPC[A].Frame = 10;
                                                NPC[A].Special3 = 21;
                                                NPC[A].Special = 1;
                                                NPC[A].Location.SpeedX = 0;
                                            }
                                        }
                                    }

                                    if(BlockIsSizable[Block[B].Type] || BlockOnlyHitspot1[Block[B].Type])
                                    {
                                        if(HitSpot != 1 || (NPCIsAParaTroopa(NPC[A]) && NPC[A].Special != WING_JUMP) || (NPC[A].Wings && NPC[A].WingBehavior != WING_JUMP))
                                            HitSpot = 0;
                                    }

                                    // unified type-based cancellation / collision logic
                                    if(NPC[A]->IsFish)
                                    {
                                        // fish ignore block collisions while out of water (eg, jumping)
                                        if(NPC[A].Wet == 0)
                                        {
                                            if(NPC[A].WallDeath >= 9)
                                                HitSpot = 0;
                                        }

                                        // leaping fish always ignore non-ceiling collisions
                                        if(NPC[A].Special == 2 && HitSpot != 3)
                                            HitSpot = 0;
                                    }
                                    else if(NPC[A].Type == NPCID_PLR_HEAVY || NPC[A].Type == NPCID_SWORDBEAM || NPC[A].Type == NPCID_CHAR4_HEAVY)
                                    {
                                        if(Block[B].Type == 457)
                                            SafelyKillBlock(B);

                                        HitSpot = 0;
                                    }
                                    else if(NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG)
                                    {
                                        if(Block[B].tempBlockVehiclePlr > 0 || Block[B].tempBlockNpcType == NPCID_VEHICLE)
                                        {
                                            HitSpot = 0;
                                            NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                        }
                                    }
                                    else if(NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M)
                                    {
                                        if(HitSpot != 1)
                                            HitSpot = 0;
                                    }
                                    else if(NPC[A].Type == NPCID_SPIKY_BALL_S3)
                                    {
                                        if(Block[B].tempBlockNpcType == NPCID_CANNONITEM || Block[B].tempBlockNpcType == NPCID_TOOTHYPIPE) // spiney eggs don't walk on special items
                                            HitSpot = 0;
                                    }
                                    else if(NPC[A].Type == NPCID_RAFT) // Skull raft
                                    {
                                        if(Block[B].tempBlockNpcType > 0)
                                            HitSpot = 0;

                                        if(g_config.fix_skull_raft) // reached a solid wall
                                        {
                                            auto bt = Block[B].Type;
                                            if(Block[B].tempBlockNpcType <= 0 && NPC[A].Special == 1 &&
                                              (HitSpot == COLLISION_LEFT || HitSpot == COLLISION_RIGHT) &&
                                               BlockSlope[bt] == SLOPE_FLOOR && BlockSlope2[bt] == SLOPE_CEILING)
                                            {
                                                if(npcHasFloor(NPC[A]))
                                                {
                                                    SkullRide(A, false, &Block[B].Location);
                                                    NPC[A].Special = 3; // 3 - watcher, 2 - waiter
                                                }
                                            }
                                        }
                                    }
                                    else if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL || NPC[A].Type == NPCID_PET_FIRE)
                                    {
                                        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL)
                                        {
                                            if(Block[B].Type == 626 && NPC[A].Special == 1)
                                                HitSpot = 0;
                                            else if(Block[B].Type == 627 && NPC[A].Special == 2)
                                                HitSpot = 0;
                                            else if(Block[B].Type == 628 && NPC[A].Special == 3)
                                                HitSpot = 0;
                                            else if(Block[B].Type == 629 && NPC[A].Special == 4)
                                                HitSpot = 0;

                                            if(NPCTraits[Block[B].tempBlockNpcType].IsABonus)
                                                HitSpot = 0;

                                            if(NPC[A].Type == NPCID_PLR_FIREBALL && Block[B].tempBlockNpcType == NPCID_ICE_CUBE)
                                                HitSpot = 0;
                                        }

                                        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PET_FIRE)
                                        {
                                            if(Block[B].Type == 621 || Block[B].Type == 620)
                                            {
                                                NPCHit(A, 3, A);
                                                if(Block[B].Type == 621)
                                                    Block[B].Type = 109;
                                                else
                                                {
                                                    Block[B].Layer = LAYER_DESTROYED_BLOCKS;
                                                    Block[B].Hidden = true;
                                                    syncLayersTrees_Block(B);
                                                    numNPCs++;
                                                    NPC[numNPCs] = NPC_t();
                                                    NPC[numNPCs].Location.Width = 28;
                                                    NPC[numNPCs].Location.Height = 32;
                                                    NPC[numNPCs].Type = NPCID_COIN_S3;
                                                    NPC[numNPCs].Location.Y = Block[B].Location.Y;
                                                    NPC[numNPCs].Location.X = Block[B].Location.X + 2;
                                                    NPC[numNPCs].Active = true;
                                                    NPC[numNPCs].DefaultType = NPC[numNPCs].Type;
                                                    NPC[numNPCs].DefaultLocationX = NPC[numNPCs].Location.X;
                                                    NPC[numNPCs].DefaultLocationY = NPC[numNPCs].Location.Y;
                                                    NPC[numNPCs].TimeLeft = 100;
                                                    syncLayers_NPC(numNPCs);
                                                    CheckSectionNPC(numNPCs);
                                                }
                                            }
                                        }
                                    }
                                    else if(NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4)
                                    {
                                        if(HitSpot != 1)
                                            HitSpot = 0;
                                    }
                                    else if(NPC[A].Type == NPCID_SPIT_GUY_BALL)
                                    {
                                        if(HitSpot > 0)
                                        {
                                            NPC[A].Killed = 4;
                                            NPCQueues::Killed.push_back(A);
                                        }
                                    }
                                    else if(NPC[A].Type == NPCID_BOMB)
                                    {
                                        if(NPC[A].Projectile && HitSpot != 0)
                                            NPC[A].Special = 1000;
                                    }
                                    // vine makers
                                    else if(NPC[A].Type == NPCID_RED_VINE_TOP_S3 || NPC[A].Type == NPCID_GRN_VINE_TOP_S3 || NPC[A].Type == NPCID_GRN_VINE_TOP_S4)
                                    {
                                        if(HitSpot == 3)
                                            NPC[A].Special = 1;
                                    }
                                    else if(NPC[A].Type == NPCID_GOALTAPE)
                                    {
                                        if(Block[B].tempBlockNpcType > 0)
                                            HitSpot = 0;
                                    }
                                    else if(NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_TURTLE)
                                    {
                                        NPC[A].Special5 = 0;

                                        if(HitSpot == 1)
                                        {
                                            if(NPC[A].Special == 4 && NPC[A].Location.X + 0.99_n == Block[B].Location.X + Block[B].Location.Width)
                                                HitSpot = 0;

                                            if(NPC[A].Special == 2 && NPC[A].Location.X + NPC[A].Location.Width - 0.99_n == Block[B].Location.X)
                                                HitSpot = 0;
                                        }

                                        if(BlockIsSizable[Block[B].Type] || BlockOnlyHitspot1[Block[B].Type])
                                            HitSpot = 0;

                                        if(BlockSlope2[Block[B].Type] != 0 && HitSpot == 3)
                                        {
                                            if(NPC[A].Special == 4 && NPC[A].Special2 == -1)
                                            {
                                                if(NPC[A].Location.X - 0.01_n == Block[B].Location.X)
                                                {
                                                    NPC[A].Special = 3;
                                                    NPC[A].Special2 = 1;
                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedY;
                                                }
                                            }
                                        }

                                        if(BlockSlope2[Block[B].Type] != 0 && HitSpot == 1)
                                        {
                                            if(NPC[A].Special == 4)
                                                HitSpot = 2;
                                            if(NPC[A].Special == 2)
                                                HitSpot = 4;
                                        }

                                        if(NPC[A].Special == 3)
                                        {
                                            // if(BlockSlope2[Block[B].Type] != 0)
                                            // {
                                            //     if(HitSpot == 2 || HitSpot == 4)
                                            //         HitSpot = 0;
                                            // }
                                            // else
                                            if(HitSpot == 2 || HitSpot == 4)
                                                HitSpot = 0;
                                        }
                                    }
                                    else if(NPC[A].Type == NPCID_SAW)
                                    {
                                        if(Block[B].tempBlockNpcType > 0)
                                            HitSpot = 0;
                                    }

                                    // shadow mode cancellation
                                    if(NPC[A].Shadow && HitSpot != 1 && !(Block[B].Special > 0 && NPC[A].Projectile))
                                        HitSpot = 0;

                                    if(BlockSlope2[Block[B].Type] != 0 && HitSpot > 0 && ((NPC[A].Location.Y > Block[B].Location.Y) || ((NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_TURTLE) && NPC[A].Special == 3)))
                                    {
                                        // the modifications to Special and Special2 here are invalid and could affect any NPC. They were meant to affect wall climbers.
                                        num_t NPC_A_Special = NPC[A].Special;
                                        num_t NPC_A_Special2 = NPC[A].Special2;

                                        char Special_from = 0;
                                        char Special2_from = 0;

                                        if(NPC[A].Type == NPCID_YEL_PLATFORM || NPC[A].Type == NPCID_BLU_PLATFORM || NPC[A].Type == NPCID_GRN_PLATFORM || NPC[A].Type == NPCID_RED_PLATFORM || NPC[A].Type == NPCID_PLATFORM_S3 || NPC[A].Type == NPCID_SAW)
                                        {
                                            Special_from = 'Y';
                                            Special2_from = 'X';
                                            NPC_A_Special = NPC[A].SpecialY;
                                            NPC_A_Special2 = NPC[A].SpecialX;
                                        }
                                        else if(NPC[A].Type == NPCID_HOMING_BALL_GEN)
                                        {
                                            Special_from = 'Y';
                                            NPC_A_Special = NPC[A].SpecialY;
                                        }
                                        else if(NPC[A].Type == NPCID_VILLAIN_S3 || NPC[A].Type == NPCID_MINIBOSS || NPC[A].Type == NPCID_GOALTAPE || NPC[A].Type == NPCID_BAT)
                                        {
                                            Special2_from = 'Y';
                                            NPC_A_Special2 = NPC[A].SpecialY;
                                        }
                                        else if(NPC[A].Type == NPCID_SPIT_BOSS)
                                        {
                                            Special_from = '5';
                                            NPC_A_Special = NPC[A].Special5;
                                        }

                                        if(HitSpot == 5)
                                        {
                                            if(NPC_A_Special == 2 && NPC_A_Special2 == 1)
                                            {
                                                NPC_A_Special2 = 1;
                                                NPC_A_Special = 3;
                                            }

                                            if(NPC_A_Special == 4 && NPC_A_Special2 == 1)
                                            {
                                                NPC_A_Special2 = -1;
                                                NPC_A_Special = 3;
                                            }
                                        }

                                        HitSpot = 0;
                                        num_t PlrMid;
                                        if(BlockSlope2[Block[B].Type] == 1)
                                            PlrMid = NPC[A].Location.X + NPC[A].Location.Width;
                                        else
                                            PlrMid = NPC[A].Location.X;

                                        num_t Slope = (PlrMid - Block[B].Location.X) / (int_ok)Block[B].Location.Width;
                                        if(BlockSlope2[Block[B].Type] > 0)
                                            Slope = 1 - Slope;
                                        if(Slope < 0)
                                            Slope = 0;
                                        if(Slope > 1)
                                            Slope = 1;

                                        if(NPC[A].Location.Y < Block[B].Location.Y + Block[B].Location.Height - ((int_ok)Block[B].Location.Height * Slope) - 0.1_n)
                                        {
                                            if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_PLR_ICEBALL)
                                                NPCHit(A, 3, A);

                                            if(NPC[A].Type == NPCID_ICE_CUBE)
                                            {
                                                if(NPC[A].Location.SpeedY < -2)
                                                    NPCHit(A, 3, A);
                                            }

                                            if(num_t::fEqual_d(NPC[A].Location.SpeedY, Physics.NPCGravity) || NPC[A].Slope > 0 || oldSlope > 0)
                                            {
                                                if((NPC_A_Special == 2 || NPC_A_Special == 4) && NPC_A_Special2 == -1)
                                                {
                                                    if(NPC_A_Special == 4)
                                                        NPC_A_Special2 = 1;
                                                    if(NPC_A_Special == 2)
                                                        NPC_A_Special2 = -1;
                                                    NPC_A_Special = 3;
                                                }

                                                PlrMid = NPC[A].Location.Y;
                                                Slope = (PlrMid - Block[B].Location.Y) / (int_ok)Block[B].Location.Height;
                                                if(Slope < 0)
                                                    Slope = 0;
                                                if(Slope > 1)
                                                    Slope = 1;
                                                if(BlockSlope2[Block[B].Type] < 0)
                                                    NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width - ((int_ok)Block[B].Location.Width * Slope);
                                                else
                                                    NPC[A].Location.X = Block[B].Location.X + ((int_ok)Block[B].Location.Width * Slope) - NPC[A].Location.Width;
                                                SlopeTurn = true;
                                                if(NPC[A].Location.SpeedX < 0)
                                                    HitSpot = 2;
                                                else
                                                    HitSpot = 4;
                                            }
                                            else
                                            {
                                                NPC[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height - ((int_ok)Block[B].Location.Height * Slope);
                                                if(NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_TURTLE)
                                                {
                                                    NPC[A].Location.Y += 1;
                                                    tempBlockHit1 = 0;
                                                    tempBlockHit2 = 0;
                                                }

                                                if(NPC[A].Location.SpeedY < -0.01_n)
                                                    NPC[A].Location.SpeedY = -0.01_n + Block[B].Location.SpeedY;

                                                if(is_winged)
                                                    NPC[A].Location.SpeedY += 2;
                                            }
                                        }

                                        // could make compat flag here and refuse to clobber these values for cases other than wall climbers
                                        if(Special2_from == 'X')
                                            NPC[A].SpecialX = NPC_A_Special2;
                                        else if(Special2_from == 'Y')
                                            NPC[A].SpecialY = NPC_A_Special2;
                                        else
                                            NPC[A].Special2 = (vbint_t)NPC_A_Special2;

                                        if(Special_from == 'X')
                                            NPC[A].SpecialX = NPC_A_Special;
                                        else if(Special_from == 'Y')
                                            NPC[A].SpecialY = NPC_A_Special;
                                        else if(Special_from == '5')
                                            NPC[A].Special5 = (vbint_t)NPC_A_Special;
                                        else
                                            NPC[A].Special = (vbint_t)NPC_A_Special;
                                    }


                                    if(BlockSlope[Block[B].Type] != 0 && HitSpot > 0)
                                    {
                                        HitSpot = 0;
                                        if(NPC[A].Location.Y + NPC[A].Location.Height <= Block[B].Location.Y + Block[B].Location.Height + NPC[A].Location.SpeedY + 4)
                                        {
                                            if(NPC[A].Location.X < Block[B].Location.X + Block[B].Location.Width && NPC[A].Location.X + NPC[A].Location.Width > Block[B].Location.X)
                                            {
                                                num_t PlrMid;
                                                if(BlockSlope[Block[B].Type] == 1)
                                                    PlrMid = NPC[A].Location.X;
                                                else
                                                    PlrMid = NPC[A].Location.X + NPC[A].Location.Width;

                                                num_t Slope = (PlrMid - Block[B].Location.X) / (int_ok)Block[B].Location.Width;

                                                if(BlockSlope[Block[B].Type] < 0)
                                                    Slope = 1 - Slope;
                                                if(Slope < 0)
                                                    Slope = 0;
                                                if(Slope > 100)
                                                    Slope = 100;

                                                if(tempHitBlock > 0) // VERIFY ME
                                                {
                                                    if(!BlockIsSizable[Block[tempHitBlock].Type])
                                                    {
                                                        if(Block[tempHitBlock].Location.Y != Block[B].Location.Y)
                                                        {
                                                            tempHitBlock = 0;
                                                            tempHit = 0;
                                                        }
                                                    }
                                                }

                                                if(NPC[A].Location.Y >= Block[B].Location.Y + ((int_ok)Block[B].Location.Height * Slope) - NPC[A].Location.Height - 0.1_n)
                                                {
                                                    if(NPC[A].Type == NPCID_EARTHQUAKE_BLOCK && NPC[A].Location.SpeedY > 2)
                                                        NPCHit(A, 4, A);

                                                    if((NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_TURTLE) && NPC[A].Special == 3)
                                                    {
                                                        NPC[A].Special = 1;
                                                        NPC[A].Special2 = -NPC[A].Special2;
                                                    }

                                                    if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_SPIT_BOSS_BALL) // Bullet bills crash on slopes
                                                    {
                                                        NPC[A].Slope = 1;
                                                        if(NPC[A].Location.SpeedX < 0)
                                                            HitSpot = 2;
                                                        else
                                                            HitSpot = 4;
                                                    }
                                                    else
                                                    {
                                                        NPC[A].Location.Y = Block[B].Location.Y + ((int_ok)Block[B].Location.Height * Slope) - NPC[A].Location.Height - 0.1_n;

                                                        if(NPC[A]->IsFish)
                                                            NPC[A].TurnAround = true;

                                                        NPC[A].Slope = B;
                                                        HitSpot = 1;

                                                        // Fireballs dont go up steep slopes
                                                        if(Block[B].Location.Height >= Block[B].Location.Width && ((BlockSlope[Block[B].Type] == -1 && NPC[A].Location.SpeedX > 0) || (BlockSlope[Block[B].Type] == 1 && NPC[A].Location.SpeedX < 0)))
                                                        {
                                                            if((NPC[A].Type == NPCID_PLR_FIREBALL && NPC[A].Special != 2 && NPC[A].Special != 3) || (NPC[A].Type == NPCID_PLR_ICEBALL && NPC[A].Special == 5))
                                                            {
                                                                if(NPC[A].Location.SpeedX < 0)
                                                                    HitSpot = 2;
                                                                else
                                                                    HitSpot = 4;
                                                            }
                                                        }

                                                        if(NPC[A]->IsAShell || (NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1) || NPC[A].Type == NPCID_ICE_CUBE)
                                                        {
                                                            if(NPC[A].Location.SpeedY > NPC[A].Location.SpeedX * (int_ok)Block[B].Location.Height / (int_ok)Block[B].Location.Width * BlockSlope[Block[B].Type])
                                                            {
                                                                NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (int_ok)Block[B].Location.Height / (int_ok)Block[B].Location.Width * BlockSlope[Block[B].Type];
                                                                HitSpot = 0;
                                                                if(NPC[A].Location.SpeedY > 0)
                                                                    NPC[A].Location.SpeedY = 0;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }



                                    if(NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_TURTLE)
                                    {
                                        if(NPC[A].Special == 3 && (HitSpot == 2 || HitSpot == 4))
                                        {
                                            if(Block[B].Location.Y + Block[B].Location.Height <= NPC[A].Location.Y + 1)
                                                HitSpot = 3;
                                        }

                                        if(Block[B].tempBlockNpcType > 0)
                                            HitSpot = 0;
                                    }

                                    if(BlockKills[Block[B].Type] && (HitSpot > 0 || NPC[A].Slope == B))
                                        NPCHit(A, 6, B);

                                    if(NPC[A].Type == NPCID_ITEM_POD && HitSpot == 1)
                                    {
                                        if((NPC[A].Location.SpeedY > 2 && HitSpot == 1) || (NPC[A].Location.SpeedY < -2 && HitSpot == 3) || (NPC[A].Location.SpeedX > 2 && HitSpot == 4) || (NPC[A].Location.SpeedX < -2 && HitSpot == 2))
                                            NPC[A].Special2 = 1;
                                    }

                                    if(HitSpot == 5)
                                    {
                                        if(NPC[A].Slope > 0 && Block[B].Location.Y + Block[B].Location.Height < NPC[A].Location.Y + 4)
                                        {
                                            if(Block[B].Location.X + Block[B].Location.Width < NPC[A].Location.X + 4 || Block[B].Location.X > NPC[A].Location.X + NPC[A].Location.Width - 4)
                                                HitSpot = 0;
                                        }
                                    }

                                    // beech koopa kicking an ice block
                                    if(((g_config.fix_npc55_kick_ice_blocks && NPC[A].Type == NPCID_EXT_TURTLE) || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4) && Block[B].tempBlockNpcType == NPCID_SLIDE_BLOCK)
                                    {
                                        if(HitSpot == 2 || HitSpot == 4)
                                        {
                                            if(NPC[A].Location.SpeedY == Physics.NPCGravity ||
                                               NPC[A].Location.SpeedY == 0 || NPC[A].Slope > 0 ||
                                               (oldSlope > 0 && !NPC[Block[B].tempBlockNpcIdx].Projectile))
                                            {
                                                NPC[Block[B].tempBlockNpcIdx].Special = 1;
                                                NPC[A].Special = 10;
                                                Player[numPlayers + 1].Direction = NPC[A].Direction;
                                                NPC[A].Location.X += -NPC[A].Direction;
                                                NPCHit(Block[B].tempBlockNpcIdx, 1, numPlayers + 1);
                                                HitSpot = 0;
                                            }
                                        }
                                    }

                                    if(Block[B].tempBlockNpcType == NPCID_BOSS_CASE || Block[B].tempBlockNpcType == NPCID_BOSS_FRAGILE)
                                    {
                                        if(NPC[A].Projectile)
                                        {
                                            NPCHit(Block[B].tempBlockNpcIdx, 3, A);
                                            NPCHit(A, 4, Block[B].tempBlockNpcIdx);
                                        }
                                    }



                                    if((NPC[A].Type == NPCID_ICE_BLOCK || NPC[A].Type == NPCID_ICE_CUBE) && (HitSpot == 2 || HitSpot == 4 || HitSpot == 5))
                                    {
                                        if(Block[B].tempBlockNpcType == NPCID_ICE_CUBE)
                                        {
                                            NPCHit(Block[B].tempBlockNpcIdx, 3, Block[B].tempBlockNpcIdx);
                                            NPC[Block[B].tempBlockNpcIdx].Location.SpeedX = -NPC[A].Location.SpeedX;
                                            NPC[A].Multiplier += 1;
                                        }

                                        NPCHit(A, 3, A);
                                    }

                                    if(NPC[A].Type == NPCID_ICE_CUBE && (HitSpot == 1 || HitSpot == 3 || HitSpot == 5))
                                    {
                                        if(NPC[A].Location.SpeedX > -Physics.NPCShellSpeed * 0.8_r && NPC[A].Location.SpeedX < Physics.NPCShellSpeed * 0.8_r)
                                        {
                                            if(NPC[A].Location.SpeedY > 5 || NPC[A].Location.SpeedY < -2)
                                                NPCHit(A, 3, A);
                                        }
                                    }

                                    // dead code: this procedure isn't called for coins with NPC[A].Special == 0
                                    // if(NPC[A]->IsACoin && NPC[A].Special == 0 && HitSpot > 0)
                                    //     NPCHit(A, 3, A);

                                    // NPC pinch logic
                                    if(Block[B].Location.SpeedX != 0 && (HitSpot == 2 || HitSpot == 4))
                                        NPC[A].Pinched.Moving = 2;

                                    if(Block[B].Location.SpeedY != 0 && (HitSpot == 1 || HitSpot == 3))
                                        NPC[A].Pinched.Moving = 2;

                                    if(NPC[A].TimeLeft > 1)
                                    {
                                        if(HitSpot == 1)
                                            NPC[A].Pinched.Bottom1 = 2;
                                        else if(HitSpot == 2)
                                            NPC[A].Pinched.Left2 = 2;
                                        else if(HitSpot == 3)
                                            NPC[A].Pinched.Top3 = 2;
                                        else if(HitSpot == 4)
                                            NPC[A].Pinched.Right4 = 2;
                                        else if(HitSpot == 5)
                                        {
                                            num_t dx = Block[B].Location.minus_center_x(NPC[A].Location);
                                            num_t dy = Block[B].Location.minus_center_y(NPC[A].Location);

                                            int D = 0;

                                            if(dx > 0)
                                                D = 2;
                                            else
                                            {
                                                dx = -dx;
                                                D = 4;
                                            }

                                            if(dy > 0)
                                            {
                                                if(dx < dy)
                                                    D = 1;
                                            }
                                            else
                                            {
                                                if(dx < -dy)
                                                    D = 3;
                                            }

                                            if(D == 1)
                                                NPC[A].Pinched.Bottom1 = 2;
                                            if(D == 2)
                                                NPC[A].Pinched.Left2 = 2;
                                            if(D == 3)
                                                NPC[A].Pinched.Top3 = 2;
                                            if(D == 4)
                                                NPC[A].Pinched.Right4 = 2;

                                            if(Block[B].Location.SpeedX != 0 && (D == 2 || D == 4))
                                                NPC[A].Pinched.Moving = 2;
                                            if(Block[B].Location.SpeedY != 0 && (D == 1 || D == 3))
                                                NPC[A].Pinched.Moving = 2;

                                            // If Not (.Location.Y + .Location.Height - .Location.SpeedY <= Block(B).Location.Y - Block(B).Location.SpeedY) Then .Pinched1 = 2
                                            // If Not (.Location.Y - .Location.SpeedY >= Block(B).Location.Y + Block(B).Location.Height - Block(B).Location.SpeedY) Then .Pinched3 = 2
                                            // If Not (.Location.X + .Location.Width - .Location.SpeedX <= Block(B).Location.X - Block(B).Location.SpeedX) Then .Pinched2 = 2
                                            // If Not (.Location.X - .Location.SpeedX >= Block(B).Location.X + Block(B).Location.Width - Block(B).Location.SpeedX) Then .Pinched4 = 2
                                        }

                                        if(NPC[A].Pinched.Moving > 0)
                                        {
                                            if((NPC[A].Pinched.Bottom1 > 0 && NPC[A].Pinched.Top3 > 0) || (NPC[A].Pinched.Left2 > 0 && NPC[A].Pinched.Right4 > 0))
                                            {
                                                if(HitSpot > 1)
                                                    HitSpot = 0;

                                                // for coins, Damage was overloaded to store coinSwitchBlockType. don't clobber it here!
                                                if(!NPCTraits[NPC[A].DefaultType].IsACoin)
                                                {
                                                    NPC[A].Damage += 10000;

                                                    // NEW bounds check
                                                    // this reduces the risk of a signed integer overflow, and SMBX 1.3 includes no comparisons to values above 20000
                                                    // SMBX 1.3 uses a float for Damage, which effectively saturates at such high values
                                                    if(NPC[A].Damage > 16000)
                                                        NPC[A].Damage = 16000;
                                                }

                                                NPC[A].Immune = 0;
                                                NPC[0].Multiplier = 0;
                                                NPCHit(A, 3, 0);
                                            }
                                        }
                                    }

                                    if((NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL) && NPC[A].Special == 5 && HitSpot > 0)
                                        NPCHit(A, 3, A);

                                    if(NPC[A].Type == NPCID_PLR_ICEBALL && HitSpot > 1)
                                        NPCHit(A, 3, A);

                                    if(NPC[A].Type == NPCID_ITEM_BUBBLE && !BlockIsSizable[Block[B].Type])
                                        NPCHit(A, 3, A);

                                    if(NPC[A].Type == NPCID_CHAR3_HEAVY && HitSpot > 0)
                                        NPCHit(A, 3, A);

                                    // safe to leave uninitialized, they're only read if g_config.fix_npc_downward_clip is set
                                    num_t tempHitOld;
                                    int tempHitOldBlock;

                                    if(g_config.fix_npc_downward_clip)
                                    {
                                        tempHitOld = tempHit;
                                        tempHitOldBlock = tempHitBlock;
                                    }

                                    // hitspot 1
                                    if(HitSpot == 1) // Hitspot 1
                                    {
                                        tempSpeedA = (tempf_t)Block[B].Location.SpeedY;
                                        if(tempSpeedA < 0)
                                            tempSpeedA = 0;

                                        if(Block[B].tempBlockNpcIdx > 0 && NPC[Block[B].tempBlockNpcIdx].Type != NPCID_CONVEYOR
                                            && NPC[Block[B].tempBlockNpcIdx].Type != NPCID_YEL_PLATFORM && NPC[Block[B].tempBlockNpcIdx].Type != NPCID_BLU_PLATFORM && NPC[Block[B].tempBlockNpcIdx].Type != NPCID_GRN_PLATFORM && NPC[Block[B].tempBlockNpcIdx].Type != NPCID_RED_PLATFORM)
                                        {
                                            if(NPC[Block[B].tempBlockNpcIdx].TimeLeft < NPC[A].TimeLeft - 1)
                                                NPC[Block[B].tempBlockNpcIdx].TimeLeft = NPC[A].TimeLeft - 1;
                                            else if(NPC[Block[B].tempBlockNpcIdx].TimeLeft - 1 > NPC[A].TimeLeft)
                                                NPC[A].TimeLeft = NPC[Block[B].tempBlockNpcIdx].TimeLeft - 1;
                                        }

                                        // type-based logic for floor collisions
                                        if(NPC[A].Type == NPCID_STACKER)
                                        {
                                            // If a Pokey head stands on a top of another Pokey segment
                                            // FIXME: need a compat guard for second condition
                                            if(Block[B].tempBlockNpcType == NPCID_STACKER
                                                && NPC[Block[B].tempBlockNpcIdx].Type == NPCID_STACKER) // Make sure Pokey didn't transformed into ice cube or anything also
                                            {
                                                NPC[Block[B].tempBlockNpcIdx].Special = -3;
                                                NPC[A].Special2 = 0;
                                            }
                                        }
                                        else if(NPC[A].Type == NPCID_SPIKY_BALL_S4)
                                            NPC[A].Special = 1;
                                        else if(NPC[A].Type == NPCID_GOALTAPE || NPC[A].Type == NPCID_FLAG_EXIT)
                                            NPC[A].Special = 1;
                                        else if(NPC[A].Type == NPCID_SQUID_S3 || NPC[A].Type == NPCID_SQUID_S1)
                                            NPC[A].Special4 = 1;
                                        else if(NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4)
                                            NPC[A].Special = 2;
                                        else if(NPC[A].Type == NPCID_DOOR_MAKER)
                                        {
                                            NPC[A].Special3 = 1;
                                            NPC[A].Projectile = false;
                                        }
                                        else if(NPC[A].Type == NPCID_EARTHQUAKE_BLOCK)
                                        {
                                            if(NPC[A].Location.SpeedY > 2)
                                                NPCHit(A, 4, A);
                                        }
                                        else if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL)
                                        {
                                            if(NPC[A].Location.SpeedX == 0)
                                                NPCHit(A, 4, A);
                                        }
                                        else if(NPC[A].Type == NPCID_SLIDE_BLOCK)
                                        {
                                            if(NPC[A].Special == 1 && NPC[A].Location.SpeedX == 0 && NPC[A].Location.SpeedY > 7.95_n)
                                                NPCHit(A, 4, A);
                                        }
                                        else if(NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG)
                                        {
                                            if(NPC[A].Location.SpeedY > Physics.NPCGravity * 20)
                                                PlaySoundSpatial(SFX_Stone, NPC[A].Location);
                                        }
                                        else if(NPC[A].Type == NPCID_TANK_TREADS)
                                        {
                                            if(NPC[A].Location.SpeedY > Physics.NPCGravity * 10)
                                                PlaySoundSpatial(SFX_Stone, NPC[A].Location);
                                        }

                                        if(WalkingCollision3(NPC[A].Location, Block[B].Location, oldBeltSpeed) || NPC[A].Location.Width > 32)
                                        {
                                            resetBeltSpeed = true;

                                            if(Block[B].tempBlockNpcType != 0)
                                            {
                                                if(Block[B].Location.SpeedY > 0 && Block[B].tempBlockNpcType >= NPCID_YEL_PLATFORM && Block[B].tempBlockNpcType <= NPCID_RED_PLATFORM)
                                                    tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01_n + Block[B].Location.SpeedY;
                                                else
                                                    tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01_n;

                                                tempHitBlock = B;
                                            }
                                            else
                                            {
                                                tempHitBlock = B;
                                                tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01_n;
                                            }

                                            if(Block[B].tempBlockNpcType >= NPCID_YEL_PLATFORM && Block[B].tempBlockNpcType <= NPCID_RED_PLATFORM)
                                            {
                                                beltSpeed = 0;
                                                beltCount = 0;
                                            }

                                            num_t C = 0;

                                            if(NPC[A].Location.X > Block[B].Location.X)
                                                C = NPC[A].Location.X - 0.01_n;
                                            else
                                                C = Block[B].Location.X - 0.01_n;

                                            if(NPC[A].Location.X + NPC[A].Location.Width < Block[B].Location.X + Block[B].Location.Width)
                                                C = (NPC[A].Location.X + NPC[A].Location.Width - C + 0.01_n);
                                            else
                                                C = (Block[B].Location.X + Block[B].Location.Width - C + 0.01_n);

                                            if(Block[B].tempBlockVehiclePlr == 0)
                                            {
                                                if(Block[B].tempBlockNpcType > 0)
                                                    beltSpeed = (tempf_t)((num_t)beltSpeed + Block[B].Location.SpeedX.times(C).times((num_t)blk_npc_tr.Speedvar));
                                                else
                                                    beltSpeed = (tempf_t)((num_t)beltSpeed + Block[B].Location.SpeedX.times(C));

                                                beltCount += tempf_t(C);
                                            }
                                        }

                                        if(tempHitBlock == B)
                                        {
                                            if(NPC[A].Type == NPCID_SPIKY_BALL_S3)
                                            {
                                                if(NPC[A].Location.SpeedY > 2)
                                                {
                                                    NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY * 0.7_r + Block[B].Location.SpeedY;

                                                    if(NPC[A].Slope == 0)
                                                        NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01_n;

                                                    tempHit = 0;
                                                    tempHitBlock = 0;
                                                }
                                            }

                                            if(((NPC[A]->StandsOnPlayer && !NPC[A].Projectile) || (NPC[A]->IsAShell && NPC[A].Location.SpeedX == 0)) && Block[B].tempBlockVehiclePlr > 0)
                                            {
                                                // IMPORTANT: this case was truncation until v1.3.7.1-dev. Confirm that changing to VB6 rounding does not cause any issues.
                                                NPC[A].vehicleYOffset = Block[B].tempBlockVehicleYOffset + num_t::vb6round(NPC[A].Location.Height);
                                                NPC[A].vehiclePlr = Block[B].tempBlockVehiclePlr;
                                                if(NPC[A].vehiclePlr == 0 && Block[B].tempBlockNpcType == NPCID_VEHICLE)
                                                    NPC[A].TimeLeft = 100;
                                            }

                                            if(NPC[A].Projectile)
                                            {
                                                if(NPC[A].Type == NPCID_PLR_FIREBALL)
                                                {
                                                    if(NPC[A].Special == 4)
                                                        NPC[A].Location.SpeedY = -3 + Block[B].Location.SpeedY;
                                                    else
                                                        NPC[A].Location.SpeedY = -5 + Block[B].Location.SpeedY;
                                                    if(NPC[A].Slope == 0)
                                                        NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01_n;
                                                    tempHit = 0;
                                                    tempHitOld = 0;
                                                }
                                                else if(NPC[A].Type == NPCID_PLR_ICEBALL)
                                                {
                                                    NPC[A].Location.SpeedY = -7 + Block[B].Location.SpeedY;
                                                    if(NPC[A].Slope == 0)
                                                        NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01_n;
                                                    tempHit = 0;
                                                    tempHitOld = 0;
                                                    if(!Block[B].Slippy)
                                                        NPC[A].Special5 += 1;
                                                    if(NPC[A].Special5 >= 3)
                                                        NPCHit(A, 3, A);
                                                }
                                                else if(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4)
                                                {
                                                    // Yes, you aren't mistook, it's just a blank block, hehehe (made by Redigit originally)


                                                }
                                                else if(NPC[A].Bouce || NPC[A].Location.SpeedY > 5.8_n || ((NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_TOOTHYPIPE) && (NPC[A].Location.SpeedY > 2 || (NPC[A].Location.SpeedX > 1 || NPC[A].Location.SpeedX < -1))))
                                                {
                                                    NPC[A].Bouce = false;
                                                    if(NPC[A]->IsAShell || (NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1) || NPC[A].Type == NPCID_ICE_CUBE)
                                                    {
                                                        if(NPC[A].Slope == 0)
                                                            NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY / 2;
                                                        for(int C = 1; C <= numPlayers; C++)
                                                        {
                                                            if(Player[C].StandingOnNPC == A)
                                                            {
                                                                NPC[A].Location.SpeedY = 0;
                                                                break;
                                                            }
                                                        }

                                                    }
                                                    else if(NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG || (NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M))
                                                        NPC[A].Location.SpeedY = 0;
                                                    else if(NPC[A].Type == NPCID_VILLAIN_S3 || NPC[A].Type == NPCID_ITEM_POD)
                                                    {
                                                        NPC[A].Projectile = false;
                                                        NPC[A].Location.SpeedY = 0;
                                                    }
                                                    else
                                                        NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY * 0.6_r;
                                                    if(NPC[A].Slope == 0)
                                                        NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01_n;
                                                    tempHit = 0;
                                                    tempHitOld = 0;
                                                }
                                                else if(NPC[A].Type != NPCID_TANK_TREADS && NPC[A].Type != NPCID_BULLET && NPC[A].Type != NPCID_PLR_FIREBALL)
                                                {
                                                    if(NPC[A]->MovesPlayer)
                                                    {
                                                        if(NPC[A].Location.SpeedX == 0)
                                                        {
                                                            bool tempBool = false;
                                                            for(int C = 1; C <= numPlayers; C++)
                                                            {
                                                                if(CheckCollision(NPC[A].Location, Player[C].Location))
                                                                {
                                                                    tempBool = true;
                                                                    break;
                                                                }
                                                            }

                                                            if(!tempBool)
                                                                NPC[A].Projectile = false;
                                                        }
                                                    }
                                                    else if(NPC[A].Type == NPCID_CHASER)
                                                    {
                                                        if(NPC[A].Location.SpeedX > -0.1_n && NPC[A].Location.SpeedX < 0.1_n)
                                                            NPC[A].Projectile = false;
                                                    }
                                                    else if(!(NPC[A]->IsAShell || (NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1) || NPC[A].Type == NPCID_SPIKY_BALL_S3))
                                                        NPC[A].Projectile = false;
                                                    else if(NPC[A].Location.SpeedX == 0)
                                                        NPC[A].Projectile = false;
                                                }
                                            }
                                        }
                                    }
                                    else if(HitSpot == 2 || HitSpot == 4) // Horizontal collisions
                                    {
                                        if(HitSpot == 2 && BlockSlope[Block[oldSlope].Type] == 1 && Block[oldSlope].Location.Y == Block[B].Location.Y)
                                        {
                                        }
                                        else
                                        {
                                            beltClear = true;
                                            if(NPC[A].Type == NPCID_VILLAIN_S3)
                                                NPC[A].Location.SpeedX = 0;

                                            resetBeltSpeed = true;
                                            if(NPC[A].Type == NPCID_PLR_FIREBALL && NPC[A].Special == 3)
                                            {
                                                if(NPC[A].Special2 == 0)
                                                {
                                                    NPC[A].Special2 = 1;
                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                }
                                                else
                                                    NPCHit(A, 4, A);
                                            }
                                            else if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_SPIT_BOSS_BALL)
                                                NPCHit(A, 4, A);
                                            if(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1)
                                                NPCHit(A, 4, A);

                                            addBelt = (tempf_t)NPC[A].Location.X;

                                            if(NPC[A].Slope == 0 && !SlopeTurn)
                                            {
                                                if(HitSpot == 2)
                                                    NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01_n;
                                                else
                                                    NPC[A].Location.X = Block[B].Location.X - NPC[A].Location.Width - 0.01_n;
                                            }

                                            addBelt = (tempf_t)(NPC[A].Location.X - (num_t)addBelt);

                                            if(!(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_TANK_TREADS || NPC[A].Type == NPCID_BULLET))
                                                NPC[A].TurnAround = true;
                                            if(is_winged)
                                                NPC[A].Location.SpeedX += -Block[B].Location.SpeedX * 1.2_r;
                                            if(NPC[A]->IsAShell)
                                                NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                        }
                                    }
                                    else if(HitSpot == 3) // Hitspot 3
                                    {
                                        if(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1)
                                            NPCHit(A, 4, A);

                                        if(NPC[A].Type == NPCID_MINIBOSS)
                                            NPC[A].Special3 = 0;

                                        if(tempBlockHit1 == 0)
                                            tempBlockHit1 = B;
                                        else
                                            tempBlockHit2 = B;

                                        if(is_winged)
                                        {
                                            NPC[A].Location.SpeedY = 2 + Block[B].Location.SpeedY;
                                            NPC[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height + 0.1_n;
                                        }
                                    }
                                    else if(HitSpot == 5) // Hitspot 5
                                    {
                                        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_SPIT_BOSS_BALL)
                                            NPCHit(A, 4, A);

                                        beltClear = true;

                                        if(NPC[A].Type == NPCID_VILLAIN_S3)
                                            NPC[A].Location.SpeedX = 0;

                                        onWall = true;

                                        // item was thrown into a wall this frame (or is a fish that just re-entered water)
                                        if(NPC[A].WallDeath >= 5 && !NPC[A]->IsABonus && NPC[A].Type != NPCID_FLY_BLOCK &&
                                           NPC[A].Type != NPCID_FLY_CANNON && NPC[A].Type != NPCID_RED_BOOT && NPC[A].Type != NPCID_CANNONENEMY && NPC[A].Type != NPCID_CANNONITEM &&
                                           NPC[A].Type != NPCID_SPRING && NPC[A].Type != NPCID_HEAVY_THROWER && NPC[A].Type != NPCID_KEY && NPC[A].Type != NPCID_COIN_SWITCH &&
                                           NPC[A].Type != NPCID_GRN_BOOT &&
                                           // Duplicated segment [PVS Studio]
                                           // NPC[A].Type != NPCID_RED_BOOT &&
                                           NPC[A].Type != NPCID_BLU_BOOT && NPC[A].Type != NPCID_TOOTHYPIPE &&
                                           NPC[A].Type != NPCID_BOMB && NPC[A].Type != NPCID_CARRY_BUDDY && NPC[A].Type != NPCID_FLIPPED_RAINBOW_SHELL && NPC[A].Type != NPCID_EARTHQUAKE_BLOCK &&
                                           !((NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D))) // walldeath stuff
                                        {
                                            NPC[A].Location.SpeedX = Physics.NPCShellSpeed / 2 * NPC[A].Direction;
                                            if(NPCIsVeggie(NPC[A]))
                                                NPC[A].Projectile = true;
                                            else if(NPC[A].Type == NPCID_WALK_BOMB_S2)
                                                NPCHit(A, 4, A);
                                            else if(NPC[A].Type == NPCID_CHAR3_HEAVY)
                                                NPCHit(A, 3, A);
                                            else
                                            {
                                                NewEffect(EFFID_WHACK, NPC[A].Location);
                                                NPC[A].Killed = 3;
                                                NPCQueues::Killed.push_back(A);
                                            }
                                        }
                                        else if(NPC[A].Type != NPCID_SPIKY_BALL_S3 && !(NPC[A]->IsABlock && Block[B].tempBlockNpcType > 0) && block_is_not_conveyor)
                                        {
                                            addBelt = (tempf_t)NPC[A].Location.X;

                                            if(NPC[A].Location.X + NPC[A].Location.Width / 2 < Block[B].Location.X + Block[B].Location.Width / 2)
                                                NPC[A].Location.X = Block[B].Location.X - NPC[A].Location.Width - 0.01_n;
                                            else
                                                NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01_n;

                                            addBelt = (tempf_t)(NPC[A].Location.X - (num_t)addBelt);

                                            if(NPC[A].Type == NPCID_MINIBOSS)
                                            {
                                                NPC[A].Location.SpeedY = 0;
                                                NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01_n;
                                            }

                                            if(NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_TANK_TREADS && NPC[A].Type != NPCID_PLR_ICEBALL)
                                                NPC[A].TurnAround = true;

                                            if(NPC[A]->IsAShell)
                                            {
                                                if(NPC[A].Location.X < Block[B].Location.X && NPC[A].Location.SpeedX > 0)
                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                else if(NPC[A].Location.X + NPC[A].Location.Width > Block[B].Location.X + Block[B].Location.Width && NPC[A].Location.SpeedX < 0)
                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                            }
                                        }
                                    }

                                    // Find best block here
                                    if(g_config.fix_npc_downward_clip && (tempHitBlock != tempHitOldBlock))
                                    {
                                        CompareNpcWalkBlock(tempHitBlock, tempHitOldBlock,
                                                            tempHit, tempHitOld,
                                                            tempHitIsSlope, &NPC[A]);
                                    }

                                    if((NPC[A].Projectile && NPC[A].Type != NPCID_PLR_FIREBALL) != 0 && NPC[A].Type != NPCID_PLR_ICEBALL && NPC[A].Type != NPCID_METALBARREL && !(NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG)) // Hit the block if the NPC is a projectile
                                    {
                                        if(HitSpot == 2 || HitSpot == 4 || HitSpot == 5)
                                        {
                                            BlockHit(B);
                                            PlaySoundSpatial(SFX_BlockHit, NPC[A].Location);
                                            if(NPC[A].Type == NPCID_BULLET) // Bullet Bills
                                            {
                                                NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                NPCHit(A, 4, A);
                                                BlockHitHard(B);
                                                break;
                                            }

                                            if(NPC[A]->IsAShell || (NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1) || NPC[A].Type == NPCID_ICE_CUBE)
                                            {
                                                BlockHitHard(B);
                                                if(Block[B].Type == 4 || Block[B].Type == 188 || Block[B].Type == 60 || Block[B].Type == 90)
                                                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
                                            }
                                        }
                                    }
                                }
                                // End If
                            }
                        }
                    }
                }
                else
                {
                    if((bCheck == 2 || BlocksSorted) && PSwitchTime == 0)
                        break;
                }
            }

            if(numTempBlock == 0)
                break;
        }
    } // do block collisions

    // ceiling logic (safe to move into above braces if desired; it's impossible for tempBlockHit to be set unless that clause executed)
    if(tempBlockHit1 > 0) // find out which block was hit from below
    {
        int winningBlock = 0;

        if(tempBlockHit2 == 0)
            winningBlock = tempBlockHit1;
        else
        {
            num_t C = num_t::abs(Block[tempBlockHit1].Location.minus_center_x(NPC[A].Location));
            num_t D = num_t::abs(Block[tempBlockHit2].Location.minus_center_x(NPC[A].Location));

            if(C < D)
                winningBlock = tempBlockHit1;
            else
                winningBlock = tempBlockHit2;
        }

        // possible usually-nulled function pointer here: ceiling collision logic. Takes winningBlock. Returns true if we should set the NPC's speed / location by the ceiling, otherwise false.
        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL) // Kill the fireball
            NPCHit(A, 4);
        else if(NPC[A].Projectile || Block[winningBlock].Invis) // Hit the block hard if the NPC is a projectile
        {
            if(!(NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG || NPC[A].Type == NPCID_CANNONENEMY))
            {
                if(NPC[A].Location.SpeedY < -0.05_n)
                {
                    BlockHit(winningBlock);
                    PlaySoundSpatial(SFX_BlockHit, NPC[A].Location);
                    if(NPC[A]->IsAShell || NPC[A].Type == NPCID_ICE_CUBE)
                        BlockHitHard(winningBlock);
                }
                else
                    NPC[A].Projectile = false;
            }
        }

        if(!is_winged)
        {
            NPC[A].Location.Y = Block[winningBlock].Location.Y + Block[winningBlock].Location.Height + 0.01_n;

            if(g_config.fix_npc_ceiling_speed)
                NPC[A].Location.SpeedY = 0.01_n + Block[winningBlock].Location.SpeedY;
            else
            {
                // This is the original extremely buggy line, using an arbitrary B from the end of the old fBlock/lBlock or tempBlock query.
                // NPC[A].Location.SpeedY = 0.01 + Block[B].Location.SpeedY;

                // Unfortunately, we need to emulate it. We're lucky that NPCs don't frequently hit ceilings.

                // our old... friends?
                int fBlock, lBlock;

                // if no temp blocks, then no second pass occurred in the check loops above where B was set
                if(numTempBlock != 0)
                {
                    fBlock = numBlock + 1 - numTempBlock;
                    lBlock = numBlock;
                }
                else
                {
                    fBlock = 1;
                    lBlock = numBlock - numTempBlock;
                }

                int B = -1;

                // The first line contains the original condition that guarded the "break" in our loop above B.
                // Note that this means the bug would be different during PSwitch or after a horiz layer has moved.
                // The second line lets us find the Block that is accessed upon overflow of the original FLBlock column.
                if((PSwitchTime == 0 && (BlocksSorted || numTempBlock != 0))
                    || (BlocksSorted && numTempBlock == 0))
                {
                    // We could use a quadtree here, but this is a rare case. Just do it.

                    // Normally, need to find the first Block (in sorted order) which is to the right of the NPC.
                    num_t right_border = NPC[A].Location.X + NPC[A].Location.Width;

                    int first_after_block = -1;
                    num_t first_after_x = 0, first_after_y = 0;

                    // IF PSwitchTime != 0 but the blocks are sorted,
                    // then we would have iterated over the NPC's entire FLBlock column without breaking,
                    // and ended with B set to the first block after it.
                    // To emulate, move the right_border to the end of its FLBlock column.
                    // vb6 rounds its array indexes from doubles to integers
                    if(PSwitchTime != 0)
                        right_border = num_t::vb6round(right_border / 32 + 1) * 32;

                    // If the loop never invoked break and was not over a single column,
                    // then the game would have accessed numBlock + 1 here, but we won't.
                    // We'll assume it was properly deallocated and has SpeedY = 0.

                    for(int block = fBlock; block <= lBlock; block++)
                    {
                        // Old code checked coordinates first, then properties without affecting the loop.
                        // This means we don't need to touch any properties here.

                        const Block_t& b = Block[block];

                        num_t bx = b.Location.X;
                        num_t by = b.Location.Y;

                        // sort as they were in the original array
                        if(b.Layer != LAYER_NONE)
                        {
                            bx -= Layer[b.Layer].OffsetX;
                            by -= Layer[b.Layer].OffsetY;
                        }

                        if(b.Location.X > right_border)
                        {
                            if(first_after_block == -1 || bx < first_after_x || (bx == first_after_x && by < first_after_y))
                            {
                                first_after_block = block;
                                first_after_x = bx;
                                first_after_y = by;

                                // want the first one in the SMBX sorted order, which might not be accurate
                                if(g_config.emulate_classic_block_order && numTempBlock == 0)
                                    break;
                            }
                        }
                    }

                    B = first_after_block;
                }
                else
                {
                    // The game went through the full loop and B = numBlock + 1 in vanilla.
                    // We'll assume it was properly deallocated and has SpeedY = 0.
                }

                if(B != -1)
                {
                    NPC[A].Location.SpeedY = 0.01_n + Block[B].Location.SpeedY;
                    // pLogDebug("NPC %d hits ceiling, set SpeedY using block %d with speed %f\n", A, B, Block[B].Location.SpeedY);
                }
                else
                {
                    NPC[A].Location.SpeedY = 0.01_n;
                    // pLogDebug("NPC %d hits ceiling, set SpeedY using OOB block with speed 0\n", A);
                }
            }
        }
    }

    // beltspeed code
    if(!resetBeltSpeed)
    {
        if(NPC[A].Type == NPCID_VILLAIN_S3 && NPC[A].Special == 1)
            NPC[A].Special = 0;

        if(oldBeltSpeed >= 1 || oldBeltSpeed <= -1)
        {
            beltSpeed = oldBeltSpeed - (tempf_t)NPC[A].oldAddBelt;
            beltCount = 1;
            if(beltSpeed >= (tempf_t)2.1_n)
                beltSpeed -= (tempf_t)0.1_n;
            else if(beltSpeed <= -(tempf_t)2.1_n)
                beltSpeed += (tempf_t)0.1_n;
        }
    }

    if(beltSpeed != 0)
    {
        Location_t preBeltLoc = NPC[A].Location;
        beltSpeed = beltSpeed.divided_by(beltCount).times(speedVar);
        NPC[A].Location.X += num_t(beltSpeed);
//                            D = NPC[A].BeltSpeed; // Idk why this is needed as this value gets been overriden and never re-used
        Location_t tempLocation = NPC[A].Location;
        tempLocation.Y += 1;
        tempLocation.Height -= 2;
        tempLocation.Width = tempLocation.Width / 2;

        if(beltSpeed > 0)
            tempLocation.X += tempLocation.Width;

        if(!(NPC[A].Type >= NPCID_SHORT_WOOD && NPC[A].Type <= NPCID_SLANT_WOOD_M) && !NPC[A].Inert)
        {
            for(int C : treeNPCQuery(tempLocation, SORTMODE_NONE))
            {
                if(A != C && NPC[C].Active && !NPC[C].Projectile)
                {
                    if(NPC[C].Killed == 0 && NPC[C].vehiclePlr == 0 && NPC[C].HoldingPlayer == 0 &&
                       !NPC[C]->NoClipping && NPC[C].Effect == NPCEFF_NORMAL && !NPC[C].Inert) // And Not NPCIsABlock(NPC(C).Type) Then
                    {
                        Location_t tempLocation2 = preBeltLoc;
                        tempLocation2.Width -= 4;
                        tempLocation2.X += 2;
                        if(CheckCollision(tempLocation, NPC[C].Location))
                        {
                            if(!CheckCollision(tempLocation2, NPC[C].Location))
                            {
                                if(NPC[A].TimeLeft - 1 > NPC[C].TimeLeft)
                                    NPC[C].TimeLeft = NPC[A].TimeLeft - 1;
                                else if(NPC[A].TimeLeft < NPC[C].TimeLeft - 1)
                                    NPC[A].TimeLeft = NPC[C].TimeLeft - 1;
                                onWall = true;
                                if((NPC[A].Location.SpeedX > 0 && NPC[C].Location.X > NPC[A].Location.X) || (NPC[A].Location.SpeedX < 0 && NPC[C].Location.X < NPC[A].Location.X))
                                    NPC[A].TurnAround = true;
                                if((NPC[C].Location.SpeedX > 0 && NPC[A].Location.X > NPC[C].Location.X) || (NPC[C].Location.SpeedX < 0 && NPC[A].Location.X < NPC[C].Location.X))
                                    NPC[C].TurnAround = true;
                                NPC[A].Location = preBeltLoc;
                            }
                        }
                    }
                }
            }
        }

        if(NPC[A].Location.X == preBeltLoc.X)
        {
            beltSpeed = 0;
            addBelt = 0;
            if(NPC[A].tempBlock > 0)
                Block[NPC[A].tempBlock].Location.SpeedX = 0;
        }
    }

    if(!onWall)
        beltSpeed += addBelt;

    NPC[A].oldAddBelt = (numf_t)addBelt;

    if(beltClear || NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4)
        NPC[A].BeltSpeed = 0;
    else
        NPC[A].BeltSpeed = (numf_t)beltSpeed;
}
