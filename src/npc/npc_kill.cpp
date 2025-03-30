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

#include "../globals.h"
#include "../npc.h"
#include "../npc_id.h"
#include "../eff_id.h"
#include "../sound.h"
#include "../effect.h"
#include "../layers.h"
#include "../game_main.h"
#include "../main/speedrunner.h"
#include "config.h"
#include "../controls.h"
#include "../layers.h"

#include "npc_traits.h"

#include "sdl_proxy/sdl_stdinc.h"

#include "npc/npc_queues.h"

#include "main/game_loop_interrupt.h"

static void s_makeSparkles(const NPC_t& npc, int speed_random, double speed_mult)
{
    NewEffect(EFFID_SMOKE_S3, npc.Location);

    Location_t tempLocation;
    tempLocation.Height = EffectHeight[EFFID_SPARKLE];
    tempLocation.Width = EffectWidth[EFFID_SPARKLE];
    tempLocation.SpeedX = 0;
    tempLocation.SpeedY = 0;

    num_t tempLocX_base = npc.Location.X - EffectWidth[EFFID_SPARKLE] / 2;
    num_t tempLocY_base = npc.Location.Y - EffectHeight[EFFID_SPARKLE] / 2;

    for(int C = 1; C <= 50; C++)
    {
        tempLocation.X = tempLocX_base + dRand() * npc.Location.Width;
        tempLocation.Y = tempLocY_base + dRand() * npc.Location.Height;
        NewEffect(EFFID_SPARKLE, tempLocation);

        Effect[numEffects].Location.SpeedX = (dRand() - 0.5) * speed_random - npc.Location.SpeedX * speed_mult;
        Effect[numEffects].Location.SpeedY = (dRand() - 0.5) * speed_random + npc.Location.SpeedY * speed_mult;
        Effect[numEffects].Frame = iRand(3);
    }
}

