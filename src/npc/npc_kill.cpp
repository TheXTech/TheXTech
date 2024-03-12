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

#include "../globals.h"
#include "../npc.h"
#include "../npc_id.h"
#include "../eff_id.h"
#include "../sound.h"
#include "../effect.h"
#include "../layers.h"
#include "../game_main.h"
#include "../main/speedrunner.h"
#include "../compat.h"
#include "../controls.h"
#include "../layers.h"

#include "npc_traits.h"

#include "sdl_proxy/sdl_stdinc.h"

#include "npc/npc_queues.h"

void KillNPC(int A, int B)
{
    // ------+  KILL CODES  +-------
    // B = 1      Jumped on by a player (or kicked)
    // B = 2      Hit by a shaking block
    // B = 3      Hit by projectile
    // B = 4      Hit something as a projectile
    // B = 5      Hit something while being held
    // B = 6      Touched a lava block
    // B = 8      Stomped by boot
    // B = 9      Time to DIE
    // B = 10     Zelda Stab
    bool DontSpawnExit = false;
    bool DontResetMusic = false;
    bool tempBool = false;
    NPC_t blankNPC;
    int C = 0;
    Location_t tempLocation;

    // don't need to worry about updating NPC A's tree because that will certainly happen in either the syncLayersNPC or the Deactivate call at the end of the procedure

    if(NPC[A].Type == NPCID_ICE_CUBE && NPC[A].Special > 0 && NPC[A].Killed != 9)
    {
        NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
        for(C = 1; C <= 50; C++)
        {
            tempLocation.Height = EffectHeight[80];
            tempLocation.Width = EffectWidth[80];
            tempLocation.SpeedX = 0;
            tempLocation.SpeedY = 0;
            tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width;
            tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height;
            NewEffect(EFFID_SPARKLE, tempLocation);
            Effect[numEffects].Location.SpeedX = dRand() * 4 - 2 - NPC[A].Location.SpeedX * 0.2;
            Effect[numEffects].Location.SpeedY = dRand() * 4 - 2 + NPC[A].Location.SpeedY * 0.2;
            Effect[numEffects].Frame = iRand(3);
        }
        PlaySoundSpatial(SFX_Icebreak, NPC[A].Location);
        NPC[A].Type = NPCID(NPC[A].Special);

        NPCQueues::update(A);

        if(B != 10)
        {
            MoreScore(NPC[A]->Score, NPC[A].Location, NPC[A].Multiplier);
        }

        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.4;
        if(NPC[A].Location.SpeedX < 2 && NPC[A].Location.SpeedX > -2)
            NPC[A].Location.SpeedX = double(2 * NPC[A].Direction);
    }

    if(NPC[A].Killed == 8 && NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_KNIGHT && !NPCIsABot(NPC[A]) && NPC[A].Type != NPCID_FLIER && NPC[A].Type != NPCID_ROCKET_FLIER && NPC[A].Type != NPCID_WALL_BUG && NPC[A].Type != NPCID_HOMING_BALL)
        NewEffect(EFFID_BOOT_STOMP, NPC[A].Location);

    if(NPC[A].Type == NPCID_YELSWITCH_FODDER || NPC[A].Type == NPCID_BLUSWITCH_FODDER || NPC[A].Type == NPCID_GRNSWITCH_FODDER || NPC[A].Type == NPCID_REDSWITCH_FODDER || NPC[A].DefaultType == 59 || NPC[A].DefaultType == 61 || NPC[A].DefaultType == 63 || NPC[A].DefaultType == 65)
    {
        tempBool = false;

        for(C = 1; C <= numNPCs; C++)
        {
            if(NPC[C].Type == NPC[A].Type && NPC[C].Section == NPC[A].Section && C != A)
            {
                tempBool = true;
                break;
            }
        }

        if(LevelEditor)
            tempBool = true;

        if(!tempBool)
        {
            if(NPC[A].Type == NPCID_YELSWITCH_FODDER || NPC[A].DefaultType == 59)
            {
                PlaySoundSpatial(SFX_PSwitch, NPC[A].Location);
                for(C = 1; C <= numBlock; C++)
                {
                    if(Block[C].Type == 171)
                        Block[C].Type = 172;
                    else if(Block[C].Type == 172)
                        Block[C].Type = 171;
                }
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == NPCID_YEL_PLATFORM)
                        NPC[C].Direction = -NPC[C].Direction;
                }
            }
            else if(NPC[A].Type == NPCID_BLUSWITCH_FODDER || NPC[A].DefaultType == 61)
            {
                PlaySoundSpatial(SFX_PSwitch, NPC[A].Location);
                for(C = 1; C <= numBlock; C++)
                {
                    if(Block[C].Type == 174)
                        Block[C].Type = 175;
                    else if(Block[C].Type == 175)
                        Block[C].Type = 174;
                }
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == NPCID_BLU_PLATFORM)
                        NPC[C].Direction = -NPC[C].Direction;
                }
            }
            else if(NPC[A].Type == NPCID_GRNSWITCH_FODDER || NPC[A].DefaultType == 63)
            {
                PlaySoundSpatial(SFX_PSwitch, NPC[A].Location);
                for(C = 1; C <= numBlock; C++)
                {
                    if(Block[C].Type == 177)
                        Block[C].Type = 178;
                    else if(Block[C].Type == 178)
                        Block[C].Type = 177;
                }
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == NPCID_GRN_PLATFORM)
                        NPC[C].Direction = -NPC[C].Direction;
                }
            }
            else if(NPC[A].Type == NPCID_REDSWITCH_FODDER || NPC[A].DefaultType == 65)
            {
                PlaySoundSpatial(SFX_PSwitch, NPC[A].Location);
                for(C = 1; C <= numBlock; C++)
                {
                    if(Block[C].Type == 180)
                        Block[C].Type = 181;
                    else if(Block[C].Type == 181)
                        Block[C].Type = 180;
                }
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == NPCID_RED_PLATFORM)
                        NPC[C].Direction = -NPC[C].Direction;
                }
            }
        }
    }

    if(NPC[A].TriggerDeath != EVENT_NONE && !LevelEditor)
    {
        ProcEvent(NPC[A].TriggerDeath, 0);
    }

    if(NPC[A].TriggerLast != EVENT_NONE)
    {
        tempBool = false;

        int lr = NPC[A].Layer;
        if(lr != LAYER_NONE)
        {
            for(int other_npc : Layer[lr].NPCs)
            {
                if(other_npc != A && !NPC[other_npc].Generator)
                {
                    tempBool = true;
                    break;
                }
            }

            if(!Layer[lr].blocks.empty())
                tempBool = true;
        }

        if(!tempBool)
            ProcEvent(NPC[A].TriggerLast, 0);
    }

    if(NPC[A].HoldingPlayer > 0)
    {
        if(!NPCIsAnExit(NPC[A])) // Tell the player the NPC he was holding is dead
            Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
    }

    if(NPC[A].Killed == 10) // Things that die by Link's sword
    {
        if(!(NPC[A].Type == NPCID_MINIBOSS || NPC[A].Type == NPCID_SPIT_BOSS || NPC[A].Type == NPCID_VILLAIN_S3 || NPC[A].Type == NPCID_BOSS_FRAGILE || NPC[A].Type == NPCID_VILLAIN_S1 || NPC[A].Type == NPCID_SICK_BOSS || NPC[A].Type == NPCID_FLIER || NPC[A].Type == NPCID_ROCKET_FLIER || NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_HOMING_BALL || NPC[A].Type == NPCID_BOSS_CASE))
        {
            PlaySoundSpatial(SFX_HeroKill, NPC[A].Location);
            NewEffect(EFFID_SMOKE_S5, NPC[A].Location);
            B = 9;
            if(iRand(10) < 3)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Type = NPCID_GEM_1;
                if(iRand(5) == 0)
                    NPC[numNPCs].Type = NPCID_GEM_5;
                if(iRand(40) < 3)
                    NPC[numNPCs].Type = NPCID_GEM_20;
                NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width / 2.0;
                NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                if(NPC[A].Location.Height >= 32)
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - NPC[numNPCs].Location.Height / 2.0;
                else if(NPC[A].Type == NPCID_BOTTOM_PLANT || NPC[A].Type == NPCID_LONG_PLANT_DOWN) // Stops the rupees from spawning in blocks
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 1;
                else // Stops the rupees from spawning in blocks
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height - NPC[numNPCs].Location.Height - 1;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].TimeLeft = Physics.NPCTimeOffScreen;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TailCD = 10;
                NPC[numNPCs].Special = 1;
                NPC[numNPCs].Location.SpeedY = -5;
                NPC[numNPCs].Location.SpeedX = (1 + dRand() * 0.5) * NPC[A].Direction;
                syncLayers_NPC(numNPCs);
                CheckSectionNPC(numNPCs);
            }
        }
    }

    if(B != 9)
    {
        NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
        if(NPC[A].Type == NPCID_SQUID_S1 || NPC[A].Type == NPCID_SQUID_S3 || NPC[A].Type == NPCID_FODDER_S3 || NPC[A].Type == NPCID_FODDER_S5 || NPC[A].Type == NPCID_FLY_FODDER_S5 || NPC[A].Type == NPCID_FLY_FODDER_S3 || NPC[A].Type == NPCID_BRUTE || NPC[A].Type == NPCID_BRUTE_SQUISHED || NPC[A].Type == NPCID_BIG_MOLE || NPC[A].Type == NPCID_CARRY_FODDER || NPC[A].Type == NPCID_HIT_CARRY_FODDER || NPC[A].Type == NPCID_FLY_CARRY_FODDER || NPC[A].Type == NPCID_GRN_FISH_S3 || NPC[A].Type == NPCID_FISH_S4 || NPC[A].Type == NPCID_RED_FISH_S3 || NPC[A].Type == NPCID_GOGGLE_FISH || NPC[A].Type == NPCID_GRN_FISH_S1 || NPC[A].Type == NPCID_BONE_FISH || NPC[A].Type == NPCID_ICE_BLOCK || NPC[A].Type == NPCID_ICE_CUBE) // Goomba / Rex
        {
            if(B == 1 && NPC[A].Type != NPCID_GRN_FISH_S3 && NPC[A].Type != NPCID_FISH_S4 && NPC[A].Type != NPCID_RED_FISH_S3 && NPC[A].Type != NPCID_GOGGLE_FISH && NPC[A].Type != NPCID_GRN_FISH_S1 && NPC[A].Type != NPCID_BONE_FISH)
            {
                if(NPC[A].Type == NPCID_FODDER_S3)
                    NewEffect(EFFID_FODDER_S3_SQUISH, NPC[A].Location);
                else if(NPC[A].Type == NPCID_FODDER_S5)
                    NewEffect(EFFID_FODDER_S5_SQUISH, NPC[A].Location);
                else if(NPC[A].Type == NPCID_SQUID_S1 && g_compatibility.fix_squid_stomp_effect)
                {
                    NPC[A].Location.SpeedY = 0.123;
                    NewEffect(EFFID_SQUID_S1_DIE, NPC[A].Location, NPC[A].Direction);
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                }
                else if(NPC[A].Type == NPCID_SQUID_S3 && g_compatibility.fix_squid_stomp_effect)
                {
                    NPC[A].Location.SpeedY = 0.123;
                    NewEffect(EFFID_SQUID_S3_DIE, NPC[A].Location, NPC[A].Direction);
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                }
                else
                    NewEffect(EFFID_BRUTE_SQUISH, NPC[A].Location, NPC[A].Direction);
            }
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - 16;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Y += NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - 16;
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                if(NPC[A].Type == NPCID_ICE_BLOCK) // Yoshi's ice break
                    PlaySoundSpatial(SFX_Icebreak, NPC[A].Location);
                else
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                if(NPC[A].Type == NPCID_FODDER_S3 || NPC[A].Type == NPCID_FLY_FODDER_S3)
                    NewEffect(EFFID_FODDER_S3_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_FODDER_S5 || NPC[A].Type == NPCID_FLY_FODDER_S5)
                    NewEffect(EFFID_FODDER_S5_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_SQUID_S3)
                    NewEffect(EFFID_SQUID_S3_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_SQUID_S1)
                    NewEffect(EFFID_SQUID_S1_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_BRUTE)
                    NewEffect(EFFID_BRUTE_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_BIG_MOLE)
                    NewEffect(EFFID_BIG_MOLE_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_BRUTE_SQUISHED)
                    NewEffect(EFFID_BRUTE_SQUISHED_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_GRN_FISH_S3)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(EFFID_GRN_FISH_S3_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_FISH_S4)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(EFFID_FISH_S4_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_RED_FISH_S3)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(EFFID_RED_FISH_S3_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_GOGGLE_FISH)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(EFFID_GOGGLE_FISH_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_GRN_FISH_S1)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(EFFID_GRN_FISH_S1_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_BONE_FISH)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(B == 1)
                        NPC[A].Location.SpeedY = -2;
                    NewEffect(EFFID_BONE_FISH_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_ICE_BLOCK || NPC[A].Type == NPCID_ICE_CUBE)
                {
                    NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                    for(C = 1; C <= 50; C++)
                    {
                        tempLocation.Height = EffectHeight[80];
                        tempLocation.Width = EffectWidth[80];
                        tempLocation.SpeedX = 0;
                        tempLocation.SpeedY = 0;
                        tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width;
                        tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height;
                        NewEffect(EFFID_SPARKLE, tempLocation);
                        Effect[numEffects].Location.SpeedX = dRand() * 2 - 1 - NPC[A].Location.SpeedX * 0.3;
                        Effect[numEffects].Location.SpeedY = dRand() * 2 - 1 + NPC[A].Location.SpeedY * 0.3;
                        Effect[numEffects].Frame = iRand(3);
                    }
                }
                else
                    NewEffect(EFFID_CARRY_FODDER_DIE, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == NPCID_FLIER || NPC[A].Type == NPCID_ROCKET_FLIER || NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_HOMING_BALL)
        {
            PlaySoundSpatial(SFX_SMKilled, NPC[A].Location);
            NewEffect(EFFID_BOSS_FRAGILE_EXPLODE, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_BOSS_CASE)
        {
            PlaySoundSpatial(SFX_SMGlass, NPC[A].Location);
            for(C = 1; C <= 100; C++)
                NewEffect(EFFID_BOSS_CASE_BREAK, newLoc(NPC[A].Location.X + dRand() * NPC[A].Location.Width - 16, NPC[A].Location.Y + dRand() * NPC[A].Location.Height - 16));
        }
        else if(NPC[A].Type == NPCID_BOSS_FRAGILE)
        {
            PlaySoundSpatial(SFX_SMCry, NPC[A].Location);
            PlaySoundSpatial(SFX_SMExplosion, NPC[A].Location);
            NewEffect(EFFID_BOSS_FRAGILE_DIE, NPC[A].Location, NPC[A].Direction);
        }
        else if(NPC[A].Type == NPCID_GHOST_S3 || NPC[A].Type == NPCID_GHOST_FAST || NPC[A].Type == NPCID_GHOST_S4 || NPC[A].Type == NPCID_BIG_GHOST || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_FIRE_DISK)
        {
            PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
            NPC[A].Location.SpeedY = -10;
            if(NPC[A].Type == NPCID_GHOST_S3)
                NewEffect(EFFID_GHOST_S3_DIE, NPC[A].Location, NPC[A].Direction);
            else if(NPC[A].Type == NPCID_GHOST_FAST)
                NewEffect(EFFID_GHOST_FAST_DIE, NPC[A].Location, NPC[A].Direction);
            else if(NPC[A].Type == NPCID_GHOST_S4)
                NewEffect(EFFID_GHOST_S4_DIE, NPC[A].Location, NPC[A].Direction);
            else if(NPC[A].Type == NPCID_WALL_SPARK)
                NewEffect(EFFID_WALL_SPARK_DIE, NPC[A].Location);
            else if(NPC[A].Type == NPCID_FIRE_DISK)
            {
                NewEffect(EFFID_FIRE_DISK_DIE, NPC[A].Location);
                Effect[numEffects].Frame = NPC[A].Frame;
                Effect[numEffects].Life = 100;
            }
            else
                NewEffect(EFFID_BIG_GHOST_DIE, NPC[A].Location, NPC[A].Direction);
        }
        else if(NPC[A].Type == NPCID_BULLY) // bully
        {
            NPC[A].Location.SpeedY = -8;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height - EffectHeight[10];
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_BULLY_DIE, NPC[A].Location, NPC[A].Direction);
            }

        }
        // turnips
        else if(NPCIsVeggie(NPC[A]))
        {
            NPC[A].Location.Y += -(32 - NPC[A].Location.Height);
            NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
            NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
            NPC[A].Location.Height = 32;
            NPC[A].Location.Width = 32;
            if(NPC[A].Killed == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_COIN_SWITCH) // P Switch
        {
            if(B == 1)
            {
                NPC[A].Location.Y += 2;
                NewEffect(EFFID_COIN_SWITCH_PRESS, NPC[A].Location);
            }
            else if(B == 2)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_TIME_SWITCH) // P Switch Time
        {
            if(B == 1)
            {
                NPC[A].Location.Y += 2;
                NewEffect(EFFID_TIME_SWITCH_PRESS, NPC[A].Location);
            }
            else if(B == 2)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_TNT) // Red Switch
        {
            if(B == 1)
            {
                NPC[A].Location.Y += 2;
                NewEffect(124 , NPC[A].Location);
            }
            else if(B == 2)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            }
        }
        else if((NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_STATUE_S4) && B == 6) // lava only
        {
            NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
            NPC[A].Location.Y += NPC[A].Location.Height - 32;
            NPC[A].Location.Height = 32;
            NPC[A].Location.Width = 32;
            PlaySoundSpatial(SFX_Lava, NPC[A].Location);
            NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            if(!NPC[A].NoLavaSplash)
                NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_SAW || NPC[A].Type == NPCID_STONE_S4) // thwomp
        {
            if(B == 3 || B == 4 || B == 2)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NPC[A].Location.SpeedY = -10;
                if(NPC[A].Type == NPCID_STONE_S3)
                    NewEffect(EFFID_STONE_S3_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_SAW)
                {
                    NPC[A].Location.Width = 64;
                    NPC[A].Location.Height = 64;
                    NPC[A].Location.X -= 8;
                    NPC[A].Location.Y -= 8;
                    NewEffect(EFFID_SAW_DIE, NPC[A].Location);
                }
                else if(NPC[A].Type == NPCID_STONE_S4)
                    NewEffect(EFFID_STONE_S4_DIE, NPC[A].Location);
            }
            else if(B == 6)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(NPC[A].Type == NPCID_SAW)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NPC[A].Location.SpeedY = -10;
                NPC[A].Location.Width = 64;
                NPC[A].Location.Height = 64;
                NPC[A].Location.X -= 8;
                NPC[A].Location.Y -= 8;
                NewEffect(EFFID_SAW_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_WALK_BOMB_S3 || NPC[A].Type == NPCID_LIT_BOMB_S3)
        {
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NPC[A].Location.SpeedY = -11;
                NewEffect(EFFID_WALK_BOMB_S3_DIE, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == NPCID_FODDER_S1) // SMB1 Goomba
        {
            if(B == 1)
                NewEffect(EFFID_FODDER_S1_SQUISH, NPC[A].Location);
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_FODDER_S1_DIE, NPC[A].Location);
            }
        // Zelda NPCs
        }
        else if(NPC[A].Type == NPCID_KNIGHT || NPCIsABot(NPC[A]) || NPC[A].Type == NPCID_LOCK_DOOR)
        {
            PlaySoundSpatial(SFX_HeroKill, NPC[A].Location);
            NewEffect(EFFID_SMOKE_S5, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_SPIT_GUY_BALL)
        {
            NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX * 0.3;
            NewEffect(EFFID_SPIT_GUY_BALL_DIE, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_YELSWITCH_FODDER || NPC[A].Type == NPCID_BLUSWITCH_FODDER || NPC[A].Type == NPCID_GRNSWITCH_FODDER || NPC[A].Type == NPCID_REDSWITCH_FODDER) // switch goombas
        {
            if(B == 1)
            {
                NewEffect(NPC[A].Type - NPCID_YELSWITCH_FODDER + EFFID_YELSWITCH_FODDER_SQUISH, NPC[A].Location);
            }
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(NPC[A].Type - NPCID_YELSWITCH_FODDER + EFFID_YELSWITCH_FODDER_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_MAGIC_BOSS || NPC[A].Type == NPCID_MAGIC_BOSS_SHELL || NPC[A].Type == NPCID_FIRE_BOSS || NPC[A].Type == NPCID_FIRE_BOSS_SHELL) // larry koopa
        {
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(NPC[A].Type == NPCID_FIRE_BOSS || NPC[A].Type == NPCID_FIRE_BOSS_SHELL)
                NewEffect(EFFID_FIRE_BOSS_DIE, NPC[A].Location);
            else
                NewEffect(EFFID_MAGIC_BOSS_DIE, NPC[A].Location);

        }
        else if(NPC[A].Type == NPCID_SICK_BOSS || NPC[A].Type == NPCID_BOMBER_BOSS) // wart, smb2 bosses
        {
            if(NPC[A].Type == NPCID_BOMBER_BOSS && NPC[A].Killed != 3 && NPC[A].Killed != 6 && NPC[A].Killed != 10)
                PlaySoundSpatial(SFX_SpitBossBeat, NPC[A].Location);

            // If B <> 6 Then MoreScore NPCScore(.Type), .Location
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NPC[A].Location.Y += 24;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else
            {
                NPC[A].Location.SpeedY = -7;
                if(NPC[A].Type == NPCID_BOMBER_BOSS)
                {
                    NewEffect(EFFID_BOMBER_BOSS_DIE, NPC[A].Location, NPC[A].Direction);
                    Effect[numEffects].Location.SpeedX = 0;
                    Effect[numEffects].Location.SpeedY = -8;
                }
                else
                    NewEffect(EFFID_SICK_BOSS_DIE, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == NPCID_VILLAIN_S1) // king koopa
        {
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NPC[A].Location.Y += 24;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else
            {
                // .Location.Width += 2
                // .Location.X += -1
                NewEffect(EFFID_VILLAIN_S1_DIE, NPC[A].Location, NPC[A].Direction);
            }
            PlaySoundSpatial(SFX_VillainKilled, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_VILLAIN_S3) // bowser
        {
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NPC[A].Location.Y += 24;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else
            {
                NPC[A].Location.Width += 2;
                NPC[A].Location.X -= 1;
                NewEffect(EFFID_VILLAIN_S3_DIE, NPC[A].Location, NPC[A].Direction);
            }
            PlaySoundSpatial(SFX_VillainKilled, NPC[A].Location);

            if(NPC[A].Legacy)
            {
                tempBool = false;
                for(B = 1; B <= numNPCs; B++)
                {
                    if(B != A && NPC[B].Type == NPCID_VILLAIN_S3)
                    {
                        tempBool = true;
                        break;
                    }
                }

                if(!tempBool)
                {
                    speedRun_bossDeadEvent();
                    LevelMacroCounter = 0;
                    LevelMacro = LEVELMACRO_GAME_COMPLETE_EXIT;
                }
            }
        }
        else if(NPC[A].Type == NPCID_RED_FODDER || NPC[A].Type == NPCID_RED_FLY_FODDER) // Red goomba
        {
            if(B == 1)
                NewEffect(EFFID_RED_FODDER_SQUISH, NPC[A].Location);
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_RED_FODDER_DIE, NPC[A].Location);
            }
        }
        else if(NPCIsToad(NPC[A])) // toad
        {
            if(B != 2)
            {
                if(B == 6)
                {
                    NPC[A].Location.Y += (NPC[A].Location.Height - 32);
                    PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                    NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                    if(!NPC[A].NoLavaSplash)
                        NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
                }
                else
                {
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                    NewEffect(EFFID_POWER_S3_DIE, NPC[A].Location, NPC[A].Direction);
                }
            }
        }
        else if(NPC[A].Type == NPCID_ITEM_POD) // yoshi egg
        {
            C = NPC[A].Special;
            CharStuff(A, true);
            if(NPC[A].Special == 287)
                NPC[A].Special = RandomBonus();
            NewEffect(EFFID_ITEM_POD_OPEN, NPC[A].Location, 1, (int)SDL_floor(NPC[A].Special), false, NPC[A].Variant);
            if(C == 98)
                Effect[numEffects].Frame += 3;
            else if(C == 99)
                Effect[numEffects].Frame += 5;
            else if(C == 100)
                Effect[numEffects].Frame += 7;
            else if(C == 148)
                Effect[numEffects].Frame += 9;
            else if(C == 149)
                Effect[numEffects].Frame += 11;
            else if(C == 150)
                Effect[numEffects].Frame += 13;
            else if(C == 228)
                Effect[numEffects].Frame += 15;
        }
        else if(NPC[A].Type == NPCID_BIG_FODDER) // giagnormous goomba
        {
            if(B == 1)
                NewEffect(EFFID_BIG_FODDER_SQUISH, NPC[A].Location);
            else if(B == 6)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_BIG_FODDER_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_UNDER_FODDER) // Grey goomba
        {
            if(B == 1)
                NewEffect(EFFID_UNDER_FODDER_SQUISH, NPC[A].Location);
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                if(B != 5)
                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                NewEffect(EFFID_UNDER_FODDER_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_EXT_TURTLE) // nekkid koopa
        {
            if(B == 1)
                NewEffect(EFFID_EXT_TURTLE_SQUISH, NPC[A].Location);
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                if(B != 5)
                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                NewEffect(EFFID_EXT_TURTLE_DIE, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == NPCID_SKELETON) // Dry Bones
        {
            NPC[A].Location.Width = 48;
            NPC[A].Location.X -= 8;
            if(B == 6)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_SKELETON_DIE, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4) // SMW Beach Koopas
        {
            if(B == 1)
            {
                NewEffect(EFFID_HIT_TURTLE_S4_SQUISH, NPC[A].Location);
                Effect[numEffects].Frame = NPC[A].Type - 117;
            }
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                if(B != 5)
                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                NewEffect(EFFID_HIT_TURTLE_S4_DIE, NPC[A].Location, NPC[A].Direction);
                Effect[numEffects].Frame = (NPC[A].Type - 117) * 4;
                if(NPC[A].Direction == 1)
                    Effect[numEffects].Frame += 2;
            }
        }
        else if(NPC[A].Type == NPCID_GRN_TURTLE_S3 || NPC[A].Type == NPCID_GRN_SHELL_S3 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S3) // Green Koopa
        {
             NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
             NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_GRN_SHELL_S3_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_WALL_TURTLE) // spike top
        {
            if(B == 6)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_WALL_TURTLE_DIE, NPC[A].Location, static_cast<double>(NPC[A].Frame));
            }
        }
        else if(NPC[A].Type == NPCID_GRN_SHELL_S1 || NPC[A].Type == NPCID_GRN_TURTLE_S1 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S1) // smb1 Green Koopa
        {
             NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
             NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_GRN_SHELL_S1_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_RED_SHELL_S1 || NPC[A].Type == NPCID_RED_TURTLE_S1 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S1) // smb1 red Koopa
        {
             NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
             NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_RED_SHELL_S1_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_EARTHQUAKE_BLOCK)
        {
            Controls::RumbleAllPlayers(200, 1.0);

            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else
                NewEffect(125 , NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_BIG_TURTLE || NPC[A].Type == NPCID_BIG_SHELL) // giant Green Koopa
        {
             NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[8] / 2.0;
             NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[8] / 2.0;
            if(B == 6)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_BIG_SHELL_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_SPIT_BOSS) // Birdo
        {
            PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
            if(NPC[A].Legacy && !LevelEditor)
            {
                for(C = 1; C <= numNPCs; C++)
                {
                    if(NPC[C].Type == NPCID_SPIT_BOSS && C != A)
                    {
                        DontSpawnExit = true;
                        break;
                    }
                }

                if(!DontSpawnExit)
                {
                    numNPCs++;
                    NPC[numNPCs] = NPC_t();
                    NPC[numNPCs].Type = NPCID_GOALORB_S2;
                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                    NPC[numNPCs].Location.X = NPC[A].Location.X;
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                    NPC[numNPCs].Location.SpeedY = -6;
                    NPC[numNPCs].Active = true;
                    NPC[numNPCs].TimeLeft = 100;
                    NPC[numNPCs].Frame = 0;
                    syncLayers_NPC(numNPCs);
                    CheckSectionNPC(numNPCs);
                    PlaySoundSpatial(SFX_SpitBossBeat, NPC[A].Location);
                }
                else
                {
                    for(int nc : NPCQueues::Active.no_change)
                    {
                        if(NPC[nc].Type == NPCID_SPIT_BOSS && NPC[nc].Active && nc != A)
                        {
                            DontResetMusic = true;
                            break;
                        }
                    }

                    if(!DontResetMusic)
                    {
                        bgMusic[NPC[A].Section] = bgMusicREAL[NPC[A].Section];
                        StopMusic();
                        StartMusic(NPC[A].Section);
                    }
                }
            }

            NPC[A].Location.Y += -NPC[A].Location.Height / 2.0 + 32;
            NPC[A].Location.X += -NPC[A].Location.Width / 2.0 + 20;
            NewEffect(EFFID_SPIT_BOSS_DIE, NPC[A].Location, NPC[A].Direction);
        }
        else if(NPC[A].Type == NPCID_SPIT_BOSS_BALL) // Egg
        {
            if(NPC[A].Special == 1)
                NPC[A].Location.SpeedY = -5.1;
            NewEffect(EFFID_SPIT_BOSS_BALL_DIE, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_RED_TURTLE_S3 || NPC[A].Type == NPCID_RED_SHELL_S3 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S3) // Red Koopa
        {
             NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[9] / 2.0;
             NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[9] / 2.0;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                 NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                 NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_RED_SHELL_S3_DIE, NPC[A].Location);
            }

        }
        else if((NPC[A].Type >= NPCID_GRN_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_SHELL_S4) || (NPC[A].Type >= NPCID_GRN_FLY_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_FLY_TURTLE_S4)) // SMW Koopas
        {
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_SHELL_S4_DIE, NPC[A].Location);
                if(NPC[A].Type <= NPCID_YEL_TURTLE_S4)
                    Effect[numEffects].Frame = NPC[A].Type - 109;
                else if(NPC[A].Type <= NPCID_YEL_SHELL_S4)
                    Effect[numEffects].Frame = NPC[A].Type - 113;
                else
                    Effect[numEffects].Frame = NPC[A].Type - 121;
            }
        }
        else if(NPC[A].Type == NPCID_GLASS_TURTLE || NPC[A].Type == NPCID_GLASS_SHELL || NPC[A].Type == NPCID_SPIKY_S3 || NPC[A].Type == NPCID_CRAB || NPC[A].Type == NPCID_FLY || NPC[A].Type == NPCID_SPIKY_S4 || NPC[A].Type == NPCID_SPIKY_BALL_S4) // Hard thing / Spiney
        {
            NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[9] / 2.0;
            NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[9] / 2.0;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                if(NPC[A].Type == NPCID_SPIKY_S3)
                    NewEffect(EFFID_SPIKY_S3_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_SPIKY_S4 || NPC[A].Type == NPCID_SPIKY_BALL_S4)
                    NewEffect(EFFID_SPIKY_S4_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_CRAB)
                    NewEffect(EFFID_CRAB_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_FLY)
                    NewEffect(EFFID_FLY_DIE, NPC[A].Location);
                else
                    NewEffect(EFFID_GLASS_SHELL_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_HEAVY_THROWN) // Hammer
        {
            if(B == 3)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_LONG_PLANT_UP || NPC[A].Type == NPCID_LONG_PLANT_DOWN)
        {
            NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
            tempLocation = NPC[A].Location;
            if(NPC[A].Type == NPCID_LONG_PLANT_DOWN)
            {
                int npcH = NPC[A].Location.Height;
                for(C = 0; C <= npcH; C += 32)
                {
                    if(NPC[A].Location.Height - C > 16)
                    {
                        tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 32 - C;
                        NewEffect(EFFID_SMOKE_S3, tempLocation);
                    }
                }
            }
            else
            {
                int npcH = NPC[A].Location.Height;
                for(C = 0; C <= npcH; C += 32)
                {
                    if(NPC[A].Location.Height - C > 16)
                    {
                        tempLocation.Y = NPC[A].Location.Y + C;
                        NewEffect(EFFID_SMOKE_S3, tempLocation);
                    }
                }
            }
            PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
        }
        else if(NPC[A].Type == NPCID_PLANT_S3 || NPC[A].Type == NPCID_QUAD_SPITTER || NPC[A].Type == NPCID_PLANT_S1 || NPC[A].Type == NPCID_LAVABUBBLE || NPC[A].Type == NPCID_BOTTOM_PLANT || NPC[A].Type == NPCID_SIDE_PLANT || NPC[A].Type == NPCID_BIG_PLANT || NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_GHOST_S3 || NPC[A].Type == NPCID_GHOST_FAST || NPC[A].Type == NPCID_GHOST_S4 || NPC[A].Type == NPCID_BIG_GHOST || NPC[A].Type == NPCID_FIRE_PLANT || NPC[A].Type == NPCID_JUMP_PLANT) // Piranha Plant / Fireball
        {
            NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
            NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
            NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            if(B == 8)
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            else
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
        }
        else if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PET_FIRE || NPC[A].Type == NPCID_PLR_ICEBALL) // Small Fireball / Yoshi Fireball
        {
            if(B == 6)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[9] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[9] / 2.0;
                NPC[A].Location.Width = 32;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location, 1, 0, NPC[A].Shadow);
                NPC[A].Location.X += 2;
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else
            {
                for(C = 1; C <= 10; C++)
                {
                    if(NPC[A].Type == NPCID_PLR_ICEBALL)
                        NewEffect(EFFID_PLR_ICEBALL_TRAIL, NPC[A].Location, static_cast<float>(NPC[A].Special), 0, NPC[A].Shadow);
                    else
                        NewEffect(EFFID_PLR_FIREBALL_TRAIL, NPC[A].Location, static_cast<float>(NPC[A].Special), 0, NPC[A].Shadow);
                    Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5 + NPC[A].Location.SpeedX * 0.1;
                    Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5 - NPC[A].Location.SpeedY * 0.1;
                }
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                if((NPC[A].Type == NPCID_PLR_FIREBALL && NPC[A].Special == 5) || NPC[A].Type == NPCID_PET_FIRE)
                    NewEffect(EFFID_SMOKE_S3, NPC[A].Location, 1, 0, NPC[A].Shadow);
                else
                    NewEffect(EFFID_SMOKE_S4, NPC[A].Location, 1, 0, NPC[A].Shadow);
            }
        }
        else if(NPC[A].Type == NPCID_MINIBOSS) // Big Koopa
        {
            NPC[A].Location.Y += -(NPC[A]->THeight - NPC[A].Location.Height);
            NPC[A].Location.Height = NPC[A]->THeight;
            if(NPC[A].Legacy)
            {
                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Type == NPCID_MINIBOSS && NPC[B].Killed == 0 && B != A)
                    {
                        DontSpawnExit = true;
                        break;
                    }
                }

                if(LevelEditor)
                    DontSpawnExit = true;

                if(!DontSpawnExit)
                    NewEffect(EFFID_MINIBOSS_DIE, NPC[A].Location, 1, 16);
                else
                {
                    NewEffect(EFFID_MINIBOSS_DIE, NPC[A].Location);

                    for(int nc : NPCQueues::Active.no_change)
                    {
                        if(NPC[nc].Type == NPCID_MINIBOSS && NPC[nc].Active && nc != A && NPC[nc].Killed == 0)
                        {
                            DontResetMusic = true;
                            break;
                        }
                    }

                    if(!DontResetMusic)
                    {
                        bgMusic[NPC[A].Section] = bgMusicREAL[NPC[A].Section];
                        StopMusic();
                        StartMusic(Player[1].Section);
                    }
                }
            }
            else
                NewEffect(EFFID_MINIBOSS_DIE, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET) // Bullet Bills
        {
            NPC[A].Location.SpeedX = NPC[A].Location.SpeedX / 2;
            if(B == 1)
                NPC[A].Location.SpeedX = 0.0001 * NPC[A].Direction;
            else if(B == 5)
                NPC[A].Location.SpeedX = 3 * NPC[A].Direction;
            else
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX / 2;

            if(B != 1)
                NPC[A].Location.SpeedY = -9;

            if(B == 8)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                if(B == 3 || B == 2)
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                if(NPC[A].Type == NPCID_BULLET)
                    NewEffect(EFFID_BULLET_DIE, NPC[A].Location, NPC[A].Direction);
                else
                    NewEffect(EFFID_BIG_BULLET_DIE, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type == NPCID_STACKER || NPC[A].Type == NPCID_BLU_GUY || NPC[A].Type == NPCID_RED_GUY || NPC[A].Type == NPCID_JUMPER_S3 || NPC[A].Type == NPCID_RED_FISH_S1 || NPC[A].Type == NPCID_HEAVY_THROWER || NPC[A].Type == NPCID_SPIKY_THROWER || NPC[A].Type == NPCID_ITEM_THROWER || NPC[A].Type == NPCID_SPIKY_BALL_S3 || NPC[A].Type == NPCID_JUMPER_S4 || NPC[A].Type == NPCID_BAT || (NPC[A].Type >= NPCID_BIRD && NPC[A].Type <= NPCID_GRY_SPIT_GUY) || NPC[A].Type == NPCID_SATURN || NPC[A].Type == NPCID_WALK_PLANT || NPC[A].Type == NPCID_VINE_BUG) // Misc Things
        {
            if(B == 6)
            {
                NPC[A].Location.Y += (NPC[A].Location.Height - 32);
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                if(NPC[A].Type >= NPCID_BIRD && NPC[A].Type <= NPCID_GRY_SPIT_GUY)
                    NewEffect(NPC[A].Type - NPCID_BIRD + EFFID_BIRD_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_BLU_GUY)
                    NewEffect(EFFID_RED_GUY_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_RED_GUY)
                    NewEffect(EFFID_BLU_GUY_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_WALK_PLANT)
                    NewEffect(EFFID_WALK_PLANT_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_JUMPER_S3)
                    NewEffect(EFFID_JUMPER_S3_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_RED_FISH_S1)
                {
                    NPC[A].Location.SpeedY = -11;
                    if(NPC[A].Killed == 1)
                        NPC[A].Location.SpeedY = 0;
                    NewEffect(EFFID_RED_FISH_S1_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_SATURN)
                    NewEffect(EFFID_SATURN_DIE, NPC[A].Location, NPC[A].Direction);
                else if(NPC[A].Type == NPCID_STACKER)
                    NewEffect(EFFID_STACKER_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_VINE_BUG)
                    NewEffect(EFFID_VINE_BUG_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_JUMPER_S4)
                {
                    if(B == 1)
                    {
                        NPC[A].Location.SpeedY = 0;
                        NPC[A].Location.SpeedX = 0;
                        PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                    }
                    else
                    {
                        NPC[A].Location.SpeedY = -11;
                        PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    }

                    NewEffect(EFFID_JUMPER_S4_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_BAT)
                {
                    if(B == 1)
                    {
                        NPC[A].Location.SpeedY = 0;
                        NPC[A].Location.SpeedX = 0;
                    }
                    else
                    {
                        NPC[A].Location.SpeedY = -11;
                        PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    }

                    NewEffect(EFFID_BAT_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_HEAVY_THROWER)
                {
                    if(B == 1)
                    {
                        NPC[A].Location.SpeedY = 0;
                        NPC[A].Location.SpeedX = 0;
                        PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                    }
                    else
                    {
                        NPC[A].Location.SpeedY = -11;
                        PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    }

                    NewEffect(EFFID_HEAVY_THROWER_DIE, NPC[A].Location, NPC[A].Direction);
                }
                else if(NPC[A].Type == NPCID_SPIKY_BALL_S3)
                    NewEffect(EFFID_SPIKY_BALL_S3_DIE, NPC[A].Location);
                else if(NPC[A].Type == NPCID_SPIKY_THROWER || NPC[A].Type == NPCID_ITEM_THROWER)
                {
                    if(NPC[A].Location.SpeedX > 4)
                        NPC[A].Location.SpeedX = 4;
                    if(NPC[A].Location.SpeedX < -4)
                        NPC[A].Location.SpeedX = -4;

                    if(B == 1)
                    {
                        NPC[A].Location.SpeedY = 0;
                        NPC[A].Location.SpeedX = 0;
                        PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                    }
                    else
                    {
                        NPC[A].Location.SpeedY = -11;
                        PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    }

                    if(NPC[A].Type == NPCID_ITEM_THROWER)
                        NewEffect(EFFID_ITEM_THROWER_DIE, NPC[A].Location);
                    else
                    {
                        NPC[A].Location.Y -= 14;
                        NewEffect(EFFID_SPIKY_THROWER_DIE, NPC[A].Location);
                    }
                }

                if(NPC[A].Type != NPCID_HEAVY_THROWER && NPC[A].Type != NPCID_SPIKY_THROWER && NPC[A].Type != NPCID_JUMPER_S4)
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_SLIDE_BLOCK) // ice block
        {
            if(B == 6)
            {
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
            }
            else
            {
                NewEffect(EFFID_SLIDE_BLOCK_SMASH, NPC[A].Location);
                PlaySoundSpatial(SFX_BlockSmashed, NPC[A].Location);
            }
        }
        else if(NPC[A]->IsABonus) // NPC is a bonus
        {
            if(B == 3 || B == 4 || B == 5)
            {
                if(!NPC[A]->IsACoin || LevelEditor || TestLevel) // Shell hit sound
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            }
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
        }
        else if(LevelEditor || MagicHand)
        {
            if(!(NPC[A].Type == NPCID_COIN_SWITCH && B == 1))
            {
                NPC[A].Location.X += NPC[A].Location.Width / 2.0 - EffectWidth[10] / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
        }
    }

    if(BattleMode)
    {
        NPC[A].RespawnDelay = 65 * 30;
        NPCQueues::RespawnDelay.insert(A);
    }

    if(NPC[A].AttLayer != LAYER_NONE && NPC[A].AttLayer != LAYER_DEFAULT)
    {
        // for(C = 1; C <= maxLayers; C++)
        {
            // if(NPC[A].AttLayer == Layer[C].Name)
            {
                Layer[NPC[A].AttLayer].SpeedX = 0;
                Layer[NPC[A].AttLayer].SpeedY = 0;

                // @Wohlstand, do we want to add some code to keep the climbing on BGOs working?
            }
        }
    }

    if((!GameMenu && !BattleMode) || NPC[A].DefaultType == 0)
    {
        for(B = 1; B <= numPlayers; B++) // Tell the player to stop standing on me because im dead kthnx
        {
            if(Player[B].StandingOnNPC == A)
            {
                Player[B].StandingOnNPC = 0;
                if(NPC[A].Type != NPCID_VEHICLE)
                    Player[B].Location.SpeedY = NPC[A].Location.SpeedY;
            }
            else if(Player[B].StandingOnNPC == numNPCs)
                Player[B].StandingOnNPC = A;

            if(Player[B].YoshiNPC == numNPCs)
                Player[B].YoshiNPC = A;
            if(fEqual(Player[B].VineNPC, numNPCs))
                Player[B].VineNPC = A;
        }

        SDL_assert_release(A > 0);
        NPC[A] = NPC[numNPCs];
        NPC[numNPCs] = blankNPC;
        numNPCs--;
        syncLayers_NPC(A);
        syncLayers_NPC(numNPCs + 1);

        if(NPC[A].HoldingPlayer > 0)
        {
            Player[NPC[A].HoldingPlayer].HoldingNPC = A;
//            if(nPlay.Online == true && nPlay.Mode == 1)
//                Netplay::sendData "K" + std::to_string(C) + "|" + NPC[A].Type + "|" + NPC[A].Location.X + "|" + NPC[A].Location.Y + "|" + std::to_string(NPC[A].Location.Width) + "|" + std::to_string(NPC[A].Location.Height) + "|" + NPC[A].Location.SpeedX + "|" + NPC[A].Location.SpeedY + "|" + NPC[A].Section + "|" + NPC[A].TimeLeft + "|" + NPC[A].Direction + "|" + std::to_string(static_cast<int>(floor(static_cast<double>(NPC[A].Projectile)))) + "|" + NPC[A].Special + "|" + NPC[A].Special2 + "|" + NPC[A].Special3 + "|" + NPC[A].Special4 + "|" + NPC[A].Special5 + "|" + NPC[A].Effect + LB + "1n" + NPC[A].HoldingPlayer + "|" + std::to_string(A) + "|" + NPC[A].Type + LB;
        }

        if(NPC[A].Effect == NPCEFF_PET_TONGUE || NPC[A].Effect == NPCEFF_PET_INSIDE)
            Player[NPC[A].Effect2].YoshiNPC = A;

        if(NPC[A].Type == NPCID_TOOTHYPIPE && NPC[A].Special == 1)
        {
            for(int C : NPCQueues::Active.no_change)
            {
                if(NPC[C].Type == NPCID_TOOTHY && NPC[C].Special2 == numNPCs + 1)
                    NPC[C].Special2 = A;
            }
        }
        else if(NPC[A].Type == NPCID_TOOTHYPIPE && NPC[A].Special2 > 0)
            NPC[(int)NPC[A].Special2].Special2 = A;
        else if(NPC[A].Type == NPCID_TOOTHY && NPC[A].Special2 > 0)
            NPC[(int)NPC[A].Special2].Special2 = A;
    }
    else
    {
        Deactivate(A);
    }
}
