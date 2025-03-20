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

#include <Logger/logger.h>

#include "globals.h"

#include "player.h"
#include "blocks.h"
#include "sound.h"
#include "layers.h"
#include "effect.h"
#include "eff_id.h"
#include "collision.h"
#include "eff_id.h"
#include "blk_id.h"
#include "config.h"

#include "main/trees.h"

void PlayerBlockLogic(int A, int& floorBlock, bool& movingBlock, bool& DontResetGrabTime, float cursed_value_C)
{
    int oldSlope = Player[A].Slope;
    Player[A].Slope = 0;

    int oldStandingOnNpc = Player[A].StandingOnNPC;

    int blockPushX = 0;
    bool tempHit = false;
    bool tempHit2 = false;

    int tempSlope = 0;
    int tempSlope2 = 0;
    int tempSlope3 = 0; // keeps track of hit 5 for slope detection

    int ceilingBlock1 = 0;
    int ceilingBlock2 = 0;

    Location_t floorLocation; // was previously called tempLocation3

    // This was previously shared between players, but is safe unless Block[tempSlope2] satisfies certain properties before tempSlope2 gets set
    double tempSlope2X = 0; // The old X before player was moved

    if(Player[A].Character == 5 && Player[A].Duck && (Player[A].Location.SpeedY == Physics.PlayerGravity || Player[A].StandingOnNPC != 0 || Player[A].Slope != 0))
    {
        Player[A].Location.Y += Player[A].Location.Height;
        Player[A].Location.Height = 30;
        Player[A].Location.Y += -Player[A].Location.Height;
    }


    // block collision optimization
    // fBlock = FirstBlock[(Player[A].Location.X / 32) - 1];
    // lBlock = LastBlock[((Player[A].Location.X + Player[A].Location.Width) / 32.0) + 1];
    // blockTileGet(Player[A].Location, fBlock, lBlock);

    UpdatableQuery<BlockRef_t> q(Player[A].Location, SORTMODE_COMPAT, QUERY_FLBLOCK);

    for(auto it = q.begin(); it != q.end(); ++it)
    {
        int B = *it;

        // checks to see if a collision happened
        if(Player[A].Location.X + Player[A].Location.Width >= Block[B].Location.X)
        {
            if(Player[A].Location.X <= Block[B].Location.X + Block[B].Location.Width)
            {
                if(Player[A].Location.Y + Player[A].Location.Height >= Block[B].Location.Y)
                {
                    if(Player[A].Location.Y <= Block[B].Location.Y + Block[B].Location.Height)
                    {

                        if(!Block[B].Hidden)
                        {
                            // the hitspot is used for collision detection to find out where to put the player after it collides with a block
                            // the numbers tell what side the collision happened so it can move the plaer to the correct position
                            // 1 means the player hit the block from the top
                            // 2 is from the right
                            // 3 is from the bottom
                            // 4 is from the left
                            double block_belt_speed = 0;
                            if(Block[B].Type >= BLKID_CONVEYOR_L_START && Block[B].Type <= BLKID_CONVEYOR_L_END)
                                block_belt_speed = -0.8;
                            else if(Block[B].Type >= BLKID_CONVEYOR_R_START && Block[B].Type <= BLKID_CONVEYOR_R_END)
                                block_belt_speed = 0.8;

                            int HitSpot = FindRunningCollision(Player[A].Location, Block[B].Location, block_belt_speed); // this finds what part of the block the player collided

                            if(BlockNoClipping[Block[B].Type]) // blocks that the player can't touch are forced to hitspot 0 (which means no collision)
                                HitSpot = 0;

                            if(BlockIsSizable[Block[B].Type] || BlockOnlyHitspot1[Block[B].Type]) // for sizable blocks, if the player didn't land on them from the top then he can walk through them
                            {
                                if(HitSpot != 1)
                                    HitSpot = 0;

                                if(Player[A].Mount == 2 || Player[A].StandingOnVehiclePlr != 0)
                                    HitSpot = 0;
                            }

                            // for blocks that hurt the player
                            if(BlockHurts[Block[B].Type])
                            {
                                if(Player[A].Mount == 2 ||
                                   (
                                       (HitSpot == 1 && Player[A].Mount != 0) &&
                                       Block[B].Type != 598
                                   )
                                 )
                                {}
                                else
                                {
                                    if(HitSpot == 1 && (Block[B].Type == 110 || Block[B].Type == 408 || Block[B].Type == 430 || Block[B].Type == 511))
                                        PlayerHurt(A);
                                    if(HitSpot == 4 && (Block[B].Type == 269 || Block[B].Type == 429))
                                        PlayerHurt(A);
                                    if(HitSpot == 3 && (Block[B].Type == 268 || Block[B].Type == 407 || Block[B].Type == 431))
                                        PlayerHurt(A);
                                    if(HitSpot == 2 && (Block[B].Type == 267 || Block[B].Type == 428))
                                        PlayerHurt(A);
                                    if(Block[B].Type == 109)
                                        PlayerHurt(A);
                                    if(Block[B].Type == 598)
                                    {
                                        if(Player[A].Mount > 0 && HitSpot == 1)
                                        {
                                            cursed_value_C = Player[A].Location.Y + Player[A].Location.Height;
                                            Player[A].Location.Y = Block[B].Location.Y - Player[A].Location.Height;
                                            PlayerHurt(A);
                                            Player[A].Location.Y = cursed_value_C - Player[A].Location.Height;
                                        }
                                        else
                                            PlayerHurt(A);
                                    }


                                    if(Player[A].TimeToLive > 0)
                                        break;
                                }
                            }

                            // hitspot 5 means the game doesn't know where the collision happened
                            // if the player just stopped ducking and there is a hitspot 5 then force hitspot 3 (hit block from below)
                            if(HitSpot == 5 && (Player[A].StandUp || NPC[Player[A].StandingOnNPC].Location.SpeedY < 0))
                            {
                                if(BlockSlope[Block[B].Type] == 0)
                                    HitSpot = 3;
                            }

                            // if the block is invisible and the player didn't hit it from below then the player won't collide with it
                            if(Block[B].Invis)
                            {
                                if(HitSpot != 3)
                                    HitSpot = 0;
                            }

                            // unclear that this does what Redigit thought; maybe it did with a previous version of the coin switch code

                            // ' fixes a bug with holding an npc that is really a block
                            if(Player[A].HoldingNPC > 0)
                            {
                                if(NPC[Player[A].HoldingNPC].coinSwitchBlockType > 0)
                                {
                                    if(NPC[Player[A].HoldingNPC].coinSwitchBlockType == B)
                                        HitSpot = 0;
                                }
                            }

                            // destroy some blocks if the player is touching it as a statue
                            if(Block[B].Type == 457 && Player[A].Stoned)
                            {
                                HitSpot = 0;
                                SafelyKillBlock(B);
                            }

                            // shadowmode is a cheat that allows the player to walk through walls
                            if(ShadowMode && HitSpot != 1 && !(Block[B].Special > 0 && HitSpot == 3))
                                HitSpot = 0;

                            // this handles the collision for blocks that are sloped on the bottom
                            if(BlockSlope2[Block[B].Type] != 0 && (Player[A].Location.Y > Block[B].Location.Y || (HitSpot != 2 && HitSpot != 4)) && HitSpot != 1 && !ShadowMode)
                            {
                                HitSpot = 0;
                                tempSlope = B;

                                double PlrMid;
                                if(BlockSlope2[Block[B].Type] == 1)
                                    PlrMid = Player[A].Location.X + Player[A].Location.Width;
                                else
                                    PlrMid = Player[A].Location.X;

                                double Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;

                                if(BlockSlope2[Block[B].Type] > 0)
                                    Slope = 1 - Slope;

                                if(Slope < 0)
                                    Slope = 0;

                                if(Slope > 1)
                                    Slope = 1;

                                if(Player[A].Location.Y <= Block[B].Location.Y + Block[B].Location.Height - (Block[B].Location.Height * Slope))
                                {
                                    if(BlockKills[Block[B].Type])
                                    {
                                        if(!GodMode)
                                            PlayerDead(A);
                                    }

                                    if(Player[A].Location.SpeedY == 0.0 ||
                                       fEqual(float(Player[A].Location.SpeedY), Physics.PlayerGravity) || Player[A].Slope > 0 || Player[A].StandingOnNPC != 0)
                                    {
                                        double PlrMid = Player[A].Location.Y;
                                        Slope = (PlrMid - Block[B].Location.Y) / Block[B].Location.Height;

                                        if(Slope < 0)
                                            Slope = 0;

                                        if(Slope > 1)
                                            Slope = 1;

                                        if(BlockSlope2[Block[B].Type] < 0)
                                            Player[A].Location.X = Block[B].Location.X + Block[B].Location.Width - (Block[B].Location.Width * Slope);
                                        else
                                            Player[A].Location.X = Block[B].Location.X + (Block[B].Location.Width * Slope) - Player[A].Location.Width;

                                        Player[A].Location.SpeedX = 0;

                                    }
                                    else
                                    {
                                        Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height - (Block[B].Location.Height * Slope);
                                        if(Player[A].Location.SpeedY < 0)
                                            PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                        if(Player[A].Location.SpeedY < -0.01)
                                            Player[A].Location.SpeedY = -0.01;
                                        if(Player[A].Mount == 2)
                                            Player[A].Location.SpeedY = 2;
                                        if(Player[A].CanFly2)
                                            Player[A].Location.SpeedY = 2;
                                    }

                                    Player[A].Jump = 0;
                                }
                            }

                            // collision for blocks that are sloped on the top
                            // MOST CURSED LINE: cursed_value_C (C in VB6 code) is entirely arbitrary at this point. BE WARNED, this may be a cause of incompatibilities with SMBX 1.3, and if so, we will need to improve this logic in the future.
                            if(BlockSlope[Block[B].Type] != 0 && HitSpot != 3 && !(BlockSlope[Block[B].Type] == -1 && HitSpot == 2) && !(BlockSlope[Block[B].Type] == 1 && HitSpot == 4) && (Player[A].Location.Y + Player[A].Location.Height - 4 - cursed_value_C <= Block[B].Location.Y + Block[B].Location.Height || (Player[A].Location.Y + Player[A].Location.Height - 12 <= Block[B].Location.Y + Block[B].Location.Height && Player[A].StandingOnNPC != 0)))
                            {
                                HitSpot = 0;
                                if(
                                        (Player[A].Mount == 1 || Player[A].Location.SpeedY >= 0 ||
                                         Player[A].Slide || SuperSpeed || Player[A].Stoned) &&
                                        (Player[A].Location.Y + Player[A].Location.Height <= Block[B].Location.Y + Block[B].Location.Height + Player[A].Location.SpeedY + 0.001 ||
                                         (Player[A].Slope == 0 && Block[B].Location.SpeedY < 0))
                                        )
                                {
                                    double PlrMid;
                                    if(BlockSlope[Block[B].Type] == 1)
                                        PlrMid = Player[A].Location.X;
                                    else
                                        PlrMid = Player[A].Location.X + Player[A].Location.Width;

                                    double Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;

                                    if(BlockSlope[Block[B].Type] < 0)
                                        Slope = 1 - Slope;

                                    if(Slope < 0)
                                        Slope = 0;

                                    if(Slope > 1)
                                        Slope = 1;

                                    // if we're already on top of another (higher or more leftwards, at level load time) block this frame, consider canceling it
                                    if(floorBlock > 0)
                                    {
                                        // the bug this is fixing is vanilla, but this case happens for a single frame every time a slope falls through ground since TheXTech 1.3.6,
                                        // and only in the rare case where a slope falls through ground *it was originally below* in vanilla
                                        if(g_config.fix_player_downward_clip && !CompareWalkBlock(floorBlock, B, Player[A].Location))
                                        {
                                            // keep the old block, other conditions are VERY likely to cancel it
                                        }
                                        else if(!BlockIsSizable[Block[floorBlock].Type])
                                        {
                                            if(Block[floorBlock].Location.Y != Block[B].Location.Y)
                                                floorBlock = 0;
                                        }
                                        else
                                        {
                                            // NOTE: looks like a good place for a vb6-style fEqual
                                            if(Block[floorBlock].Location.Y == Block[B].Location.Y + Block[B].Location.Height)
                                                floorBlock = 0;
                                        }
                                    }

                                    if(tempHit2)
                                    {
                                        // NOTE: looks like a good place for a vb6-style fEqual
                                        if(Block[tempSlope2].Location.Y + Block[tempSlope2].Location.Height == Block[B].Location.Y && BlockSlope[Block[tempSlope2].Type] == BlockSlope[Block[B].Type])
                                        {
                                            tempHit2 = false;
                                            tempSlope2 = 0;
                                            Player[A].Location.X = tempSlope2X;
                                        }
                                    }

                                    if(tempSlope3 > 0)
                                    {
                                        Player[A].Location.Y = Block[tempSlope3].Location.Y + Block[tempSlope3].Location.Height + 0.01;
                                        double PlrMid = Player[A].Location.Y + Player[A].Location.Height;
                                        double Slope = 1 - (PlrMid - Block[B].Location.Y) / Block[B].Location.Height;

                                        if(Slope < 0)
                                            Slope = 0;

                                        if(Slope > 1)
                                            Slope = 1;

                                        if(BlockSlope[Block[B].Type] > 0)
                                            Player[A].Location.X = Block[B].Location.X + Block[B].Location.Width - (Block[B].Location.Width * Slope);
                                        else
                                            Player[A].Location.X = Block[B].Location.X + (Block[B].Location.Width * Slope) - Player[A].Location.Width;

                                        Player[A].Location.SpeedX = 0;
                                    }
                                    else
                                    {
                                        if(Player[A].Location.Y >= Block[B].Location.Y + (Block[B].Location.Height * Slope) - Player[A].Location.Height - 0.1)
                                        {

                                            if(Player[A].GroundPound)
                                            {
                                                YoshiPound(A, Player[A].Mount, true);
                                                Player[A].GroundPound = false;
                                            }
                                            else if(Player[A].YoshiYellow)
                                            {
                                                if(oldSlope == 0)
                                                    YoshiPound(A, Player[A].Mount);
                                            }

                                            Player[A].Location.Y = Block[B].Location.Y + (Block[B].Location.Height * Slope) - Player[A].Location.Height - 0.1;

                                            if(Player[A].Location.SpeedY > Player[A].Location.SpeedX * (Block[B].Location.Height / Block[B].Location.Width) * BlockSlope[Block[B].Type] || !Player[A].Slide)
                                            {
                                                if(!Player[A].WetFrame)
                                                {
                                                    cursed_value_C = Player[A].Location.SpeedX * (Block[B].Location.Height / Block[B].Location.Width) * BlockSlope[Block[B].Type];
                                                    Player[A].Location.SpeedY = cursed_value_C;
                                                    if(Player[A].Location.SpeedY > 0 && !Player[A].Slide && Player[A].Mount != 1 && Player[A].Mount != 2)
                                                        Player[A].Location.SpeedY = Player[A].Location.SpeedY * 4;
                                                }
                                            }

                                            Player[A].Slope = B;
                                            if(BlockSlope[Block[B].Type] == 1 && GameMenu && Player[A].Location.SpeedX >= 2)
                                            {
                                                if(Player[A].Mount == 0 && Player[A].HoldingNPC == 0 && Player[A].Character <= 2)
                                                {
                                                    if(Player[A].Duck)
                                                        UnDuck(Player[A]);
                                                    Player[A].Slide = true;
                                                }
                                            }



                                            if(Player[A].Location.SpeedY < 0 && !Player[A].Slide && !SuperSpeed && !Player[A].Stoned)
                                                Player[A].Location.SpeedY = 0;
                                            if(Block[B].Location.SpeedX != 0 || Block[B].Location.SpeedY != 0)
                                            {
                                                NPC[-A] = NPC_t();
                                                NPC[-A].Location = Block[B].Location;
                                                NPC[-A].Type = NPCID_METALBARREL;
                                                NPC[-A].Active = true;
                                                NPC[-A].TimeLeft = 100;
                                                NPC[-A].Section = Player[A].Section;
                                                NPC[-A].Special = B;
                                                NPC[-A].Special2 = BlockSlope[Block[B].Type];
                                                Player[A].StandingOnNPC = -A;
                                                movingBlock = true;
                                                // NOTE: Here was a bug that makes compare bool with 0 is always false
                                                if(
                                                        (g_config.fix_player_slope_speed &&
                                                         Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX < 0 &&
                                                         BlockSlope[Block[B].Type]/*)*/ < 0) ||
                                                        (Player[A].Location.SpeedX - NPC[Player[A].StandingOnNPC].Location.SpeedX > 0 &&
                                                         BlockSlope[Block[B].Type] > 0)
                                                        )
                                                {
                                                    if((Player[A].Location.SpeedX < 0 && Block[B].Location.SpeedX > 0) || (Player[A].Location.SpeedX > 0 && Block[B].Location.SpeedX < 0))
                                                        Player[A].Location.SpeedY = 12;
                                                }

                                                NPC[-A].Location.Y = Player[A].Location.Y + Player[A].Location.Height;
                                            }
                                        }
                                    }
                                }
                            }


                            // this is a fix to help the player deal with lava blocks a bit easier
                            // it moves the blocks hitbox down a few pixels
                            if(BlockKills[Block[B].Type] && BlockSlope[Block[B].Type] == 0 && !GodMode && !(Player[A].Mount == 1 && Player[A].MountType == 2))
                            {
                                if(Player[A].Location.Y + Player[A].Location.Height < Block[B].Location.Y + 6)
                                    HitSpot = 0;
                            }

                            // kill the player if touching a lava block
                            if(BlockKills[Block[B].Type] && (HitSpot > 0 || Player[A].Slope == B))
                            {
                                if(!GodMode)
                                {
                                    if(!(Player[A].Mount == 1 && Player[A].MountType == 2))
                                    {
                                        PlayerDead(A);
                                        break;
                                    }
                                    else if(HitSpot != 1 && BlockSlope[Block[B].Type] == 0)
                                    {
                                        PlayerDead(A);
                                        break;
                                    }
                                    else
                                    {
                                        Location_t tempLocation;
                                        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 2;
                                        tempLocation.X = Player[A].Location.X - 4 + dRand() * (Player[A].Location.Width + 8) - 4;
                                        NewEffect(EFFID_SKID_DUST, tempLocation);
                                    }
                                }
                            }

                            // if hitspot 5 with a sloped block then don't collide with it. the collision should have already been handled by the slope code above
                            if(HitSpot == 5 && BlockSlope[Block[B].Type] != 0)
                                HitSpot = 0;

                            // shelsurfing code
                            if(HitSpot > 1 && Player[A].ShellSurf)
                            {
                                Player[A].ShellSurf = false;
                                Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY + Physics.PlayerJumpVelocity * 0.75;
                                Player[A].StandingOnNPC = 0;
                                PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                            }

                            if(BlockCheckPlayerFilter(B, A))  // Optmizied
                                HitSpot = 0;
                            //if(Block[B].Type == 626 && Player[A].Character == 1)
                            //    HitSpot = 0;
                            //if(Block[B].Type == 627 && Player[A].Character == 2)
                            //    HitSpot = 0;
                            //if(Block[B].Type == 628 && Player[A].Character == 3)
                            //    HitSpot = 0;
                            //if(Block[B].Type == 629 && Player[A].Character == 4)
                            //    HitSpot = 0;
                            //if(Block[B].Type == 632 && Player[A].Character == 5)
                            //    HitSpot = 0;

                            if(g_config.fix_player_clip_wall_at_npc && (HitSpot == 5 || HitSpot == 3) && oldStandingOnNpc > 0 && Player[A].Jump)
                            {
                                // Re-compute the collision with a block to avoid the unnecessary clipping through the wall
                                auto pLoc = Player[A].Location;
                                pLoc.SpeedX += NPC[oldStandingOnNpc].Location.SpeedX;
                                pLoc.SpeedY += NPC[oldStandingOnNpc].Location.SpeedY;
                                HitSpot = FindRunningCollision(pLoc, Block[B].Location, block_belt_speed);
                                D_pLogDebug("Conveyor: Recomputed collision with block %d", B);
                            }

                            // the following code is where the collisions are handled


                            if((HitSpot == 1 || Player[A].Slope == B) && Block[B].Slippy)
                                Player[A].Slippy = true;


                            if(HitSpot == 5 && Player[A].Quicksand > 0) // fixes quicksand hitspot 3 bug
                            {
                                if(Player[A].Location.Y - Player[A].Location.SpeedY < Block[B].Location.Y + Block[B].Location.Height)
                                    HitSpot = 3;
                            }

                            if(HitSpot == 1) // landed on the block from the top V
                            {
                                if(Player[A].Fairy && (Player[A].FairyCD > 0 || Player[A].Location.SpeedY > 0))
                                    Player[A].FairyTime = 0;

                                Player[A].Pinched.Bottom1 = 2; // for players getting squashed

                                if(Block[B].Location.SpeedY != 0)
                                {
                                    Player[A].Pinched.Moving = 2;
                                    Player[A].Pinched.MovingUD = true;
                                }

                                Player[A].Vine = 0; // stop climbing because you are now walking
                                if(Player[A].Mount == 2) // for the clown car, make a niose and pound the ground if moving down fast enough
                                {
                                    if(Player[A].Location.SpeedY > 3)
                                    {
                                        PlaySoundSpatial(SFX_Stone, Player[A].Location);
                                        YoshiPound(A, Player[A].Mount, true);
                                    }
                                }

                                if(floorBlock == 0) // For walking
                                {
                                    floorBlock = B;
                                    floorLocation = Block[B].Location;
                                }
                                else // Find the best block to walk on if touching multiple blocks
                                {
                                    if(g_config.fix_player_downward_clip)
                                    {
                                        if(CompareWalkBlock(floorBlock, B, Player[A].Location))
                                        {
                                            floorBlock = B;
                                            floorLocation = Block[B].Location;
                                        }
                                    }
                                    else // Using old code
                                    {
                                        if(Block[B].Location.SpeedY != 0 && Block[floorBlock].Location.SpeedY == 0)
                                        {
                                            floorBlock = B;
                                            floorLocation = Block[B].Location;
                                        }
                                        else if(Block[B].Location.SpeedY == 0 && Block[floorBlock].Location.SpeedY != 0)
                                        {
                                        }
                                        else
                                        {
                                            cursed_value_C = Block[B].Location.X + Block[B].Location.Width * 0.5;
                                            float D = Block[floorBlock].Location.X + Block[floorBlock].Location.Width * 0.5;

                                            cursed_value_C += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
                                            D += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
                                            if(cursed_value_C < 0)
                                                cursed_value_C = -cursed_value_C;
                                            if(D < 0)
                                                D = -D;
                                            if(cursed_value_C < D)
                                                floorBlock = B;
                                        }

                                        // if this block is moving up give it priority
                                        if(Block[B].Location.SpeedY < 0 && Block[B].Location.Y < Block[floorBlock].Location.Y)
                                        {
                                            floorBlock = B;
                                            floorLocation = Block[B].Location;
                                        }
                                    }
                                }

                            }
                            else if(HitSpot == 2 || HitSpot == 4) // hit the block from the right <---- (or left now! -------.)
                            {
                                if(HitSpot == 2 && BlockSlope[Block[oldSlope].Type] == 1 && Block[oldSlope].Location.Y <= Block[B].Location.Y)
                                {
                                    // Just a blank block :-P
                                }
                                else
                                {
                                    tempSlope2X = Player[A].Location.X;

                                    if(HitSpot == 4)
                                    {
                                        Player[A].Location.X = Block[B].Location.X - Player[A].Location.Width - 0.01;
                                        Player[A].Pinched.Right4 = 2;
                                    }
                                    else
                                    {
                                        Player[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01;
                                        Player[A].Pinched.Left2 = 2;
                                    }

                                    if(Block[B].Location.SpeedX != 0)
                                    {
                                        Player[A].Pinched.Moving = 2;
                                        Player[A].Pinched.MovingLR = true;
                                    }

                                    if(Player[A].Mount == 2)
                                    {
                                        // cast to float because in VB6 the old X location was temporarily stored in mountBump, which is a float
                                        Player[A].mountBump = Player[A].Location.X - (float)tempSlope2X;
                                    }

                                    tempSlope2 = B;
                                    tempHit2 = true;

                                    // FIXME: is truncation really the correct behavior here? that's what TheXTech has always done.
                                    blockPushX = (int)(Block[B].Location.SpeedX);
                                }
                            }
                            else if(HitSpot == 3) // hit the block from below
                            {
                                // add more generous margin to prevent unfair crush death with sloped ceiling
                                bool ignore = (g_config.fix_player_crush_death
                                    && (Block[B].Location.X + Block[B].Location.Width - 2 < Player[A].Location.X
                                        || Player[A].Location.X + Player[A].Location.Width - 2 < Block[B].Location.X));

                                if(!Player[A].ForceHitSpot3 && !Player[A].StandUp && !ignore)
                                    Player[A].Pinched.Top3 = 2;

                                if(Block[B].Location.SpeedY != 0 && !ignore)
                                {
                                    Player[A].Pinched.Moving = 2;
                                    Player[A].Pinched.MovingUD = true;
                                }

                                tempHit = true;
                                if(ceilingBlock1 == 0)
                                    ceilingBlock1 = B;
                                else
                                    ceilingBlock2 = B;
                            }
                            else if(HitSpot == 5) // try to find out where the player hit the block from
                            {
                                if(oldSlope > 0)
                                {
                                    Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height + 0.01;
                                    double PlrMid = Player[A].Location.Y + Player[A].Location.Height;
                                    double Slope = 1 - (PlrMid - Block[oldSlope].Location.Y) / Block[oldSlope].Location.Height;
                                    if(Slope < 0)
                                        Slope = 0;
                                    if(Slope > 1)
                                        Slope = 1;
                                    if(BlockSlope[Block[oldSlope].Type] > 0)
                                        Player[A].Location.X = Block[oldSlope].Location.X + Block[oldSlope].Location.Width - (Block[oldSlope].Location.Width * Slope);
                                    else
                                        Player[A].Location.X = Block[oldSlope].Location.X + (Block[oldSlope].Location.Width * Slope) - Player[A].Location.Width;
                                    Player[A].Location.SpeedX = 0;
                                }
                                else
                                {
                                    tempSlope3 = B;
                                    if(Player[A].Location.X + Player[A].Location.Width / 2 < Block[B].Location.X + Block[B].Location.Width / 2)
                                        Player[A].Pinched.Right4 = 2;
                                    else
                                        Player[A].Pinched.Left2 = 2;
                                    if(Block[B].Location.SpeedX != 0 || Block[B].Location.SpeedY != 0)
                                    {
                                        Player[A].Pinched.Moving = 2;

                                        if(Block[B].Location.SpeedX != 0)
                                            Player[A].Pinched.MovingLR = true;

                                        if(Block[B].Location.SpeedY != 0)
                                            Player[A].Pinched.MovingUD = true;
                                    }
                                    Location_t tempLocation;
                                    tempLocation.X = Player[A].Location.X;
                                    tempLocation.Width = Player[A].Location.Width;
                                    tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height;
                                    tempLocation.Height = 0.1;
                                    bool tempBool = false;

                                    // this could have caused an unusual TheXTech bug where lBlock would get overwritten
                                    // (this wouldn't affect VB6 because loop bounds are evaluated only on loop start)

                                    // fBlock = FirstBlock[(tempLocation.X / 32) - 1];
                                    // lBlock = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                    // blockTileGet(tempLocation, fBlock, lBlock);

                                    for(int C : treeFLBlockQuery(tempLocation, SORTMODE_COMPAT))
                                    {
                                        if(CheckCollision(tempLocation, Block[C].Location) && !Block[C].Hidden)
                                        {
                                            if(BlockSlope[Block[C].Type] == 0)
                                                tempBool = true;
                                            else
                                            {
                                                Player[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height; // + 0.01
                                                double PlrMid = Player[A].Location.Y + Player[A].Location.Height;
                                                double Slope = 1 - (PlrMid - Block[C].Location.Y) / Block[C].Location.Height;
                                                if(Slope < 0)
                                                    Slope = 0;
                                                if(Slope > 1)
                                                    Slope = 1;
                                                if(BlockSlope[Block[C].Type] > 0)
                                                    Player[A].Location.X = Block[C].Location.X + Block[C].Location.Width - (Block[C].Location.Width * Slope);
                                                else
                                                    Player[A].Location.X = Block[C].Location.X + (Block[C].Location.Width * Slope) - Player[A].Location.Width;
                                                Player[A].Location.SpeedX = 0;
                                                break;
                                            }
                                        }
                                    }

                                    // arbitrary but fine for here (for now)
                                    // FIXME: look for cases where this is too high
                                    cursed_value_C = numBlock;

                                    /// TODO: find all cases where C is set and could "stick", and make them persistent
                                    //// decided whether or not to care about 2P and higher (C values from NPC checks)

                                    if(tempBool)
                                    {
                                        Player[A].CanJump = false;
                                        Player[A].Jump = 0;
                                        Player[A].Location.X += -4 * Player[A].Direction;
                                        Player[A].Location.Y += -Player[A].Location.SpeedY;
                                        Player[A].Location.SpeedX = 0;
                                        Player[A].Location.SpeedY = 0;

                                        q.update(Player[A].Location, it);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
        }
    }

    if(Player[A].Character == 5 && Player[A].Duck)
    {
        Player[A].Location.Y += Player[A].Location.Height;
        Player[A].Location.Height = Physics.PlayerDuckHeight[Player[A].Character][Player[A].State];
        Player[A].Location.Y += -Player[A].Location.Height;
    }


    // helps the player run down slopes at different angles
    if(Player[A].Slope == 0 && oldSlope > 0 && Player[A].Mount != 1 && Player[A].Mount != 2 && !Player[A].Slide)
    {
        if(Player[A].Location.SpeedY > 0)
        {
            float C = Player[A].Location.SpeedX * (Block[oldSlope].Location.Height / Block[oldSlope].Location.Width) * BlockSlope[Block[oldSlope].Type];
            if(C > 0)
                Player[A].Location.SpeedY = C;
        }
    }

    if(floorBlock > 0) // For walking
    {
        if(Player[A].StandingOnNPC == -A) // fors standing on movable blocks
        {
            if(NPC[Player[A].StandingOnNPC].Special2 != 0)
            {
                Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX;
                movingBlock = false;
                Player[A].StandingOnNPC = 0;
            }
        }

        // diggable dirt
        if(Block[floorBlock].Type == 370 && Player[A].StandingOnNPC <= 0) // dig dirt
        {
            DontResetGrabTime = true;
            // B = floorBlock;
            if(Player[A].TailCount == 0 && Player[A].Controls.Down && Player[A].Controls.Run && Player[A].Mount == 0 && !Player[A].Stoned && Player[A].HoldingNPC == 0 && (Player[A].GrabTime > 0 || Player[A].RunRelease))
            {
                if((Player[A].GrabTime >= 12 && Player[A].Character < 3) || (Player[A].GrabTime >= 16 && Player[A].Character == 3) || (Player[A].GrabTime >= 8 && Player[A].Character == 4))
                {
                    Player[A].Location.SpeedX = Player[A].GrabSpeed;
                    Player[A].GrabSpeed = 0;
                    Block[floorBlock].Hidden = true;
                    Block[floorBlock].Layer = LAYER_DESTROYED_BLOCKS;
                    syncLayersTrees_Block(floorBlock);
                    NewEffect(EFFID_SMOKE_S3, Block[floorBlock].Location);
                    Effect[numEffects].Location.SpeedY = -2;
                    Player[A].GrabTime = 0;
                }
                else
                {
                    if(Player[A].GrabTime == 0)
                    {
                        PlaySoundSpatial(SFX_Grab, Player[A].Location);
                        Player[A].FrameCount = 0;
                        Player[A].GrabSpeed = Player[A].Location.SpeedX;
                    }
                    Player[A].Location.SpeedX = 0;
                    Player[A].Slide = false;
                    Player[A].GrabTime += 1;
                }
            }
            else if(g_config.fix_player_stuck_on_dirt)
                DontResetGrabTime = false;
        }

        if(tempHit2)
        {
            if(WalkingCollision(Player[A].Location, Block[floorBlock].Location))
            {
                Player[A].Location.Y = floorLocation.Y - Player[A].Location.Height;
                if(Player[A].GroundPound)
                {
                    YoshiPound(A, Player[A].Mount, true);
                    Player[A].GroundPound = false;
                }
                else if(Player[A].YoshiYellow)
                    YoshiPound(A, Player[A].Mount);

                Player[A].Location.SpeedY = 0;
                if(floorLocation.SpeedX != 0 || floorLocation.SpeedY != 0)
                {
                    NPC[-A] = NPC_t();
                    NPC[-A].Location = floorLocation;
                    NPC[-A].Type = NPCID_METALBARREL;
                    NPC[-A].Active = true;
                    NPC[-A].TimeLeft = 100;
                    NPC[-A].Section = Player[A].Section;
                    NPC[-A].Special = floorBlock;
                    Player[A].StandingOnNPC = -A;
                    movingBlock = true;
                    Player[A].Location.SpeedY = 12;
                }

                if(Block[floorBlock].Type == 55 && !FreezeNPCs) // Make the player jump if the block is bouncy
                {
                    if(!Player[A].Slide)
                        Player[A].Multiplier = 0;
                    BlockHit(floorBlock, true);
                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                    PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                    if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                    {
                        PlaySoundSpatial(SFX_Jump, Player[A].Location);
                        Player[A].Jump = Physics.PlayerBlockJumpHeight;
                        if(Player[A].Character == 2)
                            Player[A].Jump += 3;
                        if(Player[A].SpinJump)
                            Player[A].Jump -= 6;
                    }
                }

                if(Player[A].SpinJump && (Block[floorBlock].Type == 90 || Block[floorBlock].Type == 526) && Player[A].State > 1 && Block[floorBlock].Special == 0)
                {
                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                    Block[floorBlock].Kill = true;
                    iBlocks += 1;
                    iBlock[iBlocks] = floorBlock;
                    // HitSpot = 0; // this definition is never used
                    floorBlock = 0;
                    Player[A].Jump = 7;

                    if(Player[A].Character == 2)
                        Player[A].Jump += 3;

                    if(Player[A].Controls.Down)
                    {
                        Player[A].Jump = 0;
                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.5;
                    }
                }
            }
        }
        else
        {
            Player[A].Location.Y = floorLocation.Y - Player[A].Location.Height;
            if(Player[A].StandingOnNPC != 0)
            {
                if(NPC[Player[A].StandingOnNPC].Location.Y <= floorLocation.Y && Player[A].StandingOnNPC != Player[A].HoldingNPC)
                    Player[A].Location.Y = NPC[Player[A].StandingOnNPC].Location.Y - Player[A].Location.Height;
            }

            if(Player[A].GroundPound)
            {
                YoshiPound(A, Player[A].Mount, true);
                Player[A].GroundPound = false;
            }
            else if(Player[A].YoshiYellow)
                YoshiPound(A, Player[A].Mount);

            if(Player[A].Slope == 0 || Player[A].Slide)
                Player[A].Location.SpeedY = 0;

            if(floorLocation.SpeedX != 0 || floorLocation.SpeedY != 0)
            {
                NPC[-A] = NPC_t();
                NPC[-A].Location = floorLocation;
                NPC[-A].Type = NPCID_METALBARREL;
                NPC[-A].Active = true;
                NPC[-A].TimeLeft = 100;
                NPC[-A].Section = Player[A].Section;
                NPC[-A].Special = floorBlock;
                Player[A].StandingOnNPC = -A;
                movingBlock = true;
                Player[A].Location.SpeedY = 12;
            }

            if(Player[A].StandingOnNPC != 0 && !movingBlock)
            {
                Player[A].Location.SpeedY = 1;
                // the single Pinched variable has always been false since SMBX64
                // if(!NPC[Player[A].StandingOnNPC].Pinched && !FreezeNPCs)
                if(!FreezeNPCs)
                    Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed;
                Player[A].StandingOnNPC = 0;
            }
            else if(movingBlock)
            {
                Player[A].Location.SpeedY = NPC[-A].Location.SpeedY + 1;
                if(Player[A].Location.SpeedY < 0)
                    Player[A].Location.SpeedY = 0;
            }
            else
            {
                if(Player[A].Slope == 0 || Player[A].Slide)
                    Player[A].Location.SpeedY = 0;
            }

            if(Block[floorBlock].Type == 55 && !FreezeNPCs) // Make the player jump if the block is bouncy
            {
                BlockHit(floorBlock, true);

                if(!Player[A].Slide)
                    Player[A].Multiplier = 0;

                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                PlaySoundSpatial(SFX_BlockHit, Player[A].Location);

                if(Player[A].Controls.Jump || Player[A].Controls.AltJump)
                {
                    PlaySoundSpatial(SFX_Jump, Player[A].Location);
                    Player[A].Jump = Physics.PlayerBlockJumpHeight;

                    if(Player[A].Character == 2)
                        Player[A].Jump += 3;

                    if(Player[A].SpinJump)
                        Player[A].Jump -= 6;
                }
            }

            if(Player[A].SpinJump && (Block[floorBlock].Type == 90 || Block[floorBlock].Type == 526) && Player[A].State > 1 && Block[floorBlock].Special == 0)
            {
                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                Block[floorBlock].Kill = true;
                iBlocks += 1;
                iBlock[iBlocks] = floorBlock;
                floorBlock = 0;
                Player[A].Jump = 7;

                if(Player[A].Character == 2)
                    Player[A].Jump += 3;

                if(Player[A].Controls.Down)
                {
                    Player[A].Jump = 0;
                    Player[A].Location.SpeedY = Physics.PlayerJumpVelocity * 0.5;
                }
            }
        }
    }

    if(tempSlope2 > 0 && tempSlope > 0)
    {
        if(Block[tempSlope].Location.Y + Block[tempSlope].Location.Height == Block[tempSlope2].Location.Y + Block[tempSlope2].Location.Height)
            tempHit2 = false;
    }

    if(!tempHit && tempHit2)
    {
        if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX > 0 && Player[A].Controls.Right)
        {
            Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
            if(blockPushX > 0)
                Player[A].Location.SpeedX += blockPushX;
        }
        else if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX < 0 && Player[A].Controls.Left)
        {
            Player[A].Location.SpeedX = 0.2 * Player[A].Direction;
            if(blockPushX < 0)
                Player[A].Location.SpeedX += blockPushX;
        }
        else
        {
            if(Player[A].Controls.Right || Player[A].Controls.Left)
                Player[A].Location.SpeedX = -NPC[Player[A].StandingOnNPC].Location.SpeedX + 0.2 * Player[A].Direction;
            else
                Player[A].Location.SpeedX = 0;
        }

        if(Player[A].Mount == 2)
            Player[A].Location.SpeedX = 0;
    }

    int ceilingBlock = 0; // was called B

    if(ceilingBlock2 != 0) // Hitting a block from below
    {
        float C = Block[ceilingBlock1].Location.X + Block[ceilingBlock1].Location.Width * 0.5;
        float D = Block[ceilingBlock2].Location.X + Block[ceilingBlock2].Location.Width * 0.5;
        C += -(Player[A].Location.X + Player[A].Location.Width * 0.5);
        D += -(Player[A].Location.X + Player[A].Location.Width * 0.5);

        if(C < 0)
            C = -C;

        if(D < 0)
            D = -D;

        if(C < D)
            ceilingBlock = ceilingBlock1;
        else
            ceilingBlock = ceilingBlock2;
    }
    else if(ceilingBlock1 != 0)
    {
        ceilingBlock = ceilingBlock1;
        if(Block[ceilingBlock].Location.X + Block[ceilingBlock].Location.Width - Player[A].Location.X <= 4)
        {
            Player[A].Location.X = Block[ceilingBlock].Location.X + Block[ceilingBlock].Location.Width + 0.1;
            ceilingBlock = 0;
        }
        else if(Player[A].Location.X + Player[A].Location.Width - Block[ceilingBlock].Location.X <= 4)
        {
            Player[A].Location.X = Block[ceilingBlock].Location.X - Player[A].Location.Width - 0.1;
            ceilingBlock = 0;
        }
    }

    if(ceilingBlock > 0)
    {
        PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
        Player[A].Jump = 0;
        Player[A].Location.Y = Block[ceilingBlock].Location.Y + Block[ceilingBlock].Location.Height + 0.01;
        Player[A].Location.SpeedY = -0.01 + Block[ceilingBlock].Location.SpeedY;

        if(Player[A].Fairy)
            Player[A].Location.SpeedY = 2;

        if(Player[A].Vine > 0)
            Player[A].Location.Y += 0.1;

        if(Player[A].Mount == 2)
            Player[A].Location.SpeedY = 2;

        if(Player[A].CanFly2)
            Player[A].Location.SpeedY = 2;

        if(Player[A].Mount != 2) // Tell the block it was hit
            BlockHit(ceilingBlock, false, A);

        if(Block[ceilingBlock].Type == 55) // If it is a bouncy block the knock the player down
            Player[A].Location.SpeedY = 3;

        if(Player[A].State > 1 && Player[A].Character != 5) // If the player was big ask the block nicely to die
        {
            if(Player[A].Mount != 2 && Block[ceilingBlock].Type != 293)
                BlockHitHard(ceilingBlock);
        }
    }

    if(Player[A].StandingOnNPC != 0)
    {
        if(!tempHit2)
        {
            // the single Pinched variable has always been false since SMBX64
            // if(!NPC[Player[A].StandingOnNPC].Pinched && !FreezeNPCs)
            if(!FreezeNPCs)
                Player[A].Location.SpeedX += -NPC[Player[A].StandingOnNPC].Location.SpeedX - NPC[Player[A].StandingOnNPC].BeltSpeed;
        }
    }

    if(Player[A].Slide && oldSlope > 0 && Player[A].Slope == 0 && Player[A].Location.SpeedY < 0)
    {
        if(Player[A].NoGravity == 0)
        {
            // Player[A].NoGravity = static_cast<int>(floor(static_cast<double>(Player[A].Location.SpeedY / Physics.PlayerJumpVelocity * 8)));
            // PlayerJumpVelocity is -5.7, SpeedY is negative
            Player[A].NoGravity = (int)(Player[A].Location.SpeedY * -80) / 57;
        }
    }
    else if(Player[A].Slope > 0 || oldSlope > 0 || !Player[A].Slide)
        Player[A].NoGravity = 0;

    // if(Player[A].Slide)  // Simplified below
    // {
    //     if(Player[A].Location.SpeedX > 1 || Player[A].Location.SpeedX < -1)
    //         Player[A].SlideKill = true;
    //     else
    //         Player[A].SlideKill = false;
    // }
    // else
    //     Player[A].SlideKill = false;

    Player[A].SlideKill = Player[A].Slide && (std::abs(Player[A].Location.SpeedX) > 1);
}