bool KillNPC(int A, int B)
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
    // bool DontSpawnExit = false;
    // bool DontResetMusic = false;
    // bool tempBool = false;
    // NPC_t blankNPC;
    int C = 0;
    Location_t tempLocation;

    switch(g_gameLoopInterrupt.site)
    {
    case GameLoopInterrupt::UpdateNPCs_KillNPC:
        if(g_gameLoopInterrupt.bool1)
            goto resume_TriggerLast;
        else
            goto resume_TriggerDeath;
    default:
        break;
    }

    // don't need to worry about updating NPC A's tree because that will certainly happen in either the syncLayersNPC or the Deactivate call at the end of the procedure

    if(NPC[A].Type == NPCID_ICE_CUBE && NPC[A].Special > 0 && NPC[A].Killed != 9)
    {
        s_makeSparkles(NPC[A], 4, 0.2);
        PlaySoundSpatial(SFX_Icebreak, NPC[A].Location);
        NPC[A].Type = NPCID(NPC[A].Special);

        NPCQueues::update(A);

        if(B != 10)
        {
            MoreScore(NPC[A]->Score, NPC[A].Location, NPC[A].Multiplier);
        }

        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.4_r;
        if(NPC[A].Location.SpeedX < 2 && NPC[A].Location.SpeedX > -2)
            NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
    }

    if(NPC[A].Killed == 8 && NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_KNIGHT && !NPCIsABot(NPC[A]) && NPC[A].Type != NPCID_FLIER && NPC[A].Type != NPCID_ROCKET_FLIER && NPC[A].Type != NPCID_WALL_BUG && NPC[A].Type != NPCID_HOMING_BALL)
        NewEffect(EFFID_BOOT_STOMP, NPC[A].Location);

    if(NPC[A].Type == NPCID_YELSWITCH_FODDER || NPC[A].Type == NPCID_BLUSWITCH_FODDER || NPC[A].Type == NPCID_GRNSWITCH_FODDER || NPC[A].Type == NPCID_REDSWITCH_FODDER
        || NPC[A].DefaultType == NPCID_YELSWITCH_FODDER || NPC[A].DefaultType == NPCID_BLUSWITCH_FODDER || NPC[A].DefaultType == NPCID_GRNSWITCH_FODDER || NPC[A].DefaultType == NPCID_REDSWITCH_FODDER)
    {
        bool tempBool = false;

        for(C = 1; C <= numNPCs; C++)
        {
            // WARNING: this does not check DefaultType
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
            if(NPC[A].Type == NPCID_YELSWITCH_FODDER || NPC[A].DefaultType == NPCID_YELSWITCH_FODDER)
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
            else if(NPC[A].Type == NPCID_BLUSWITCH_FODDER || NPC[A].DefaultType == NPCID_BLUSWITCH_FODDER)
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
            else if(NPC[A].Type == NPCID_GRNSWITCH_FODDER || NPC[A].DefaultType == NPCID_GRNSWITCH_FODDER)
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
            else if(NPC[A].Type == NPCID_REDSWITCH_FODDER || NPC[A].DefaultType == NPCID_REDSWITCH_FODDER)
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
        eventindex_t resume_index;
        resume_index = ProcEvent_Safe(false, NPC[A].TriggerDeath, 0);
        while(resume_index != EVENT_NONE)
        {
            g_gameLoopInterrupt.C = resume_index;
            g_gameLoopInterrupt.bool1 = false; // marks as TriggerDeath
            return true;

resume_TriggerDeath:
            resume_index = g_gameLoopInterrupt.C;
            g_gameLoopInterrupt.site = GameLoopInterrupt::None;

            resume_index = ProcEvent_Safe(true, resume_index, 0);
        }
    }

    if(NPC[A].TriggerLast != EVENT_NONE)
    {
        bool last_in_layer;

        {
            last_in_layer = true;

            int lr = NPC[A].Layer;
            if(lr != LAYER_NONE)
            {
                for(int other_npc : Layer[lr].NPCs)
                {
                    if(other_npc != A && !NPC[other_npc].Generator)
                    {
                        last_in_layer = false;
                        break;
                    }
                }

                if(!Layer[lr].blocks.empty())
                    last_in_layer = false;
            }
        }

        if(last_in_layer)
        {
            eventindex_t resume_index;
            resume_index = ProcEvent_Safe(false, NPC[A].TriggerLast, 0);
            while(resume_index != EVENT_NONE)
            {
                g_gameLoopInterrupt.C = resume_index;
                g_gameLoopInterrupt.bool1 = true; // marks as TriggerLast
                return true;

resume_TriggerLast:
                resume_index = g_gameLoopInterrupt.C;
                g_gameLoopInterrupt.site = GameLoopInterrupt::None;

                resume_index = ProcEvent_Safe(true, resume_index, 0);
            }
        }
    }

    num_t NPC_CenterX = NPC[A].Location.X + NPC[A].Location.Width / 2;
    num_t NPC_CenterY = NPC[A].Location.Y + NPC[A].Location.Height / 2;
    num_t NPC_BottomY = NPC[A].Location.Y + NPC[A].Location.Height;

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
                NPC[numNPCs].Location.X = NPC_CenterX - NPC[numNPCs].Location.Width / 2;
                NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                if(NPC[A].Location.Height >= 32)
                    NPC[numNPCs].Location.Y = NPC_CenterY - NPC[numNPCs].Location.Height / 2;
                else if(NPC[A].Type == NPCID_BOTTOM_PLANT || NPC[A].Type == NPCID_LONG_PLANT_DOWN) // Stops the rupees from spawning in blocks
                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + 1;
                else // Stops the rupees from spawning in blocks
                    NPC[numNPCs].Location.Y = NPC_BottomY - NPC[numNPCs].Location.Height - 1;
                NPC[numNPCs].Section = NPC[A].Section;
                NPC[numNPCs].TimeLeft = Physics.NPCTimeOffScreen;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TailCD = 10;
                NPC[numNPCs].Special = 1;
                NPC[numNPCs].Location.SpeedY = -5;
                NPC[numNPCs].Location.SpeedX = (1 + dRand() / 2) * NPC[A].Direction;
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
                else if(NPC[A].Type == NPCID_SQUID_S1 && g_config.fix_squid_stomp_effect)
                {
                    NPC[A].Location.SpeedY = 0.123_n;
                    NewEffect(EFFID_SQUID_S1_DIE, NPC[A].Location, NPC[A].Direction);
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                }
                else if(NPC[A].Type == NPCID_SQUID_S3 && g_config.fix_squid_stomp_effect)
                {
                    NPC[A].Location.SpeedY = 0.123_n;
                    NewEffect(EFFID_SQUID_S3_DIE, NPC[A].Location, NPC[A].Direction);
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                }
                else
                    NewEffect(EFFID_BRUTE_SQUISH, NPC[A].Location, NPC[A].Direction);
            }
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NPC[A].Location.X = NPC_CenterX - 16;
                NPC[A].Location.Width = 32;
                NPC[A].Location.Y = NPC_BottomY - 32;
                NPC[A].Location.Height = 32;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
                    s_makeSparkles(NPC[A], 2, 0.3);
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
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_BottomY - EffectHeight[EFFID_SMOKE_S3];
                NPC[A].Location.Width = 32;
                NPC[A].Location.Height = 32;
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
            NPC[A].Location.Y = NPC_BottomY - 32;
            NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
            NPC[A].Location.Y += NPC[A].Location.Height / 2 - EffectHeight[EFFID_SMOKE_S3] / 2;
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
                NewEffect(EFFID_TNT_PRESS, NPC[A].Location);
            }
            else if(B == 2)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            }
        }
        else if((NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_STATUE_S4) && B == 6) // lava only
        {
            NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
            NPC[A].Location.Y = NPC_BottomY - 32;
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
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_BottomY - 32;
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
            NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX * 0.3_r;
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_SMOKE_S3] / 2;
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
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_SMOKE_S3] / 2;
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
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_SMOKE_S3] / 2;
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
                bool tempBool = false;
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
            if(NPC[A].Special == NPCID_RANDOM_POWER)
                NPC[A].Special = RandomBonus();
            NewEffect(EFFID_ITEM_POD_OPEN, NPC[A].Location, 1, NPC[A].Special, false, NPC[A].Variant);
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
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_BottomY - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
            if(B == 6)
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_BottomY - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else
            {
                NPC[A].Location.Width = 48;
                NPC[A].Location.X -= 8;
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NewEffect(EFFID_SKELETON_DIE, NPC[A].Location, NPC[A].Direction);
            }
        }
        else if(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4) // SMW Beach Koopas
        {
            if(B == 1)
            {
                NewEffect(EFFID_HIT_TURTLE_S4_SQUISH, NPC[A].Location);
                Effect[numEffects].Frame = NPC[A].Type - NPCID_GRN_HIT_TURTLE_S4;
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                if(B != 5)
                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                NewEffect(EFFID_HIT_TURTLE_S4_DIE, NPC[A].Location, NPC[A].Direction);
                Effect[numEffects].Frame = (NPC[A].Type - NPCID_GRN_HIT_TURTLE_S4) * 4;
                if(NPC[A].Direction == 1)
                    Effect[numEffects].Frame += 2;
            }
        }
        else if(NPC[A].Type == NPCID_GRN_TURTLE_S3 || NPC[A].Type == NPCID_GRN_SHELL_S3 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S3) // Green Koopa
        {
            NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
            NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_SMOKE_S3] / 2;
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
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_WALL_TURTLE_DIE, NPC[A].Location, NPC[A].Frame);
            }
        }
        else if(NPC[A].Type == NPCID_GRN_SHELL_S1 || NPC[A].Type == NPCID_GRN_TURTLE_S1 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S1) // smb1 Green Koopa
        {
            NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
            NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_SMOKE_S3] / 2;
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
            NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
            NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_SMOKE_S3] / 2;
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
                NewEffect(EFFID_EARTHQUAKE_BLOCK_HIT, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_BIG_TURTLE || NPC[A].Type == NPCID_BIG_SHELL) // giant Green Koopa
        {
            if(B == 6)
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_BottomY - 32;
                NPC[A].Location.Height = 32;
                NPC[A].Location.Width = 32;
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
            else
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_GRN_SHELL_S3_DIE] * 0.5_n;
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
                NewEffect(EFFID_BIG_SHELL_DIE, NPC[A].Location);
            }
        }
        else if(NPC[A].Type == NPCID_SPIT_BOSS) // Birdo
        {
            PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
            if(NPC[A].Legacy && !LevelEditor)
            {
                bool DontSpawnExit = false;

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
                    bool DontResetMusic = false;

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
                        StartMusicIfOnscreen(NPC[A].Section);
                    }
                }
            }

            NPC[A].Location.Y += -NPC[A].Location.Height / 2 + 32;
            NPC[A].Location.X += -NPC[A].Location.Width / 2 + 20;
            NewEffect(EFFID_SPIT_BOSS_DIE, NPC[A].Location, NPC[A].Direction);
        }
        else if(NPC[A].Type == NPCID_SPIT_BOSS_BALL) // Egg
        {
            if(NPC[A].Special == 1)
                NPC[A].Location.SpeedY = -5.1_n;
            NewEffect(EFFID_SPIT_BOSS_BALL_DIE, NPC[A].Location);
        }
        else if(NPC[A].Type == NPCID_RED_TURTLE_S3 || NPC[A].Type == NPCID_RED_SHELL_S3 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S3) // Red Koopa
        {
            NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_RED_SHELL_S3_DIE] * 0.5_n;
            NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_RED_SHELL_S3_DIE] * 0.5_n;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_SMOKE_S3] / 2;
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
                    Effect[numEffects].Frame = NPC[A].Type - NPCID_GRN_TURTLE_S4;
                else if(NPC[A].Type <= NPCID_YEL_SHELL_S4)
                    Effect[numEffects].Frame = NPC[A].Type - NPCID_GRN_SHELL_S4;
                else
                    Effect[numEffects].Frame = NPC[A].Type - NPCID_GRN_FLY_TURTLE_S4;
            }
        }
        else if(NPC[A].Type == NPCID_GLASS_TURTLE || NPC[A].Type == NPCID_GLASS_SHELL || NPC[A].Type == NPCID_SPIKY_S3 || NPC[A].Type == NPCID_CRAB || NPC[A].Type == NPCID_FLY || NPC[A].Type == NPCID_SPIKY_S4 || NPC[A].Type == NPCID_SPIKY_BALL_S4) // Hard thing / Spiney
        {
            NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_RED_SHELL_S3_DIE] * 0.5_n;
            NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_RED_SHELL_S3_DIE] * 0.5_n;
            if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
            else if(B == 8)
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_SMOKE_S3] / 2;
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
            NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_SMOKE_S3] / 2;
            tempLocation = NPC[A].Location;
            if(NPC[A].Type == NPCID_LONG_PLANT_DOWN)
            {
                int npcH = (int)NPC[A].Location.Height;
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
                int npcH = (int)NPC[A].Location.Height;
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
            NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
            if(B == 8)
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            else
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location); // Shell hit sound
        }
        else if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PET_FIRE || NPC[A].Type == NPCID_PLR_ICEBALL) // Small Fireball / Yoshi Fireball
        {
            if(B == 6)
            {
                NPC[A].Location.X = NPC_CenterX - EffectWidth[EFFID_RED_SHELL_S3_DIE] * 0.5_n;
                NPC[A].Location.Y = NPC_CenterY - EffectHeight[EFFID_RED_SHELL_S3_DIE] * 0.5_n;
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
                        NewEffect(EFFID_PLR_ICEBALL_TRAIL, NPC[A].Location, NPC[A].Special, 0, NPC[A].Shadow);
                    else
                        NewEffect(EFFID_PLR_FIREBALL_TRAIL, NPC[A].Location, NPC[A].Special, 0, NPC[A].Shadow);

                    Effect[numEffects].Location.SpeedX = dRand() * 3 - 1.5_n + NPC[A].Location.SpeedX / 10;
                    Effect[numEffects].Location.SpeedY = dRand() * 3 - 1.5_n - NPC[A].Location.SpeedY / 10;
                }
                if((NPC[A].Type == NPCID_PLR_FIREBALL && NPC[A].Special == 5) || NPC[A].Type == NPCID_PET_FIRE)
                    NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location, 1, 0, NPC[A].Shadow);
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
                bool DontSpawnExit = false;

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
                    NewEffect(EFFID_MINIBOSS_DIE, NPC[A].Location, 1, NPCID_GOALORB_S3);
                else
                {
                    bool DontResetMusic = false;

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
                        StartMusicIfOnscreen(NPC[A].Section);
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
                NPC[A].Location.SpeedX = 0.0001_n * NPC[A].Direction;
            else if(B == 5)
                NPC[A].Location.SpeedX = 3 * NPC[A].Direction;
            else
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX / 2;

            if(B != 1)
                NPC[A].Location.SpeedY = -9;

            if(B == 8)
            {
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
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
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
            }
            else if(B == 6)
            {
                PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
                if(!NPC[A].NoLavaSplash)
                    NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            }
        }
        else if(LevelEditor || MagicHand)
        {
            if(!(NPC[A].Type == NPCID_COIN_SWITCH && B == 1))
            {
                NewEffect(EFFID_SMOKE_S3_CENTER, NPC[A].Location);
                PlaySoundSpatial(SFX_Smash, NPC[A].Location);
            }
        }
    }

    if(BattleMode)
    {
        NPC[A].RespawnDelay = 65 * 30;
        NPCQueues::RespawnDelay.insert(A);
    }

    // note: this is the "defective" version of SetLayerSpeed, which (in classic mode) only sets the speed to 0, and does nothing else
    if(NPC[A].AttLayer != LAYER_NONE && NPC[A].AttLayer != LAYER_DEFAULT)
        SetLayerSpeed(NPC[A].AttLayer, 0, 0, false, true);

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
            if(Player[B].VineNPC == numNPCs)
                Player[B].VineNPC = A;
        }

        SDL_assert_release(A > 0);
        NPC[A] = NPC[numNPCs];
        NPC[numNPCs] = NPC_t();
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
            NPC[NPC[A].Special2].Special2 = A;
        else if(NPC[A].Type == NPCID_TOOTHY && NPC[A].Special2 > 0)
            NPC[NPC[A].Special2].Special2 = A;
    }
    else
    {
        Deactivate(A);
    }

    return false;
}
