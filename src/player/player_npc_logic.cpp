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
#include "npc_id.h"
#include "npc_traits.h"
#include "npc/npc_queues.h"
#include "sound.h"
#include "editor.h"
#include "effect.h"
#include "game_main.h"
#include "eff_id.h"
#include "collision.h"
#include "layers.h"
#include "config.h"
#include "phys_env.h"

#include "main/trees.h"

void PlayerNPCLogic(int A, bool& tempSpring, bool& tempShell, int& MessageNPC, const bool movingBlock, const int floorBlock, const tempf_t oldSpeedY)
{
    int floorNpc1 = 0;
    int floorNpc2 = 0;
    // was previously a float
    num_t tempHitSpeed = 0;
    bool spinKill = false;

    // cleanup variables for NPC collisions

    int tempHit = 0; // Used for JUMP detection -- new: it's the ID of the last NPC that was jumped on
    bool tempHit2 = false;
    Location_t tempLocation;

    for(int B : treeNPCQuery(Player[A].Location, SORTMODE_ID))
    {
        if(NPC[B].Active && NPC[B].Killed == 0 && NPC[B].Effect != NPCEFF_PET_TONGUE && NPC[B].Effect != NPCEFF_PET_INSIDE)
        {
            // If Not (NPC(B).Type = 17 And NPC(B).CantHurt > 0) And Not (.Mount = 2 And NPC(B).Type = 56) And Not NPC(B).vehiclePlr = A And Not NPC(B).Type = 197 And Not NPC(B).Type = 237 Then
            if(!(Player[A].Mount == 2 && NPC[B].Type == NPCID_VEHICLE) &&
                NPC[B].vehiclePlr != A &&
                NPC[B].Type != NPCID_GOALTAPE &&
                NPC[B].Type != NPCID_ICE_BLOCK
            )
            {
                if(NPC[B].HoldingPlayer == 0 || NPC[B]->IsABonus || (BattleMode && NPC[B].HoldingPlayer != A))
                {
                    if(CheckCollision(Player[A].Location, NPC[B].Location))
                    {
                        if((NPC[B].Type == NPCID_METALBARREL || NPC[B].Type == NPCID_CANNONENEMY || NPC[B].Type == NPCID_HPIPE_SHORT || NPC[B].Type == NPCID_HPIPE_LONG || NPC[B].Type == NPCID_VPIPE_SHORT || NPC[B].Type == NPCID_VPIPE_LONG) && NPC[B].Projectile)
                            PlayerHurt(A);

                        // the hitspot is used for collision detection to find out where to put the player after it collides with a block
                        // the numbers tell what side the collision happened so it can move the plaer to the correct position
                        // 1 means the player hit the block from the top
                        // 2 is from the right
                        // 3 is from the bottom
                        // 4 is from the left
                        int HitSpot;
                        if((Player[A].Mount == 1 || Player[A].Mount == 3 || Player[A].SpinJump || (Player[A].ShellSurf && NPC[B]->IsAShell) || (Player[A].Stoned && !NPC[B]->CanWalkOn)) && !NPC[B]->MovesPlayer)
                            HitSpot = BootCollision(Player[A].Location, NPC[B].Location, NPC[B]->CanWalkOn); // find the hitspot for normal mario
                        else
                            HitSpot = EasyModeCollision(Player[A].Location, NPC[B].Location, NPC[B]->CanWalkOn); // find the hitspot when in a shoe or on a yoshi

                        if(GameOutro)
                            HitSpot = 0;

                        if(NPC[B].Inert) // if the npc is friendly then you can't touch it
                        {
                            HitSpot = 0;
                            if(NPC[B].Text != STRINGINDEX_NONE && Player[A].Controls.Up && !FreezeNPCs)
                                MessageNPC = B;
                        }

                        // BEGIN type-based "onCollidePlayer" logic

                        if(!NPC[B].Inert)
                        {
                            // battlemode stuff
                            if(NPC[B].Type == NPCID_PLR_FIREBALL || NPC[B].Type == NPCID_PLR_HEAVY || NPC[B].Type == NPCID_PLR_ICEBALL || NPC[B].Type == NPCID_SWORDBEAM || NPC[B].Type == NPCID_PET_FIRE || NPC[B].Type == NPCID_CHAR3_HEAVY || NPC[B].Type == NPCID_CHAR4_HEAVY)
                            {
                                if(BattleMode && NPC[B].CantHurtPlayer != A)
                                {
                                    // duck projectile-resistance in heavy suit
                                    if(Player[A].State == 6 && Player[A].Duck && Player[A].Character != 5)
                                        NPCHit(B, 3, B);
                                    else
                                    {
                                        if(Player[A].Immune == 0)
                                        {
                                            NPCHit(B, 3, B);
                                            if(NPC[B].Type == NPCID_SWORDBEAM)
                                                PlaySoundSpatial(SFX_HeroHit, Player[A].Location);
                                        }
                                        PlayerHurt(A);
                                    }
                                }
                                HitSpot = 0;
                            }

                            if(NPC[B].Type == NPCID_BULLET && NPC[B].CantHurt > 0)
                            {
                                if(!BattleMode)
                                    HitSpot = 0;
                                else if(NPC[B].CantHurtPlayer != A)
                                {
                                    if(HitSpot != 1)
                                        PlayerHurt(A);
                                    else
                                    {
                                        NPC[B].CantHurt = 0;
                                        NPC[B].CantHurtPlayer = 0;
                                        NPC[B].Projectile = false;
                                    }
                                }
                            }

                            if((NPC[B].Type == NPCID_TOOTHY || NPC[B].Type == NPCID_HEAVY_THROWN) && BattleMode && NPC[B].CantHurtPlayer != A)
                                PlayerHurt(A);

                            if((NPC[B].Type == NPCID_ICE_CUBE || NPC[B].Type == NPCID_ITEM_POD) && BattleMode &&
                                NPC[B].CantHurtPlayer != A && NPC[B].Projectile != 0 && NPC[B].BattleOwner != A)
                            {
                                if(Player[A].Immune == 0 && NPC[B].Type == NPCID_ITEM_POD)
                                    NPC[B].Special2 = 1;

                                PlayerHurt(A);
                                HitSpot = 0;
                            }

                            if((NPC[B]->IsAShell || NPCIsVeggie(NPC[B]) ||
                                NPC[B].Type == NPCID_ICE_CUBE || NPC[B].Type == NPCID_SLIDE_BLOCK) &&
                                BattleMode && NPC[B].HoldingPlayer > 0 && NPC[B].HoldingPlayer != A)
                            {
                                if(Player[A].Immune == 0)
                                {
                                    PlayerHurt(A);
                                    NPCHit(B, 5, B);
                                }
                            }

                            if(NPCIsAParaTroopa(NPC[B]) && BattleMode && NPC[B].CantHurtPlayer == A)
                                HitSpot = 0;

                            if(BattleMode && NPCIsVeggie(NPC[B]) && NPC[B].Projectile != 0)
                            {
                                if(NPC[B].CantHurtPlayer != A)
                                {
                                    if(Player[A].Immune == 0)
                                    {
                                        PlayerHurt(A);
                                        NPCHit(B, 4, B);
                                        PlaySoundSpatial(SFX_SpitBossHit, Player[A].Location);
                                    }
                                }
                            }

                            if(BattleMode && NPC[B].HoldingPlayer > 0 && NPC[B].HoldingPlayer != A)
                            {
                                if(NPC[B]->WontHurt)
                                    HitSpot = 0;
                                else
                                    HitSpot = 5;
                            }

                            if(BattleMode && NPC[B].BattleOwner != A && NPC[B].Projectile != 0 && NPC[B].CantHurtPlayer != A)
                            {
                                if(NPC[B].Type == NPCID_BOMB || NPC[B].Type == NPCID_LIT_BOMB_S3 || NPC[B].Type == NPCID_CARRY_BLOCK_A || NPC[B].Type == NPCID_CARRY_BLOCK_B || NPC[B].Type == NPCID_CARRY_BLOCK_C || NPC[B].Type == NPCID_CARRY_BLOCK_D || NPC[B].Type == NPCID_HIT_CARRY_FODDER || ((NPC[B]->IsAShell || NPC[B].Type == NPCID_SLIDE_BLOCK) && NPC[B].Location.SpeedX == 0))
                                {
                                    if(NPC[B]->IsAShell && HitSpot == 1 && Player[A].SpinJump)
                                    {}
                                    else if(Player[A].Immune == 0)
                                    {
                                        if(NPC[B].Type != NPCID_SLIDE_BLOCK && !NPC[B]->IsAShell)
                                            NPCHit(B, 3, B);
                                        PlayerHurt(A);
                                        HitSpot = 0;
                                    }
                                }
                            }
                            // end battlemode
                        }

                        if(NPC[B].Type == NPCID_ITEM_BUBBLE)
                        {
                            NPCHit(B, 1, A);
                            HitSpot = 0;
                        }

                        if(NPC[B].Type == NPCID_HEAVY_THROWN && NPC[B].CantHurt > 0)
                            HitSpot = 0;

                        if(NPC[B].Type == NPCID_ITEM_POD && HitSpot == 1)
                            HitSpot = 0;

                        // ' Fireball immune for ducking in the hammer suit
                        if((Player[A].State == 6 && Player[A].Duck && Player[A].Mount == 0 && Player[A].Character != 5) || (Player[A].Mount == 1 && Player[A].MountType == 2))
                        {
                            if(NPC[B].Type == NPCID_STATUE_FIRE || NPC[B].Type == NPCID_VILLAIN_FIRE || NPC[B].Type == NPCID_PLANT_FIREBALL || NPC[B].Type == NPCID_QUAD_BALL)
                            {
                                PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                HitSpot = 0;
                                NPC[B].Killed = 9;
                                NPCQueues::Killed.push_back(B);
                                for(int C = 1; C <= 10; ++C)
                                {
                                    NewEffect(EFFID_PLR_FIREBALL_TRAIL, NPC[B].Location, NPC[B].Special);
                                    Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5_n + NPC[B].Location.SpeedX / 10;
                                    Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5_n - NPC[B].Location.SpeedY / 10;
                                    if(Effect[numEffects].Frame == 0)
                                        Effect[numEffects].Frame = -iRand(3);
                                    else
                                        Effect[numEffects].Frame = 5 + iRand(3);
                                }
                                NPC[B].Location.X += NPC[B].Location.Width / 2 - EffectWidth[EFFID_SMOKE_S3] * 0.5_n;
                                NPC[B].Location.Y += NPC[B].Location.Height / 2 - EffectHeight[EFFID_SMOKE_S3] * 0.5_n;
                                NewEffect(EFFID_SMOKE_S3, NPC[B].Location);

                                treeNPCUpdate(B);
                            }
                        }


                        if(NPC[B]->IsAVine) // if the player collided with a vine then see if he should climb it
                        {
                            if(Player[A].Character == 5)
                            {
                                bool hasNoMonts = (g_config.fix_char5_vehicle_climb && Player[A].Mount <= 0) ||
                                                   !g_config.fix_char5_vehicle_climb;
                                if(hasNoMonts && Player[A].Immune == 0 && Player[A].Controls.Up)
                                {
                                    Player[A].FairyCD = 0;

                                    if(!Player[A].Fairy)
                                    {
                                        Player[A].Fairy = true;
                                        SizeCheck(Player[A]);
                                        PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                                        Player[A].Immune = 10;
                                        Player[A].Effect = PLREFF_WAITING;
                                        Player[A].Effect2 = 4;
                                        NewEffect(EFFID_SMOKE_S5, Player[A].Location);
                                    }

                                    if(Player[A].FairyTime != -1 && Player[A].FairyTime < 20)
                                        Player[A].FairyTime = 20;
                                }
                            }
                            else if(!Player[A].Fairy && !Player[A].Stoned && !Player[A].AquaticSwim)
                            {
                                if(Player[A].Mount == 0 && Player[A].HoldingNPC <= 0)
                                {
                                    if(Player[A].Vine > 0)
                                    {
                                        if(Player[A].Duck)
                                            UnDuck(Player[A]);

                                        if(Player[A].Location.Y >= NPC[B].Location.Y - 20 && Player[A].Vine < 2)
                                            Player[A].Vine = 2;

                                        if(Player[A].Location.Y >= NPC[B].Location.Y - 18)
                                            Player[A].Vine = 3;
                                    }
                                    else if((Player[A].Controls.Up ||
                                             (Player[A].Controls.Down &&
                                              !num_t::fEqual_d(Player[A].Location.SpeedY, 0) && // Not .Location.SpeedY = 0
                                              Player[A].StandingOnNPC == 0 && // Not .StandingOnNPC <> 0
                                              Player[A].Slope <= 0) // Not .Slope > 0
                                            ) && Player[A].Jump == 0)
                                    {
                                        if(Player[A].Duck)
                                            UnDuck(Player[A]);

                                        if(Player[A].Location.Y >= NPC[B].Location.Y - 20 && Player[A].Vine < 2)
                                            Player[A].Vine = 2;

                                        if(Player[A].Location.Y >= NPC[B].Location.Y - 18)
                                            Player[A].Vine = 3;
                                    }

                                    if(Player[A].Vine > 0)
                                    {
                                        Player[A].VineNPC = B;
                                        if(g_config.fix_climb_bgo_speed_adding)
                                            Player[A].VineBGO = 0;
                                    }
                                }
                            }
                        }

                        // subcon warps
                        if(NPC[B].Type == NPCID_MAGIC_DOOR && HitSpot > 0 && Player[A].Controls.Up)
                        {
                            if(NPC[B].Special2 >= 0)
                            {
                                NPC[B].Killed = 9;
                                NPCQueues::Killed.push_back(B);
                                PlaySoundSpatial(SFX_Door, Player[A].Location);
                                Player[A].Effect = PLREFF_WARP_DOOR;
                                Player[A].Warp = numWarps + 1;
                                Player[A].WarpBackward = false;
                                Warp[numWarps + 1].Entrance = static_cast<SpeedlessLocation_t>(NPC[B].Location);
                                tempLocation = NPC[B].Location;
                                tempLocation.X = NPC[B].Location.X - level[Player[A].Section].X + level[NPC[B].Special2].X;
                                tempLocation.Y = NPC[B].Location.Y - level[Player[A].Section].Y + level[NPC[B].Special2].Y;
                                Warp[numWarps + 1].Exit = static_cast<SpeedlessLocation_t>(tempLocation);
                                Warp[numWarps + 1].Hidden = false;
                                Warp[numWarps + 1].NoYoshi = false;
                                Warp[numWarps + 1].WarpNPC = true;
                                Warp[numWarps + 1].Locked = false;
                                Warp[numWarps + 1].Stars = 0;
                                Player[A].Location.SpeedX = 0;
                                Player[A].Location.SpeedY = 0;
                                // Stop
                                Player[A].Location.X = Warp[Player[A].Warp].Entrance.X + (Warp[Player[A].Warp].Entrance.Width - Player[A].Location.Width) / 2;
                                Player[A].Location.Y = Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height - Player[A].Location.Height;
                                tempLocation = static_cast<Location_t>(Warp[numWarps + 1].Entrance);
                                tempLocation.Y -= 32;
                                tempLocation.Height = 64;
                                NewEffect(EFFID_DOOR_S2_OPEN, tempLocation);
                                tempLocation = static_cast<Location_t>(Warp[numWarps + 1].Exit);
                                tempLocation.Y -= 32;
                                tempLocation.Height = 64;
                                NewEffect(EFFID_DOOR_S2_OPEN, tempLocation);
                            }
                        }

                        if(NPC[B].Type == NPCID_LOCK_DOOR && Player[A].HasKey)
                        {
                            Player[A].HasKey = false;
                            HitSpot = 0;
                            NPC[B].Killed = 3;
                            NPCQueues::Killed.push_back(B);
                        }

                        if(NPC[B].Type == NPCID_MINIBOSS && NPC[B].Special == 4 && HitSpot > 1)
                            HitSpot = 0;

                        // END type-based "onCollidePlayer" logic

                        if(Player[A].Stoned && HitSpot != 1) // if you are a statue then SLAM into the npc
                        {
                            if(Player[A].Location.SpeedX > 3 || Player[A].Location.SpeedX < -3)
                                NPCHit(B, 3, B);
                        }

                        // kill things with the vehicle
                        if(Player[A].Mount == 2 && !Player[A].SpinJump && !(Player[A].Stoned && !NPC[B]->CanWalkOn))
                        {
                            // already checked at top
                            // if(NPC[B].vehiclePlr == A)
                            //     HitSpot = 0;
                            // else
                            if(!(NPC[B].Type == NPCID_BULLET && NPC[B].CantHurt > 0))
                            {
                                if((NPC[B].Location.Y + NPC[B].Location.Height > Player[A].Location.Y + 18 && HitSpot != 3) || HitSpot == 1)
                                {
                                    NPCHit(B, 8, A);
                                    if(NPC[B].Killed == 8)
                                        HitSpot = 0;

                                    if(NPC[B].Type == NPCID_WALK_BOMB_S2 || NPC[B].Type == NPCID_WALK_BOMB_S3 || NPC[B].Type == NPCID_LIT_BOMB_S3)
                                    {
                                        NPCHit(B, 3, B);
                                        if(NPC[B].Killed == 3)
                                            HitSpot = 0;
                                    }
                                }
                            }
                        }

                        // prevented player from grabbing active slide blocks; logic moved to side grab code
                        // if(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Projectile != 0 && HitSpot > 1)
                        //     HitSpot = 5;

                        if(HitSpot == 1) // Player landed on a NPC
                        {
                            // the following code is for spin jumping and landing on things as yoshi/shoe
                            if(!InvincibilityTime && (Player[A].Mount == 1 || Player[A].Mount == 3 || Player[A].SpinJump || (Player[A].Stoned && !NPC[B]->CanWalkOn)))
                            {
                                // these types were not hit during spin bounces in SMBX 1.3 (but may be vulnerable to stomps by mounts)
                                bool dont_hit_spin_bounce = (NPC[B].Type == NPCID_FIRE_PLANT || NPC[B].Type == NPCID_QUAD_SPITTER || NPC[B].Type == NPCID_PLANT_S3 || NPC[B].Type == NPCID_LAVABUBBLE ||
                                          NPC[B].Type == NPCID_SPIKY_S3 || NPC[B].Type == NPCID_SPIKY_S4 || NPC[B].Type == NPCID_SPIKY_BALL_S4 || NPC[B].Type == NPCID_BOTTOM_PLANT ||
                                          NPC[B].Type == NPCID_SIDE_PLANT || NPC[B].Type == NPCID_CRAB || NPC[B].Type == NPCID_FLY || NPC[B].Type == NPCID_BIG_PLANT ||
                                          NPC[B].Type == NPCID_PLANT_S1 || NPC[B].Type == NPCID_VILLAIN_S1 || NPC[B].Type == NPCID_WALL_BUG || NPC[B].Type == NPCID_WALL_TURTLE ||
                                          NPC[B].Type == NPCID_SICK_BOSS || NPC[B].Type == NPCID_WALK_PLANT || NPC[B].Type == NPCID_JUMP_PLANT);

                                // these types are immune to spin bounces and were hit (with no effect) in SMBX 1.3 but are no longer hit
                                bool immune_to_spin_bounce = (NPC[B].Type == NPCID_SAW || NPC[B].Type == NPCID_STONE_S3 || NPC[B].Type == NPCID_STONE_S4 || NPC[B].Type == NPCID_GHOST_S3 ||
                                   NPC[B].Type == NPCID_GHOST_FAST || NPC[B].Type == NPCID_GHOST_S4 || NPC[B].Type == NPCID_BIG_GHOST || NPC[B].Type == NPCID_LAVA_MONSTER || NPC[B].Type == NPCID_LONG_PLANT_UP);

                                bool force_bounce = (dont_hit_spin_bounce | immune_to_spin_bounce);

                                if(Player[A].Mount == 1 || Player[A].Mount == 2 || Player[A].Stoned)
                                    NPCHit(B, 8, A);
                                else if(!force_bounce && !NPC[B]->CanWalkOn)
                                {
                                    if(Player[A].Wet > 0 && (NPC[B]->IsFish || NPC[B].Type == NPCID_SQUID_S3 || NPC[B].Type == NPCID_SQUID_S1))
                                    {
                                    }
                                    else
                                        NPCHit(B, 8, A);
                                }

                                if(NPC[B].Killed == 8 || NPC[B]->IsFish || force_bounce) // tap
                                {
                                    if(NPC[B].Killed == 8 && Player[A].Mount == 1 && Player[A].MountType == 2)
                                    {
                                        for(int i = 0; i < 2; i++)
                                        {
                                            numNPCs++;
                                            NPC[numNPCs] = NPC_t();
                                            NPC[numNPCs].Active = true;
                                            NPC[numNPCs].TimeLeft = 100;
                                            NPC[numNPCs].Section = Player[A].Section;
                                            NPC[numNPCs].Type = NPCID_PLR_FIREBALL;
                                            NPC[numNPCs].Special = Player[A].Character;
                                            NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                                            NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                                            NPC[numNPCs].Location.Y = Player[A].Location.Height + Player[A].Location.Y - NPC[numNPCs].Location.Height;
                                            NPC[numNPCs].Location.X = Player[A].Location.X + (Player[A].Location.Width - NPC[numNPCs].Location.Width) / 2;
                                            NPC[numNPCs].Location.SpeedX = (i == 0) ? -4 : 4;
                                            NPC[numNPCs].Location.SpeedY = 10;
                                            syncLayers_NPC(numNPCs);
                                        }
                                    }

                                    Player[A].ForceHitSpot3 = true;
                                    if(/*HitSpot == 1 && */ !(Player[A].GroundPound && NPC[B].Killed == 8))
                                    {
                                        tempHit = B;
                                        tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;

                                        if(NPC[B].Killed == 0)
                                            PlaySoundSpatial(SFX_Stomp, Player[A].Location);
                                        else if(Player[A].SpinJump)
                                        {
                                            if(Player[A].Controls.Down)
                                                tempHit = 0;
                                            else
                                                spinKill = true;
                                        }
                                    }

                                    continue;
                                }
                            }

                            // moved from above
                            if(/* HitSpot == 1 && */ (NPC[B].Type == NPCID_COIN_SWITCH || NPC[B].Type == NPCID_TIME_SWITCH || NPC[B].Type == NPCID_TNT) && NPC[B].Projectile != 0)
                                HitSpot = 0;
                            // NPCs that can be walked on
                            else if(NPC[B]->CanWalkOn || (Player[A].ShellSurf && NPC[B]->IsAShell))
                            {
                                // the player landed on an NPC he can stand on
                                if(floorNpc1 == 0)
                                    floorNpc1 = B;
                                else if(floorNpc2 == 0)
                                    floorNpc2 = B;
                                else if(Player[A].StandingOnNPC == B)
                                {
                                    // if standing on 2 or more NPCs find out the best one to stand on
                                    num_t C = num_t::abs(NPC[floorNpc1].Location.minus_center_x(Player[A].Location));
                                    num_t D = num_t::abs(NPC[floorNpc2].Location.minus_center_x(Player[A].Location));

                                    if(C < D)
                                        floorNpc2 = B;
                                    else
                                        floorNpc1 = B;
                                }
                                else
                                    floorNpc2 = B;
                            }
                            // if landing on a yoshi or boot, mount up!
                            else if((NPCIsYoshi(NPC[B]) || NPCIsBoot(NPC[B])) && Player[A].Character != 5 && !Player[A].Fairy)
                            {
                                if(Player[A].Mount == 0 && NPC[B].CantHurtPlayer != A && Player[A].Dismount == 0)
                                {
                                    if(NPCIsBoot(NPC[B]))
                                    {
                                        UnDuck(Player[A]);
                                        NPC[B].Killed = 9;
                                        NPCQueues::Killed.push_back(B);

                                        if(Player[A].State == 1)
                                        {
                                            Player[A].Location.Height = Physics.PlayerHeight[1][2];
                                            Player[A].Location.Y += -Physics.PlayerHeight[1][2] + Physics.PlayerHeight[Player[A].Character][1];
                                        }

                                        Player[A].Mount = 1;

                                        if(NPC[B].Type == NPCID_GRN_BOOT)
                                            Player[A].MountType = 1;

                                        if(NPC[B].Type == NPCID_RED_BOOT)
                                            Player[A].MountType = 2;

                                        if(NPC[B].Type == NPCID_BLU_BOOT)
                                            Player[A].MountType = 3;

                                        PlaySoundSpatial(SFX_Stomp, Player[A].Location);
                                    }
                                    else if(NPCIsYoshi(NPC[B]) && (Player[A].Character == 1 || Player[A].Character == 2))
                                    {
                                        UnDuck(Player[A]);
                                        NPC[B].Killed = 9;
                                        NPCQueues::Killed.push_back(B);
                                        Player[A].Mount = 3;

                                        if(NPC[B].Type == NPCID_PET_GREEN)
                                            Player[A].MountType = 1;
                                        else if(NPC[B].Type == NPCID_PET_BLUE)
                                            Player[A].MountType = 2;
                                        else if(NPC[B].Type == NPCID_PET_YELLOW)
                                            Player[A].MountType = 3;
                                        else if(NPC[B].Type == NPCID_PET_RED)
                                            Player[A].MountType = 4;
                                        else if(NPC[B].Type == NPCID_PET_BLACK)
                                            Player[A].MountType = 5;
                                        else if(NPC[B].Type == NPCID_PET_PURPLE)
                                            Player[A].MountType = 6;
                                        else if(NPC[B].Type == NPCID_PET_PINK)
                                            Player[A].MountType = 7;
                                        else if(NPC[B].Type == NPCID_PET_CYAN)
                                            Player[A].MountType = 8;

                                        Player[A].YoshiNPC = 0;
                                        Player[A].YoshiPlayer = 0;
                                        Player[A].MountSpecial = 0;
                                        Player[A].YoshiTonugeBool = false;
                                        Player[A].YoshiTongueLength = 0;
                                        PlaySoundSpatial(SFX_Pet, Player[A].Location);
                                        UpdateYoshiMusic();
                                        YoshiHeight(A);
                                    }
                                }
                            }
                            else if(NPC[B].Type == NPCID_CANNONITEM || NPC[B].Type == NPCID_KEY ||
                                    NPC[B].Type == NPCID_TOOTHYPIPE || NPC[B].Type == NPCID_TOOTHY ||
                                    ((Player[A].SlideKill || InvincibilityTime) && !NPC[B]->WontHurt)) // NPCs that cannot be walked on
                            {
                                // cancel jump
                            }
#if 0
                            // dead code since SMBX 1.3, because NoShellKick was never set
                            else if(NPC[B].CantHurtPlayer == A && Player[A].NoShellKick > 0)
                            {
                                // Do nothing!
                            }
#endif
                            else
                            {
                                if(NPC[B]->IsABonus) // Bonus
                                    TouchBonus(A, B);
                                else if(Player[A].Rolling && Player[A].State == PLR_STATE_SHELL)
                                {
                                    if(NPC[B].Type == NPCID_SPRING)
                                    {
                                        tempSpring = true;
                                        tempHit = B;
                                        tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;
                                    }
                                    else if(NPC[B].CantHurtPlayer != A)
                                        NPCHit(B, 3, B);

                                    continue;
                                }
                                else if(NPC[B]->IsAShell && NPC[B].Location.SpeedX == 0 && Player[A].HoldingNPC == 0 && Player[A].Controls.Run && !g_config.no_shell_grab_top)
                                {
                                    // grab turtle shells
                                    //if(nPlay.Online == false || nPlay.MySlot + 1 == A)
                                    {
                                        if(Player[A].Character >= 3)
                                            PlaySoundSpatial(SFX_Grab, Player[A].Location);
                                        else
                                            UnDuck(Player[A]);

                                        Player[A].HoldingNPC = B;
                                        NPC[B].HoldingPlayer = A;
                                        NPC[B].CantHurt = Physics.NPCCanHurtWait;
                                        NPC[B].CantHurtPlayer = A;
                                    }

                                }
                                else if(NPC[B]->JumpHurt || (NPC[B]->IsFish && Player[A].WetFrame)) // NPCs that cause damage even when jumped on
                                {
                                    if(!(NPC[B].Type == NPCID_PLANT_S3 && NPC[B].Special2 == 4) && !NPC[B]->WontHurt && NPC[B].CantHurtPlayer != A)
                                    {

                                        // the n00bcollision function reduces the size of the npc's hit box before it damages the player
                                        if(n00bCollision(Player[A].Location, NPC[B].Location))
                                            PlayerHurt(A);
                                    }
                                }
                                else if(NPC[B].Type == NPCID_MINIBOSS) // Special code for BOOM BOOM
                                {
                                    if(NPC[B].Special == 0 || Player[A].Mount == 1 || Player[A].Mount == 3)
                                    {
                                        if(NPC[B].Special != 0)
                                            PlaySoundSpatial(SFX_Stomp, Player[A].Location);
                                        tempHit = B;
                                        tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;
                                    }
                                    else if(NPC[B].Special != 4)
                                    {
                                        if(n00bCollision(Player[A].Location, NPC[B].Location))
                                            PlayerHurt(A);
                                    }
                                }
                                else if((NPC[B].Type == NPCID_LIT_BOMB_S3) || NPC[B].Type == NPCID_HIT_CARRY_FODDER)
                                    NPCHit(B, 1, A); // NPC 'B' was jumped on '1' by player 'A'
                                else if(NPC[B].Killed != 10 && !NPCIsBoot(NPC[B]) && !NPCIsYoshi(NPC[B]) && !(NPC[B]->IsAShell && NPC[B].CantHurtPlayer == A)) // Bounce off everything except Bonus and Piranha Plants
                                {
                                    if(NPC[B].Type == NPCID_SPRING)
                                        tempSpring = true;

                                    if(NPC[B]->IsAShell && NPC[B].Location.SpeedX == 0 && NPC[B].Location.SpeedY == 0)
                                        tempShell = true;

                                    tempHit = B;
                                    tempLocation.Y = NPC[B].Location.Y - Player[A].Location.Height;

                                    if(NPC[B].Type == NPCID_COIN_SWITCH || NPC[B].Type == NPCID_TIME_SWITCH || NPC[B].Type == NPCID_TNT)
                                    {
                                        tempHit = 0;
                                        Player[A].Jump = false;
                                        Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
                                        Player[A].Location.SpeedY = -Physics.PlayerGravity;
                                    }
                                }

                                // If Not (.WetFrame = True And (NPC(B).Type = 229 Or NPC(B).Type = 230) Or NPCIsAVine(NPC(B).Type)) And .HoldingNPC <> B Then
                                if(
                                    !(
                                        (Player[A].WetFrame && (NPC[B].Type == NPCID_GRN_FISH_S3 || NPC[B].Type == NPCID_RED_FISH_S3)) ||
                                        NPC[B]->IsAVine
                                    ) && (Player[A].HoldingNPC != B)
                                )
                                {
                                    if(Player[A].Vine > 0)
                                    {
                                        Player[A].Vine = 0;
                                        Player[A].Jump = 1;
                                    }

                                    if(!(NPC[B]->IsAShell && NPC[B].CantHurtPlayer == A))
                                        NPCHit(B, 1, A); // NPC 'B' was jumped on '1' by player 'A'
                                }
                            }
                        }
                        else if(HitSpot == 0)
                        {
                            // if hitspot = 0 then do nothing
                        }
                        // player touched an npc anywhere except from the top
                        else // Player touched an NPC
                        {

/* If (.CanGrabNPCs = True Or NPCIsGrabbable(NPC(B).Type) = True Or (NPC(B).Effect = 2 And NPCIsABonus(NPC(B).Type) = False)) And (NPC(B).Effect = 0 Or NPC(B).Effect = 2) Or (NPCIsAShell(NPC(B).Type) And FreezeNPCs = True) Then      'GRAB EVERYTHING
*/

                            // grab from side
                            if(
                                ((Player[A].CanGrabNPCs || NPC[B]->IsGrabbable || (NPC[B].Effect == NPCEFF_DROP_ITEM && !NPC[B]->IsABonus)) && (NPC[B].Effect == NPCEFF_NORMAL || NPC[B].Effect == NPCEFF_DROP_ITEM || NPC[B].Effect == NPCEFF_MAZE)) ||
                                 (NPC[B]->IsAShell && FreezeNPCs)
                            ) // GRAB EVERYTHING
                            {
                                if(Player[A].Controls.Run && !Player[A].Rolling)
                                {
                                    if((HitSpot == 2 && Player[A].Direction == -1) ||
                                       (HitSpot == 4 && Player[A].Direction == 1) ||
                                       (NPC[B].Type == NPCID_CANNONITEM || NPC[B].Type == NPCID_TOOTHYPIPE || NPC[B].Effect == NPCEFF_DROP_ITEM || (NPCIsVeggie(NPC[B]) && NPC[B].CantHurtPlayer != A)))
                                    {
                                        if(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Projectile != 0)
                                        {
                                            // don't grab active slide blocks
                                        }
                                        else if(Player[A].HoldingNPC == 0)
                                        {
                                            if(!NPC[B]->IsAShell || Player[A].Character >= 3)
                                            {
                                                if(NPCIsVeggie(NPC[B]))
                                                    PlaySoundSpatial(SFX_Grab2, Player[A].Location);
                                                else
                                                    PlaySoundSpatial(SFX_Grab, Player[A].Location);
                                            }

                                            if(Player[A].Character <= 2)
                                                UnDuck(Player[A]);
                                            Player[A].HoldingNPC = B;
                                            NPC[B].Direction = Player[A].Direction;
                                            NPC[B].Frame = EditorNPCFrame(NPC[B].Type, NPC[B].Direction);
                                            NPC[B].HoldingPlayer = A;
                                            NPC[B].CantHurt = Physics.NPCCanHurtWait;
                                            NPC[B].CantHurtPlayer = A;
                                        }
                                    }
                                }
                            }

                            if(NPC[B]->IsAShell || (NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Special == 1)) // Turtle shell
                            {
                                if(Player[A].Rolling && NPC[B].HoldingPlayer == 0 && NPC[B].CantHurtPlayer != A) // Kill the shell!
                                    NPCHit(B, 3, B);
                                else if(NPC[B].Location.SpeedX == 0 && NPC[B].Location.SpeedY >= 0) // Shell is not moving
                                {
                                    if(((Player[A].Controls.Run && Player[A].HoldingNPC == 0) || Player[A].HoldingNPC == B) && NPC[B].CantHurtPlayer != A) // Grab the shell
                                    {
                                        if(Player[A].Character >= 3)
                                            PlaySoundSpatial(SFX_Grab, Player[A].Location);
                                        else
                                            UnDuck(Player[A]);

                                        Player[A].HoldingNPC = B;
                                        NPC[B].HoldingPlayer = A;
                                        NPC[B].CantHurt = Physics.NPCCanHurtWait;
                                        NPC[B].CantHurtPlayer = A;
                                    }
                                    else if(NPC[B].HoldingPlayer == 0) // Kick the shell
                                    {
                                        if((Player[A].Mount == 1 || Player[A].Mount == 2 || Player[A].Mount == 3) && NPC[B].Type != NPCID_SLIDE_BLOCK)
                                        {
                                            if(NPC[B].Type != NPCID_FLIPPED_RAINBOW_SHELL)
                                            {
                                                tempLocation.Y = Player[A].Location.Y;
                                                tempHit = B;
                                                NPCHit(B, 8, A);
                                            }
                                        }
                                        else
                                        {
                                            tempLocation.Height = 0;
                                            tempLocation.Width = 0;
                                            tempLocation.Y = (Player[A].Location.Y + NPC[B].Location.Y * 4) / 5;
                                            tempLocation.X = (Player[A].Location.X + NPC[B].Location.X * 4) / 5;
                                            NewEffect(EFFID_STOMP_INIT, tempLocation);
                                            NPC[B].CantHurt = 0;
                                            NPC[B].CantHurtPlayer = 0;
                                            NPCHit(B, 1, A);
                                        }
                                    }
                                }
                                else if(NPC[B].Location.SpeedX != 0) // Got hit by the shell
                                {
                                    if(NPC[B].CantHurtPlayer != A && !FreezeNPCs && NPC[B].Type != NPCID_FLIPPED_RAINBOW_SHELL)
                                    {
                                        if(n00bCollision(Player[A].Location, NPC[B].Location))
                                            PlayerHurt(A);
                                    }
                                }
                            }
                            else if(NPC[B]->IsABonus) // Bonus
                                TouchBonus(A, B);
                            else // Everything else
                            {
                                if((NPC[B].Type == NPCID_LIT_BOMB_S3 || NPC[B].Type == NPCID_HIT_CARRY_FODDER) && NPC[B].HoldingPlayer != A) // kick the bob-om
                                {
                                    if(NPC[B].TailCD == 0)
                                    {
                                        NPC[B].TailCD = 12;
                                        if(NPC[B].Type != NPCID_HIT_CARRY_FODDER && NPC[B].Type != NPCID_LIT_BOMB_S3)
                                            NewEffect(EFFID_WHACK, newLoc((Player[A].Location.X + NPC[B].Location.X + (Player[A].Location.Width + NPC[B].Location.Width) / 2) / 2, (Player[A].Location.Y + NPC[B].Location.Y + (Player[A].Location.Height + NPC[B].Location.Height) / 2) / 2));
                                        NPCHit(B, 1, A);
                                    }
                                }
                                else if(NPC[B].CantHurtPlayer != A && !NPC[B]->WontHurt)
                                {
                                    if(!(NPC[B].Type == NPCID_BULLET && NPC[B].Projectile != 0))
                                    {
                                        if(NPC[B].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[B].Type <= NPCID_YEL_HIT_TURTLE_S4 && NPC[B].Projectile != 0)
                                            NPCHit(B, 3, B);
                                        else
                                        {
                                            if(NPC[B].Effect != NPCEFF_DROP_ITEM)
                                            {
                                                if(InvincibilityTime || (Player[A].SlideKill && !NPC[B]->JumpHurt) || (Player[A].Rolling && Player[A].State == PLR_STATE_SHELL))
                                                {
                                                    // DO cause damage to VILLAIN_S3 even though it's meant to be immune to this kind of damage.
                                                    if(InvincibilityTime && NPC[B].Type == NPCID_VILLAIN_S3)
                                                    {
                                                        // But use fireball -- this will take ~6 rounds of invincibility power to kill the boss
                                                        NPC[numNPCs + 1].Type = NPCID_PLR_FIREBALL;
                                                        NPCHit(B, 3, numNPCs + 1);
                                                    }
                                                    else
                                                        NPCHit(B, 3, B);
                                                }

                                                if(NPC[B].Killed == 0)
                                                {
                                                    if(n00bCollision(Player[A].Location, NPC[B].Location))
                                                    {
                                                        if(BattleMode && NPC[B].HoldingPlayer != A && NPC[B].HoldingPlayer > 0 && Player[A].Immune == 0)
                                                            NPCHit(B, 5, B);
                                                        PlayerHurt(A);
                                                    }
                                                }
                                                else
                                                    MoreScore(NPC[B]->Score, NPC[B].Location, Player[A].Multiplier);
                                            }
                                        }
                                    }
                                }

                                // this is for NPC that physically push the player
                                if(NPC[B]->MovesPlayer && NPC[B].Projectile == 0 && Player[A].HoldingNPC != B &&
                                   !(Player[A].Mount == 2 && (NPC[B].Type == NPCID_KEY || NPC[B].Type == NPCID_COIN_SWITCH)) &&
                                   !ShadowMode && NPC[B].Effect != NPCEFF_DROP_ITEM)
                                {
                                    if(Player[A].StandUp && Player[A].StandingOnNPC == 0)
                                    {
                                        if(HitSpot == 5 && Player[A].Location.Y + Player[A].Location.Height - Physics.PlayerDuckHeight[Player[A].Character][Player[A].State] - Player[A].Location.SpeedY >= NPC[B].Location.Y + NPC[B].Location.Height)
                                            HitSpot = 3;
                                    }

                                    if(Player[A].CurMazeZone != 0)
                                    {
                                        // don't actually collide
                                    }
                                    else if(HitSpot == 3)
                                    {
                                        if(NPC[B].Type == NPCID_ICE_CUBE && Player[A].Character != 5 && Player[A].State > 1)
                                            NPCHit(B, 3, B);
                                        tempLocation = Player[A].Location;
                                        Player[A].Location.SpeedY = 0.1_n + NPC[B].Location.SpeedY;
                                        Player[A].Location.Y = NPC[B].Location.Y + NPC[B].Location.Height + 0.1_n;

                                        // fBlock = FirstBlock[(Player[A].Location.X / 32) - 1];
                                        // lBlock = LastBlock[((Player[A].Location.X + Player[A].Location.Width) / 32.0) + 1];
                                        // blockTileGet(Player[A].Location, fBlock, lBlock);

                                        for(int C : treeFLBlockQuery(Player[A].Location, SORTMODE_NONE))
                                        {
                                            if(CheckCollision(Player[A].Location, Block[C].Location) &&
                                               !Block[C].Hidden && !BlockIsSizable[Block[C].Type] &&
                                               !BlockOnlyHitspot1[Block[C].Type])
                                            {
                                                Player[A].Location = tempLocation;
                                                break;
                                            }
                                        }

                                        PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                        Player[A].Jump = 0;
                                        if(Player[A].Mount == 2)
                                            Player[A].Location.SpeedY += 2;

                                        if(NPC[B].Type == NPCID_METALBARREL || NPC[B].Type == NPCID_CANNONENEMY || NPC[B].Type == NPCID_HPIPE_SHORT || NPC[B].Type == NPCID_HPIPE_LONG || NPC[B].Type == NPCID_VPIPE_SHORT || NPC[B].Type == NPCID_VPIPE_LONG || (NPC[B].Type >= NPCID_TANK_TREADS && NPC[B].Type <= NPCID_SLANT_WOOD_M))
                                        {
                                            if(NPC[B].Location.SpeedY >= Physics.NPCGravity * 20)
                                                PlayerHurt(A);
                                        }
                                    }
                                    // player gets pushed left/right by NPC
                                    else
                                    {
                                        tempHit2 = true;
                                        // this variable was previously a numf_t
                                        tempHitSpeed = (num_t)(tempf_t)(NPC[B].Location.SpeedX + (num_t)NPC[B].BeltSpeed);

                                        // reset player speed if not on conveyor belt
                                        bool tempBool = false;
                                        if(Player[A].StandingOnNPC != 0)
                                        {
                                            if(NPC[Player[A].StandingOnNPC].Type == NPCID_CONVEYOR)
                                                tempBool = true;
                                        }

                                        if(Player[A].Rolling)
                                        {
                                            PlaySoundSpatial(SFX_BlockHit, Player[A].Location);
                                            if((Player[A].Location.SpeedX > 0) == (Player[A].Direction > 0))
                                                Player[A].Location.SpeedX = -Player[A].Location.SpeedX;
                                            if(Player[A].State != PLR_STATE_POLAR)
                                                tempHit2 = false;
                                        }
                                        else if(!tempBool && NPC[B].Type != NPCID_CHASER)
                                            Player[A].Location.SpeedX = 0.2_n * Player[A].Direction;

                                        // reset player run count
                                        Player[A].RunCount = 0;

                                        // mark moving pinched (if needed)
                                        if(NPC[B].Type != NPCID_KEY && NPC[B].Type != NPCID_COIN_SWITCH && NPC[B].Type != NPCID_CONVEYOR && (NPC[B].Location.SpeedX != 0 || NPC[B].Location.SpeedY != 0 || NPC[B].BeltSpeed))
                                        {
                                            Player[A].Pinched.Moving = 2;

                                            if(NPC[B].Location.SpeedX != 0 || NPC[B].BeltSpeed)
                                                Player[A].Pinched.MovingLR = true;
                                        }

                                        // save current X location (for NPCs riding player's vehicle)
                                        tempf_t D = (tempf_t)Player[A].Location.X;

                                        // actually move the player
                                        if(NPC[B].Location.to_right_of(Player[A].Location))
                                        {
                                            Player[A].Pinched.Right4 = 2;

                                            if(floorBlock != 0 && num_t::abs(Block[floorBlock].Location.X - NPC[B].Location.X) < 1)
                                            {
                                                Player[A].Location.X = NPC[B].Location.X - Player[A].Location.Width - 1;
                                                Player[A].Location.SpeedY = (num_t)oldSpeedY;
                                            }
                                            else
                                                Player[A].Location.X = NPC[B].Location.X - Player[A].Location.Width - 0.1_n;

                                            if(NPC[Player[A].StandingOnNPC].Type == NPCID_CONVEYOR)
                                                Player[A].Location.X -= 1;

                                            // forget about floors no longer supporting player
                                            if(floorNpc1 > 0)
                                            {
                                                if(NPC[B].Location.X >= NPC[floorNpc1].Location.X - 2 && NPC[B].Location.X <= NPC[floorNpc1].Location.X + 2)
                                                    floorNpc1 = floorNpc2;
                                            }

                                            if(floorNpc2 > 0)
                                            {
                                                if(NPC[B].Location.X >= NPC[floorNpc2].Location.X - 2 && NPC[B].Location.X <= NPC[floorNpc2].Location.X + 2)
                                                    floorNpc2 = 0;
                                            }
                                        }
                                        else
                                        {
                                            Player[A].Pinched.Left2 = 2;

                                            if(floorBlock != 0 && num_t::abs(Block[floorBlock].Location.X + Block[floorBlock].Location.Width - NPC[B].Location.X - NPC[B].Location.Width) < 1)
                                            {
                                                Player[A].Location.X = NPC[B].Location.X + NPC[B].Location.Width + 1;
                                                Player[A].Location.SpeedY = (num_t)oldSpeedY;
                                            }
                                            else
                                                Player[A].Location.X = NPC[B].Location.X + NPC[B].Location.Width + 0.01_n;

                                            // forget about floors no longer supporting player
                                            if(floorNpc1 > 0)
                                            {
                                                if(NPC[B].Location.X + NPC[B].Location.Width >= NPC[floorNpc1].Location.X + NPC[floorNpc1].Location.Width - 2 && NPC[B].Location.X + NPC[B].Location.Width <= NPC[floorNpc1].Location.X + NPC[floorNpc1].Location.Width + 2)
                                                    floorNpc1 = floorNpc2;
                                            }

                                            if(floorNpc2 > 0)
                                            {
                                                if(NPC[B].Location.X + NPC[B].Location.Width >= NPC[floorNpc2].Location.X + NPC[floorNpc2].Location.Width - 2 && NPC[B].Location.X + NPC[B].Location.Width <= NPC[floorNpc2].Location.X + NPC[floorNpc2].Location.Width + 2)
                                                    floorNpc2 = 0;
                                            }
                                        }

                                        // apply speed to vehicle riders if needed
                                        if(Player[A].Mount == 2)
                                        {
                                            D = (tempf_t)(Player[A].Location.X - (num_t)D);

                                            for(int C : NPCQueues::Active.no_change)
                                            {
                                                if(NPC[C].vehiclePlr == A)
                                                {
                                                    NPC[C].Location.X += (num_t)D;
                                                    treeNPCUpdate(C);
                                                }
                                            }

                                            for(int C = 1; C <= numPlayers; C++)
                                            {
                                                if(Player[C].StandingOnVehiclePlr && (g_ClonedPlayerMode || Player[C].StandingOnVehiclePlr == A))
                                                    Player[C].Location.X += (num_t)D;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // if the player collided on the left or right of some npcs then stop his movement
    if(tempHit2)
    {
        if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX + tempHitSpeed > 0 && Player[A].Controls.Right)
            Player[A].Location.SpeedX = 0.2_n * Player[A].Direction + tempHitSpeed;
        else if(Player[A].Location.SpeedX + NPC[Player[A].StandingOnNPC].Location.SpeedX + tempHitSpeed < 0 && Player[A].Controls.Left)
            Player[A].Location.SpeedX = 0.2_n * Player[A].Direction + tempHitSpeed;
        else
        {
            if(Player[A].Controls.Right || Player[A].Controls.Left)
                Player[A].Location.SpeedX = -NPC[Player[A].StandingOnNPC].Location.SpeedX + 0.2_n * Player[A].Direction;
            else
                Player[A].Location.SpeedX = 0;
        }
    }

    if(tempHit && Player[A].CurMazeZone)
    {
        if(NPC[tempHit].Effect == NPCEFF_MAZE && NPC[tempHit].Effect3 != MAZE_DIR_DOWN)
            NPC[tempHit].TurnAround = true;

        Player[A].Location.Y = tempLocation.Y;
        Player[A].Location.SpeedY += Physics.PlayerJumpVelocity / 2;

        if(Player[A].MazeZoneStatus == MAZE_DIR_DOWN)
            Player[A].MazeZoneStatus |= MAZE_PLAYER_FLIP;
    }
    else if(tempHit) // For multiple NPC hits
    {
        // enable another double-jump when Char4 bounces on an NPC
        if(Player[A].Character == 4 && (Player[A].State == 4 || Player[A].State == 5) && !Player[A].SpinJump)
            Player[A].DoubleJump = true;

        Player[A].CanJump = true;

        if(tempSpring)
        {
            Player[A].Jump = Physics.PlayerSpringJumpHeight;
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity - 4;
        }
        else
        {
            Player[A].Jump = Physics.PlayerNPCJumpHeight;
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
        }

        if(Player[A].Character == 2)
            Player[A].Jump += 3;

        if(Player[A].SpinJump)
            Player[A].Jump -= 6;

        if(Player[A].Wet > 0)
            Player[A].Location.SpeedY *= 0.3_r;

        // this is very likely but not certain to be the y value stored when tempHit was set
        Player[A].Location.Y = tempLocation.Y;

        if(tempShell)
            NewEffect(EFFID_STOMP_INIT, newLoc(Player[A].Location.X + Player[A].Location.Width / 2 - EffectWidth[EFFID_STOMP_INIT] * 0.5_n, Player[A].Location.Y + Player[A].Location.Height - EffectHeight[EFFID_STOMP_INIT] * 0.5_n));
        else if(!tempSpring)
            NewEffect(EFFID_WHACK, newLoc(Player[A].Location.X + Player[A].Location.Width / 2 - 16, Player[A].Location.Y + Player[A].Location.Height - 16));
        else
            tempSpring = false;

        PlayerPush(A, 3);

        if(Player[A].YoshiBlue)
        {
            Player[A].CanFly2 = true;
            Player[A].FlyCount = 300;
        }

        if(spinKill)
        {
            Player[A].Jump = 0;
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity; // * 0.5
        }
    }



    // Find out which NPC to stand on

    int B = 0;

    // this code is for standing on moving NPCs.
    if(floorNpc2 != 0)
    {
        if(NPC[floorNpc1].Location.Y == NPC[floorNpc2].Location.Y)
        {
            num_t C = num_t::abs(NPC[floorNpc1].Location.minus_center_x(Player[A].Location));
            num_t D = num_t::abs(NPC[floorNpc2].Location.minus_center_x(Player[A].Location));

            if(C < D)
                B = floorNpc1;
            else
                B = floorNpc2;
        }
        else if(NPC[floorNpc1].Location.Y < NPC[floorNpc2].Location.Y)
            B = floorNpc1;
        else
            B = floorNpc2;
    }
    else if(floorNpc1 != 0)
        B = floorNpc1;

    if(NPC[floorNpc1].Type >= NPCID_YEL_PLATFORM && NPC[floorNpc1].Type <= NPCID_RED_PLATFORM)
        B = floorNpc1;
    else if(NPC[floorNpc2].Type >= NPCID_YEL_PLATFORM && NPC[floorNpc2].Type <= NPCID_RED_PLATFORM)
        B = floorNpc2;

    if(NPC[B].Effect == NPCEFF_DROP_ITEM)
        B = 0;

    if(NPC[B].Projectile != 0 && NPCIsVeggie(NPC[B]))
        B = 0;

    // B is the number of the NPC that the player is standing on
    // .StandingOnNPC is the number of the NPC that the player was standing on last cycle
    // if B = 0 and .standingonnpc > 0 then the player was standing on something and is no longer standing on something


    if(B > 0 && Player[A].SpinJump)
    {
        if(NPC[B].Type == NPCID_ICE_CUBE)
        {
            Player[A].Location.SpeedY = Physics.PlayerJumpVelocity;
            NPC[B].Multiplier += Player[A].Multiplier;
            NPCHit(B, 3, B);
            Player[A].Jump = 7;

            if(Player[A].Character == 2)
                Player[A].Jump += 3;

            if(Player[A].Controls.Down)
            {
                Player[A].Jump = 0;
                Player[A].Location.SpeedY = Physics.PlayerJumpVelocity / 2;
            }

            B = 0;
        }
    }

    if(Player[A].HoldingNPC == B) // cant hold an npc that you are standing on
        B = 0;

    // don't stand on NPCs during maze zone
    if(Player[A].CurMazeZone)
        B = 0;

    // confusing logic, but safe, because StandingOnNPC gets set in ClownCar()
    if(B == 0 && Player[A].StandingOnVehiclePlr > 0 && Player[A].Mount == 0)
        Player[A].Location.SpeedX += (NPC[Player[A].StandingOnNPC].Location.SpeedX + (num_t)NPC[Player[A].StandingOnNPC].BeltSpeed);
    else if(B > 0 && Player[A].StandingOnNPC == 0 && NPC[B].playerTemp && Player[A].Location.SpeedY >= 0)
        Player[A].Location.SpeedX += -(NPC[B].Location.SpeedX + (num_t)NPC[B].BeltSpeed);

    if(movingBlock) // this is for when the player is standing on a moving block
    {
        if(B > 0)
        {
            if(NPC[B].Type == NPCID_ITEM_BURIED)
            {
                // movingBlock = false;
            }
            else
                B = -A;
        }
        else
            B = -A;
    }

    if(B != 0)
    {
        if(Player[A].StandingOnNPC == 0 && (Player[A].GroundPound || Player[A].YoshiYellow))
        {
            numBlock++;
            Block[numBlock].Location.Y = NPC[B].Location.Y;
            // seems weird but I'll sync it since we don't know what could happen inside YoshiPound
            syncLayersTrees_Block(numBlock);

            YoshiPound(A, Player[A].Mount, Player[A].GroundPound);
            Player[A].GroundPound = false;

            Block[numBlock].Location.Y = 0;
            numBlock--;
            syncLayersTrees_Block(numBlock + 1);
        }

        if(NPC[B].playerTemp == 0)
            Player[A].StandingOnVehiclePlr = 0;

        if(Player[A].Location.SpeedY >= 0)
            Player[A].StandingOnNPC = B;

        Player[A].Location.Y = NPC[B].Location.Y - Player[A].Location.Height;

        // NPC has been stood on (condition over NPC types)
        if(NPC[B].Type == NPCID_FALL_BLOCK_RED || NPC[B].Type == NPCID_FALL_BLOCK_BROWN)
            NPC[B].Special2 = 1;
        else if(NPC[B].Type == NPCID_CHECKER_PLATFORM)
            NPC[B].Special = 1;
        else if(NPC[B].Type == NPCID_PLATFORM_S3)
        {
            if(Player[A].Location.SpeedY > 0)
                NPC[B].Direction = 1;
        }
        else if(NPC[B].Type == NPCID_RAFT)
        {
            if(NPC[B].Special == 0)
            {
                NPC[B].Special = 1;
                SkullRide(B);
            }
        }
        else if(NPC[B].Type == NPCID_CONVEYOR)
            Player[A].Location.SpeedY = 0;
        else if(NPC[B].Type == NPCID_VEHICLE)
        {
            if(Player[A].Controls.Down && Player[A].Mount == 0 &&
               !NPC[B].playerTemp && Player[A].DuckRelease &&
               (Player[A].HoldingNPC == 0 || Player[A].Character == 5))
            {
                UnDuck(Player[A]);

                if(g_config.fix_char5_vehicle_climb && Player[A].Fairy) // Avoid the mortal glitch
                {
                    Player[A].Fairy = false;
                    PlaySoundSpatial(SFX_HeroFairy, Player[A].Location);
                    NewEffect(EFFID_SMOKE_S5, Player[A].Location);
                }

                if(g_config.fix_vehicle_altjump_bug)
                    Player[A].SpinJump = false;

                Player[A].Location = NPC[B].Location;
                Player[A].Mount = 2;
                NPC[B].Killed = 9;
                NPCQueues::Killed.push_back(B);
                Player[A].HoldingNPC = 0;
                Player[A].StandingOnNPC = 0;
                PlaySoundSpatial(SFX_Stomp, Player[A].Location);
                for(int C = 1; C <= numPlayers; ++C)
                {
                    if(Player[C].StandingOnNPC == B)
                        Player[C].StandingOnVehiclePlr = A;
                }

                B = 0;
            }
        }

        if(Player[A].Mount == 2 && B != 0)
        {
            Player[A].StandingOnNPC = 0;
            if(Player[A].Location.SpeedY > 4 + NPC[B].Location.SpeedY)
                PlaySoundSpatial(SFX_Stone, Player[A].Location);
            Player[A].Location.SpeedY = NPC[B].Location.SpeedY;
        }
    }
    else if(Player[A].Mount == 1 && Player[A].Jump == 0)
    {
        if(Player[A].StandingOnNPC != 0)
        {
            if(Player[A].Location.X > NPC[Player[A].StandingOnNPC].Location.X + NPC[Player[A].StandingOnNPC].Location.Width || Player[A].Location.X + Player[A].Location.Width < NPC[Player[A].StandingOnNPC].Location.X)
            {
                Player[A].StandingOnNPC = 0;
                Player[A].StandingOnVehiclePlr = 0;

                if(Player[A].Location.SpeedY > 4.1_n)
                {
                    Player[A].Location.Y += -Player[A].Location.SpeedY;
                    Player[A].Location.SpeedY = NPC[0 /*Player[A].StandingOnNPC*/].Location.SpeedY; // SMBX 1.3 bug

                    if(Player[A].Location.SpeedY > Physics.PlayerTerminalVelocity)
                        Player[A].Location.SpeedY = Physics.PlayerTerminalVelocity;

                    Player[A].Location.Y += Player[A].Location.SpeedY;
                }
            }
        }
    }
    else if(Player[A].Mount == 1 && Player[A].Jump > 0)
    {
        // confusing logic, but safe, because StandingOnNPC gets set in ClownCar()
        // NOTE: we know that B == 0!
        if(/*B == 0 &&*/ Player[A].StandingOnVehiclePlr > 0)
            Player[A].Location.SpeedX += (NPC[Player[A].StandingOnNPC].Location.SpeedX + (num_t)NPC[Player[A].StandingOnNPC].BeltSpeed);
        // else if(B > 0 && Player[A].StandingOnNPC == 0 && NPC[B].playerTemp)
        //     Player[A].Location.SpeedX += -(NPC[B].Location.SpeedX + NPC[B].BeltSpeed);

        Player[A].StandingOnNPC = 0;
        Player[A].StandingOnVehiclePlr = 0;
    }
    else
    {
        if(Player[A].StandingOnNPC != 0)
        {
            if(Player[A].StandingOnNPC < 0)
                Player[A].Location.SpeedX += NPC[Player[A].StandingOnNPC].Location.SpeedX;

            Player[A].Location.Y += -Player[A].Location.SpeedY;
            Player[A].Location.SpeedY = NPC[Player[A].StandingOnNPC].Location.SpeedY;

            if(FreezeNPCs)
                Player[A].Location.SpeedY = 0;
            if(Player[A].Location.SpeedY > Physics.PlayerTerminalVelocity)
                Player[A].Location.SpeedY = Physics.PlayerTerminalVelocity;

            Player[A].Location.Y += Player[A].Location.SpeedY;
        }

        Player[A].StandingOnNPC = 0;
        Player[A].StandingOnVehiclePlr = 0;
    }

    if(Player[A].StandingOnNPC > 0 && Player[A].Mount == 0) // driving stuff
    {
        if(NPC[Player[A].StandingOnNPC].Type == NPCID_COCKPIT)
        {
            Player[A].Driving = true;
            Player[A].Location.X = NPC[Player[A].StandingOnNPC].Location.X + (NPC[Player[A].StandingOnNPC].Location.Width - Player[A].Location.Width) / 2;
            Player[A].Direction = NPC[Player[A].StandingOnNPC].DefaultDirection;
        }
    }
}
