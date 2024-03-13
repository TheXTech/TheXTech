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

#include "compat.h"

#include "../globals.h"
#include "../npc.h"
#include "../sound.h"
#include "../collision.h"
#include "../effect.h"
#include "../editor.h"
#include "../game_main.h"
#include "../blocks.h"
#include "../graphics.h"
#include "../npc_id.h"
#include "../eff_id.h"
#include "../layers.h"

#include "npc_traits.h"

#include "npc/npc_queues.h"
#include "main/trees.h"

#include <Logger/logger.h>

void NPCHit(int A, int B, int C)
{
    NPC_t tempNPC;
    Location_t tempLocation;

    // NEWLY used to check when NPC's tree location needs to be updated
    NPC_t oldNPC = NPC[A];

    // if(B == 1 && C != 0)
    //     Controls::Rumble(C, 50, .25);

    // ------+  HIT CODES  +-------
    // B = 1      Jumped on by a player (or kicked)
    // B = 2      Hit by a shaking block
    // B = 3      Hit by projectile
    // B = 4      Hit something as a projectile
    // B = 5      Hit something while being held
    // B = 6      Touched a lava block
    // B = 7      Hit by a tail
    // B = 8      Stomped by Boot
    // B = 9      Fell of a cliff
    // B = 10     Link stab
    // Frost Bolt check
    if(B == 3 && NPC[A].Type != NPCID_ICE_CUBE && NPC[A].Type != NPCID_PLR_ICEBALL)
    {
        if(NPC[C].Type == NPCID_PLR_ICEBALL && NPC[A].Location.Width > 8 && NPC[A].Location.Height > 8)
        {
            if(NPC[A].Type == NPCID_ITEM_BUBBLE)
                NPCHit(A, 3, B);

            if(NPC[A]->NoIceBall || NPC[A].Location.Width > 128 || NPC[A].Location.Height > 128)
                return;

            if(NPC[A].Type == NPCID_RED_FLY_FODDER)
            {
                NPC[A].Type = NPCID_RED_FODDER;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_GRN_TURTLE_S3)
            {
                NPC[A].Type = NPCID_GRN_SHELL_S3;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_RED_TURTLE_S3 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S3)
            {
                NPC[A].Type = NPCID_RED_SHELL_S3;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_BIG_TURTLE)
            {
                NPC[A].Type = NPCID_BIG_SHELL;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_GRN_FLY_TURTLE_S3)
            {
                NPC[A].Type = NPCID_GRN_SHELL_S3;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_GRN_TURTLE_S4 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S4)
            {
                NPC[A].Type = NPCID_GRN_SHELL_S4;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_RED_TURTLE_S4 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S4)
            {
                NPC[A].Type = NPCID_RED_SHELL_S4;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_BLU_TURTLE_S4 || NPC[A].Type == NPCID_BLU_FLY_TURTLE_S4)
            {
                NPC[A].Type = NPCID_BLU_SHELL_S4;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_YEL_TURTLE_S4 || NPC[A].Type == NPCID_YEL_FLY_TURTLE_S4)
            {
                NPC[A].Type = NPCID_YEL_SHELL_S4;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_RED_TURTLE_S1 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S1)
            {
                NPC[A].Type = NPCID_RED_SHELL_S1;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_GRN_TURTLE_S1 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S1)
            {
                NPC[A].Type = NPCID_GRN_SHELL_S1;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_FLY_FODDER_S5)
            {
                NPC[A].Type = NPCID_FODDER_S5;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }
            else if(NPC[A].Type == NPCID_FLY_FODDER_S3)
            {
                NPC[A].Type = NPCID_FODDER_S3;
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            }

            NPC[A].Special = NPC[A].Type;
            NPC[A].Special2 = NPC[A].Frame;
            // If .Type = 52 Or .Type = 51 Then
            NPC[A].Special3 = 1;
            NPC[A].Location.SpeedY = 0;
            // End If

            NPC[A].Location.SpeedX = 0;

            if(NPC[A].Type == NPCID_SIDE_PLANT)
            {
                if(NPC[A].Direction == -1)
                    NPC[A].Location.Width = static_cast<int>(floor(static_cast<double>(NPC[A].Location.Width))) - 0.01;
                else
                    NPC[A].Location.X = static_cast<int>(floor(static_cast<double>(NPC[A].Location.X))) + 0.01;

                NPCQueues::Unchecked.push_back(A);
            }

            NPC[A].Location.Height = static_cast<int>(floor(static_cast<double>(NPC[A].Location.Height)));
            NPC[A].Type = NPCID_ICE_CUBE;
            NPC[A].BeltSpeed = 0;
            NPC[A].Projectile = false;
            NPC[A].RealSpeedX = 0;

            NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            for(C = 1; C <= 20; C++)
            {
                tempLocation.Height = EffectHeight[80];
                tempLocation.Width = EffectWidth[80];
                tempLocation.SpeedX = 0;
                tempLocation.SpeedY = 0;
                tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
                tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
                NewEffect(EFFID_SPARKLE, tempLocation);
                Effect[numEffects].Location.SpeedX = dRand() * 2 - 1;
                Effect[numEffects].Location.SpeedY = dRand() * 2 - 1;
                Effect[numEffects].Frame = iRand(3);
            }

            PlaySoundSpatial(SFX_Freeze, NPC[A].Location);
            // NPCHit C, 3, C

            treeNPCUpdate(A);
            if(NPC[A].tempBlock > 0)
                treeNPCSplitTempBlock(A);
            return;
        }
    }

    // Online code
    //    if(nPlay.Online == true)
    //    {
    //        if(B == 1 || B == 7 || B == 8)
    //        {
    //            if(C == nPlay.MySlot + 1 || nPlay.Allow == true || C <= 0)
    //            {
    //                if(C == nPlay.MySlot + 1)
    //                    Netplay::sendData "2d" + std::to_string(A) + "|" + std::to_string(B) + "|" + std::to_string(C) + "|" + NPC[A].Type + LB;
    //            }
    //            else
    //                return;
    //        }
    //    }

    if(!NPC[A].Active)
        return;


    // Safety
    StopHit += 1;
    if(NPC[A].Killed > 0)
        return;
    if(B == 3 || B == 4)
    {
        if(NPC[C].Generator)
            return;
    }
    if((NPC[A].Type == NPCID_SQUID_S1 || NPC[A].Type == NPCID_SQUID_S3 || NPC[A]->IsFish) && B == 1)
    {
        if(Player[C].Wet > 0)
            return;
    }
    if(NPC[A].Inert || StopHit > 2 || NPC[A].Immune > 0 || NPC[A].Killed > 0 || NPC[A].Effect == NPCEFF_ENCASED || NPC[A].Generator)
        return;
    if(B == 6 && NPC[A].Killed == 6)
        return;
    if(B == 3 || B == 4 || B == 5) // Things immune to fire
    {
        if(NPC[C].Type == NPCID_PLR_FIREBALL)
        {
            if(NPC[A]->NoFireBall)
                return;
        }
    }


    if(B == 1 && NPC[A]->JumpHurt && NPC[A].Type != NPCID_ITEM_BUBBLE) // Things that don't die from jumping
        return;

    // Add it to the queue, no matter whether it will happen. Reduces code size and increases maintainability.
    NPCQueues::Killed.push_back(A);

    if(B == 10 && NPC[A].Type == NPCID_KEY)
    {
        if(Player[C].Character == 5 && !Player[C].HasKey)
        {
            NPC[A].Killed = 9;
            Player[C].HasKey = true;
            PlaySoundSpatial(SFX_HeroKey, NPC[A].Location);
            return;
        }
    }

    // Yoshi Ice
    if(NPC[A].Type == NPCID_ICE_BLOCK)
    {
        if(B != 1 && B != 7 && B != 8 && B != 4)
            NPC[A].Killed = B;
    }
    // Frozen NPC
    else if(NPC[A].Type == NPCID_ICE_CUBE)
    {
        if(B == 3 && NPC[C].Type == NPCID_PLR_FIREBALL)
        {
            NPC[A].Type = NPCID(NPC[A].Special);

            if(NPC[A].Location.SpeedX > 0)
                NPC[A].Direction = 1;
            else if(NPC[A].Location.SpeedX < 0)
                NPC[A].Direction = -1;
            else
                NPC[A].Direction = NPC[A].DefaultDirection;

            NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
            NPC[A].Special = NPC[A].DefaultSpecial;
            NPC[A].Special2 = 0;
            NPC[A].Special3 = 0;
            NPC[A].Special4 = 0;
            NPC[A].Special5 = 0;
            NPC[A].Special6 = 0;
        }
        else if(B == 3 || B == 5)
        {
            if(A == C || B == 6 || B == 5)
                NPC[A].Killed = B;

            if(B == 3 && NPC[C].Type == NPC[A].Type)
                NPC[A].Killed = B;
        }
        else if(B == 10 || B == 2)
            NPC[A].Killed = B;
    }
    // Things that link can move with his sword
    else if(B == 10 && NPC[A].Type == NPCID_BOMB && NPC[A].CantHurt == 0 && !NPC[A].Projectile) // link picks up bombs
    {
        if(Player[C].Bombs < 9)
            Player[C].Bombs += 1;
        // .Location.X += .Location.Width / 2 - EffectWidth(10) / 2
        // .Location.Y += .Location.Height / 2 - EffectHeight(10) / 2
        // NewEffect 10, .Location
        NPC[A].Killed = 9;
        PlaySoundSpatial(SFX_HeroHeart, NPC[A].Location);
    }
    else if(B == 10 && ((NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D) || NPC[A].Type == NPCID_SPRING || NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_EARTHQUAKE_BLOCK || NPC[A].Type == NPCID_ITEM_POD || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_BOMB))
    {
        PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
        NPC[A].Bouce = true;
        if((NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D) || NPC[A].Type == NPCID_CANNONITEM)
        {
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            NPC[A].Location.SpeedY = -5;
        }
        else
        {
            NPC[A].Location.SpeedX = 4 * Player[C].Direction;
            NPC[A].Location.SpeedY = -4;
        }
        NPC[A].Projectile = true;
        NPC[A].CantHurt = 30;
        NPC[A].CantHurtPlayer = C;
        NPC[A].BattleOwner = C;
        if(NPC[A].Type == NPCID_CANNONITEM)
            NPC[A].Direction = Player[C].Direction;
    }
    // SMB2 Grass
    else if(B == 10 && (NPC[A].Type == NPCID_ITEM_BURIED || NPCIsVeggie(NPC[A])))
    {
        if(NPC[A].Type == NPCID_ITEM_BURIED)
        {
            NPC[A].Location.Y += -NPC[A].Location.Height;
            NPC[A].Type = NPCID(NPC[A].Special);
        }

        PlaySoundSpatial(SFX_HeroGrass, NPC[A].Location);
        NewEffect(EFFID_SMOKE_S5, NPC[A].Location);

        if(NPC[A].Type == NPCID_BULLET)
        {
            PlaySoundSpatial(SFX_Bullet, NPC[A].Location);
            NPC[A].Location.SpeedX = 5 * Player[C].Direction;
            NPC[A].Location.Y += NPC[A].Location.Height;
        }

        NPC[A].Direction = Player[C].Direction;

        if(NPC[A].Generator)
        {
            NPC[A].Generator = false;
            NPCQueues::update(A);
        }

        NPC[A].Frame = 0;
        NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction);
        CharStuff(A);
        NPC[A].Special = 0;
        if(NPCIsYoshi(NPC[A]))
        {
            NPC[A].Special = NPC[A].Type;
            NPC[A].Type = NPCID_ITEM_POD;
        }
        if(!(NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_SPRING || NPC[A].Type == NPCID_KEY || NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_GRN_BOOT || NPC[A].Type == NPCID_RED_BOOT || NPC[A].Type == NPCID_BLU_BOOT || NPC[A].Type == NPCID_TOOTHYPIPE || NPCIsAnExit(NPC[A])))
        {
            if(!BattleMode)
                NPC[A].DefaultType = NPCID_NULL;
        }
        NPC[A].Location.Height = NPC[A]->THeight;
        NPC[A].Location.Width = NPC[A]->TWidth;
        NPC[A].Location.Y += -NPC[A].Location.Height;
        NPC[A].Location.SpeedX = (3 + dRand() * 1) * Player[C].Direction;
        if(NPC[A].Type == NPCID_BULLET)
            NPC[A].Location.SpeedX = 5 * Player[C].Direction;
        NPC[A].Location.SpeedY = -4;
        NPC[A].CantHurtPlayer = C;
        NPC[A].CantHurt = 30;
        if(NPCIsVeggie(NPC[A]) || NPC[A].Type == NPCID_HEAVY_THROWER || NPC[A].Type == NPCID_GEM_1 || NPC[A].Type == NPCID_GEM_5)
        {
            if(NPC[A].Type != NPCID_GEM_5)
                NPC[A].Type = NPCID_GEM_1;
            NPC[A].Location.SpeedX = (1 + dRand() * 0.5) * Player[C].Direction;
            NPC[A].Location.SpeedY = -5;
            if(iRand(20) < 3)
                NPC[A].Type = NPCID_GEM_5;
            if(iRand(40) < 3)
                NPC[A].Type = NPCID_GEM_20;
            NPC[A].Location.X += NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y += NPC[A].Location.Height;
            NPC[A].Location.Width = NPC[A]->TWidth;
            NPC[A].Location.Height = NPC[A]->THeight;
            NPC[A].Location.X += -NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y += -NPC[A].Location.Height;
        }
        if(NPC[A]->IsAShell)
            NPC[A].Location.SpeedX = Physics.NPCShellSpeed * Player[C].Direction;

        NPCQueues::Unchecked.push_back(A);

        NPCFrames(A);

        if(!NPC[A]->IsACoin)
            NPC[A].Projectile = true;
        else
            NPC[A].Special = 1;

        NPC[A].Immune = 10;

        if(NPC[A].Type == NPCID_BOMB)
        {
            // .Location.SpeedX = 5 * Player(C).Direction + Player(C).Location.SpeedX
            // .Location.SpeedY = -5
            NPC[A].Projectile = false;
        }

        if(Player[C].StandingOnNPC == A)
            Player[C].StandingOnNPC = 0;
    }

    // bubble
    else if(NPC[A].Type == NPCID_ITEM_BUBBLE)
        NPC[A].Special3 = 1;

    // Larry Koopa
    else if(NPC[A].Type == NPCID_MAGIC_BOSS || NPC[A].Type == NPCID_FIRE_BOSS)
    {
        if(B != 7)
            NPC[A].Immune = 10;
        if(B == 1 || B == 2 || B == 8)
        {
            NPC[A].Damage += 5;
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Special = 5;
            if(NPC[A].Type == NPCID_MAGIC_BOSS && NPC[A].Damage < 15)
                PlaySoundSpatial(SFX_MagicBossShell, NPC[A].Location);
        }
        else if(B == 3 || B == 4 || B == 5)
        {
            if(NPC[C].Type == NPCID_PLR_FIREBALL || NPC[C].Type == NPCID_PET_FIRE)
            {
                NPC[A].Damage += 1;
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            }
            else
            {
                NPCHit(C, 3, B);
                NPC[A].Special = 5;
                NPC[A].Damage += 5;
                PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                if(NPC[A].Type == NPCID_MAGIC_BOSS && NPC[A].Damage < 15)
                    PlaySoundSpatial(SFX_MagicBossShell, NPC[A].Location);
            }
        }
        else if(B == 10)
        {
            NPC[A].Damage += 2;
            PlaySoundSpatial(SFX_HeroHit, NPC[A].Location);
        }
        else if(B == 6)
            NPC[A].Killed = B;
        if(NPC[A].Damage >= 15)
            NPC[A].Killed = B;
        else if(fiEqual(NPC[A].Special, 5) && !(NPC[A].Type == NPCID_MAGIC_BOSS_SHELL || NPC[A].Type == NPCID_FIRE_BOSS_SHELL))
        {
            NPC[A].Special = 0;
            NPC[A].Special2 = 0;
            NPC[A].Special3 = 0;
            NPC[A].Special4 = 0;
            NPC[A].Special5 = 0;
            NPC[A].Special6 = 0;
            NPC[A].Location.X += NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y += NPC[A].Location.Height;
            if(NPC[A].Type == NPCID_MAGIC_BOSS)
                NPC[A].Type = NPCID_MAGIC_BOSS_SHELL;
            else
                NPC[A].Type = NPCID_FIRE_BOSS_SHELL;
            NPC[A].Location.Width = NPC[A]->TWidth;
            NPC[A].Location.Height = NPC[A]->THeight;
            NPC[A].Location.X += -NPC[A].Location.Width / 2.0;
            NPC[A].Location.Y += -NPC[A].Location.Height;
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;

            // update now because oldNPC (which we use to check for deferred update later) has been updated
            NPCQueues::Unchecked.push_back(A);
            treeNPCUpdate(A);
            if(NPC[A].tempBlock > 0)
                treeNPCSplitTempBlock(A);

            oldNPC = NPC[A];
        }

    }
    // Larry Koop Shell
    else if(NPC[A].Type == NPCID_MAGIC_BOSS_SHELL || NPC[A].Type == NPCID_FIRE_BOSS_SHELL)
    {
        if(B != 7 && B != 1 && B != 2 && B != 8)
            NPC[A].Immune = 10;
        if(B == 1 || B == 2 || B == 8)
        {
            if(B == 1 || B == 8)
            {

                if(Player[C].Location.X + Player[C].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                    Player[C].Location.SpeedX -= 3;
                else
                    Player[C].Location.SpeedX += 3;

            }
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
        }
        else if(B == 3 || B == 4 || B == 5)
        {
            if(NPC[C].Type == NPCID_PLR_FIREBALL || NPC[C].Type == NPCID_PET_FIRE)
            {
                NPC[A].Damage += 1;
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            }
            else
            {
                NPCHit(C, 3, B);
                NPC[A].Special = 5;
                NPC[A].Damage += 5;
                PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
            }
        }
        else if(B == 10)
        {
            NPC[A].Damage += 2;
            PlaySoundSpatial(SFX_HeroHit, NPC[A].Location);
        }
        else if(B == 6)
            NPC[A].Killed = B;
        if(NPC[A].Damage >= 15)
            NPC[A].Killed = B;
    }
    // Zelda 2 Locked Door
    else if(NPC[A].Type == NPCID_LOCK_DOOR)
    {
        if(B == 10)
        {
            if(Player[C].HasKey)
            {
                Player[C].HasKey = false;
                NPC[A].Killed = 3;
            }
        }
    }
    // Goomba / Nekkid Koopa
    else if(NPC[A].Type == NPCID_FODDER_S3 || NPC[A].Type == NPCID_RED_FODDER || NPC[A].Type == NPCID_FODDER_S5 || NPC[A].Type == NPCID_UNDER_FODDER ||
            NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_YELSWITCH_FODDER || NPC[A].Type == NPCID_BLUSWITCH_FODDER || NPC[A].Type == NPCID_GRNSWITCH_FODDER ||
            NPC[A].Type == NPCID_REDSWITCH_FODDER || NPC[A].Type == NPCID_BIG_FODDER || NPC[A].Type == NPCID_JUMPER_S4 || NPC[A].Type == NPCID_BAT ||
            NPC[A].Type == NPCID_FODDER_S1 || (NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4) || NPC[A].Type == NPCID_BRUTE ||
            NPC[A].Type == NPCID_BRUTE_SQUISHED || NPC[A].Type == NPCID_GRN_FISH_S3 || NPC[A].Type == NPCID_FISH_S4 || NPC[A].Type == NPCID_RED_FISH_S3 ||
            NPC[A].Type == NPCID_GOGGLE_FISH || NPC[A].Type == NPCID_GRN_FISH_S1 || NPC[A].Type == NPCID_BONE_FISH)
    {
        if(B == 1)
        {
            if(NPC[A].Type == NPCID_BRUTE)
            {
                NPC[A].Location.Y += NPC[A].Location.Height;
                NPC[A].Location.Height = 32;
                NPC[A].Type = NPCID_BRUTE_SQUISHED;
                NPC[A].Location.Y += -NPC[A].Location.Height;
                PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            }
            else if(NPC[A].Type != NPCID_BONE_FISH)
                NPC[A].Killed = B;
        }
        else if(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4)
        {
            if(B == 3 && NPC[A].CantHurt > 0)
            {
            }
            else
                NPC[A].Killed = B;
        }
        else
        {
            if(NPC[A].Type == NPCID_BONE_FISH && B == 3)
            {
                if(NPC[C].Type != NPCID_PLR_FIREBALL && NPC[C].Type != NPCID_PET_FIRE)
                    NPC[A].Killed = B;
            }
            else
                NPC[A].Killed = B;
        }
    }
    // Mother Brain
    else if(NPC[A].Type == NPCID_BOSS_FRAGILE)
    {
        // int D = 0;
        bool tempBool;
        tempBool = false;

        for(int D : treeNPCQuery(NPC[A].Location, SORTMODE_NONE))
        {
            if(NPC[D].Type == NPCID_BOSS_CASE)
            {
                if(NPC[D].Active)
                {
                    if(NPC[D].Section == NPC[A].Section)
                    {
                        if(CheckCollision(NPC[A].Location, NPC[D].Location))
                        {
                            tempBool = true;
                            NPC[A].Immune = 65;
                            break;
                        }
                    }
                }
            }
        }

        if(!tempBool)
        {
            if(B == 3 || B == 10)
            {
                if(NPC[C].Type == NPCID_PLR_FIREBALL)
                {
                }
                else
                {
                    if(NPC[C].Type == NPCID_PLR_HEAVY)
                        NPC[A].Immune = 60;
                    else
                        NPC[A].Immune = 20;
                    NPC[A].Special = 1;
                    PlaySoundSpatial(SFX_SMBossHit, NPC[A].Location);
                    NPC[A].Damage += 1;
                    if(NPC[A].Damage >= 10)
                        NPC[A].Killed = 3;
                }
            }
        }
    }
    // Metroid Cherrio
    else if(NPC[A].Type == NPCID_HOMING_BALL)
    {
        if(B == 3 || B == 4 || B == 5 || B == 7 || B == 9 || B == 10)
            NPC[A].Killed = B;
    }
    // Metroid Glass
    else if(NPC[A].Type == NPCID_BOSS_CASE)
    {
        if(B == 3)
        {
            PlaySoundSpatial(SFX_SMBlockHit, NPC[A].Location);
            if(NPC[C].Type == NPCID_PLR_FIREBALL)
            {
            }
            else
            {
                if(NPC[C].Type == NPCID_PLR_HEAVY)
                    NPC[A].Immune = 60;
                else
                    NPC[A].Immune = 20;
                NPC[A].Damage += 3;
            }
        }
        else if(B == 10)
        {
            NPC[A].Damage += 3;
            NPC[A].Immune = 10;
        }
        if(NPC[A].Damage >= 15)
            NPC[A].Killed = B;
    }
    // Metroid Floating Things
    else if(NPC[A].Type == NPCID_FLIER || NPC[A].Type == NPCID_ROCKET_FLIER)
    {
        if(B == 3)
        {
            if(NPC[C].Type != NPCID_PLR_FIREBALL && NPC[C].Type != NPCID_PET_FIRE)
                NPC[A].Killed = B;
        }
        else if(B == 2 || B == 4 || B == 5 || B == 6 || B == 9 || B == 10)
            NPC[A].Killed = B;
    }
    // Spike Top
    else if(NPC[A].Type == NPCID_WALL_TURTLE)
    {
        if(B == 3)
        {
            if(NPC[C].Type != NPCID_PLR_FIREBALL && NPC[C].Type != NPCID_PET_FIRE)
                NPC[A].Killed = B;
        }
        else if(B != 1)
            NPC[A].Killed = B;
    }
    // Metroid Crawler
    else if(NPC[A].Type == NPCID_WALL_BUG)
    {
        if(B == 3)
        {
            if(NPC[C].Type == NPCID_PLR_FIREBALL)
                NPC[A].Damage += 1;
            else
                NPC[A].Damage += 3;
            if(NPC[A].Damage >= 3)
                NPC[A].Killed = B;
            else
                PlaySoundSpatial(SFX_SMHurt, NPC[A].Location);
        }
        else if(B == 8)
        {
            NPC[A].Damage += 1;
            if(NPC[A].Damage >= 3)
                NPC[A].Killed = B;
            else
                PlaySoundSpatial(SFX_SMHurt, NPC[A].Location);
        }
        else if(B == 10)
            NPC[A].Killed = B;
        else if(B == 4 || B == 2 || B == 9 || B == 6)
            NPC[A].Killed = B;
    }
    // mouser
    else if(NPC[A].Type == NPCID_BOMBER_BOSS)
    {
        if(B == 1)
        {
        }
        else if(B == 3)
        {
            if(NPC[C].Type != NPCID_PLR_FIREBALL)
            {
                NPC[A].Damage += 5;
                NPC[A].Immune = 60;
            }
            else
                NPC[A].Damage += 1;
        }
        else if(B == 6)
        {
            NPC[A].Killed = B;
            PlaySoundSpatial(SFX_SickBossKilled, NPC[A].Location);
        }
        else if(B == 10)
        {
            NPC[A].Damage += 2;
            NPC[A].Immune = 20;
        }
        if(NPC[A].Damage >= 20)
        {
            NPC[A].Killed = 3;
            PlaySoundSpatial(SFX_SpitBossBeat, NPC[A].Location);
        }
        else if(B == 3)
        {
            if(NPC[C].Type == NPCID_PLR_FIREBALL)
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            else
                PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
        }
        else if(B == 10)
            PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
    }
    // Wart
    else if(NPC[A].Type == NPCID_SICK_BOSS)
    {
        if(B == 1)
        {
        }
        else if(B == 3)
        {
            if(NPCIsVeggie(NPC[C]))
            {
                if(NPC[A].Special == 1)
                {
                    PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                    NPC[A].Damage += 5;
                    NPC[A].Immune = 20;
                    NPC[C].Killed = 9;
                    NPCQueues::Killed.push_back(C);
                }
            }
            else
            {
                if(NPC[C].Type != NPCID_PLR_FIREBALL)
                {
                    PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                    NPC[A].Damage += 5;
                    NPC[A].Immune = 20;
                }
                else
                {
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    NPC[A].Damage += 1;
                }
            }
        }
        else if(B == 6)
        {
            NPC[A].Killed = B;
            PlaySoundSpatial(SFX_SickBossKilled, NPC[A].Location);
        }
        else if(B == 10)
        {
            PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
            NPC[A].Damage += 5;
            NPC[A].Immune = 20;
        }
    }
    // King Koopa
    else if(NPC[A].Type == NPCID_VILLAIN_S1)
    {
        if(B == 1)
        {
        }
        else if(B == 3)
        {
            NPC[A].Immune = 20;
            if(NPC[C].Type != NPCID_PLR_FIREBALL)
            {
                PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                NPC[A].Damage += 3;
            }
            else
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NPC[A].Damage += 1;
            }
        }
        else if(B == 10)
        {
            PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
            NPC[A].Immune = 20;
            NPC[A].Damage += 1;
        }
        else if(B == 6)
            NPC[A].Killed = B;
        if(NPC[A].Damage >= 12)
            NPC[A].Killed = B;
        // SMW Dry Bones
    }
    else if(NPC[A].Type == NPCID_SKELETON)
    {
        if(B == 1 || B == 8 || B == 10)
        {
            PlaySoundSpatial(SFX_Skeleton, NPC[A].Location);
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Special = 1;
            NPC[A].Special2 = 0;
            NPC[A].Inert = true;
            NPC[A].Stuck = true;
        }
        else if(B == 3)
        {
            if(NPC[C].Type == NPCID_PLR_FIREBALL || NPC[C].Type == NPCID_PET_FIRE)
            {
            }
            else
                NPC[A].Killed = B;
        }
        else
            NPC[A].Killed = B;

        // Big Boo
    }
    else if(NPC[A].Type == NPCID_BIG_GHOST)
    {
        if((B == 3 && NPC[C].Type != NPCID_PLR_FIREBALL) || B == 4)
        {
            if(B == 3 && NPC[C].Type == NPCID_SLIDE_BLOCK)
                NPCHit(C, 3, C);
            NPC[A].Damage += 1;
            NPC[A].Immune = 30;
            if(NPC[A].Damage >= 3)
                NPC[A].Killed = B;
            else
                PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
        }
        else if(B == 6)
            NPC[A].Killed = B;

        // Projectile Only Death (Ghosts, Thwomps, Etc.)
    }
    else if(NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_GHOST_S3 || NPC[A].Type == NPCID_GHOST_FAST || NPC[A].Type == NPCID_GHOST_S4 || NPC[A].Type == NPCID_STONE_S4 || NPC[A].Type == NPCID_SAW || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_FIRE_DISK)
    {
        if((B == 3 && NPC[C].Type != NPCID_PLR_FIREBALL) || B == 4)
        {
            if(NPC[A].Type != NPCID_FIRE_DISK && NPC[C].Type != NPCID_METALBARREL) // roto disks don't die form falling blocks
            {
                if(NPC[A].Type != NPCID_SAW)
                    NPC[A].Killed = B;
                if(B == 3 && (NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4 || NPC[A].Type == NPCID_FIRE_DISK))
                {
                    if(NPC[C].Location.SpeedX > 0)
                    {
                        NPC[A].Direction = 1;
                        NPC[A].Location.SpeedX = 2;
                    }
                    else
                    {
                        NPC[A].Direction = -1;
                        NPC[A].Location.SpeedX = -2;
                    }
                }
            }
        }
        else if(B == 6)
            NPC[A].Killed = B;
        else if(NPC[A].Type == NPCID_WALL_SPARK && B == 10)
            NPC[A].Killed = B;
    }
    // Mega Mole
    else if(NPC[A].Type == NPCID_BIG_MOLE)
    {
        if(B == 6 || B == 7 || B == 9 || B == 2 || B == 10)
            NPC[A].Killed = B;
        else if(B == 3)
        {
            if(NPC[C].Type != NPCID_PLR_FIREBALL)
                NPC[A].Killed = B;
        }
    }
    // SMW Goombas
    else if(NPC[A].Type == NPCID_CARRY_FODDER || NPC[A].Type == NPCID_HIT_CARRY_FODDER ||
            NPC[A].Type == NPCID_FLY_CARRY_FODDER || NPC[A].Type == NPCID_BULLY)
    {
        if(B == 1)
        {
            if(NPC[A].Type == NPCID_FLY_CARRY_FODDER)
            {
                NPC[A].Type = NPCID_CARRY_FODDER;
                if(!NPC[A].Projectile)
                {
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    NPC[A].Projectile = true;
                }
            }
            else if(NPC[A].Type == NPCID_BULLY)
            {
                PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                if(NPC[A].Special2 == 0.0)
                {
                    NPC[A].Special2 = 1;
                    NPC[A].Location.SpeedX = 4 * Player[C].Direction;
                    NPC[A].Location.SpeedY = -2;
                }
            }
            else if(NPC[A].Type == NPCID_CARRY_FODDER)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NPC[A].Projectile = true;
                NPC[A].Type = NPCID_HIT_CARRY_FODDER;
            }
            else if(NPC[A].Type == NPCID_HIT_CARRY_FODDER)
            {
                if(NPC[A].CantHurt == 0)
                {
                    NPC[A].CantHurtPlayer = C;
                    NPC[A].CantHurt = 10;
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    NPC[A].Projectile = true;
                    if(Player[C].Location.X + Player[C].Location.Width / 2.0 > NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                        NPC[A].Direction = -1;
                    else
                        NPC[A].Direction = 1;
                    NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
                    NPC[A].Location.SpeedY = -2.5;
                }
            }
            NPC[A].Special = 0;
        }
        else if(B == 2)
        {
            if(NPC[A].Type != NPCID_BULLY)
                NPC[A].Type = NPCID_HIT_CARRY_FODDER;
            NPC[A].Special = 0;
            if(NPC[A].Location.SpeedY > -4)
            {
                PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                if(NPC[A].Type == NPCID_BULLY)
                    NPC[A].Special2 = 1;
                else
                    NPC[A].Projectile = true;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
            }
        }
        else if(B == 7)
        {
            if(NPC[A].Type != NPCID_BULLY)
                NPC[A].Type = NPCID_HIT_CARRY_FODDER;
            NPC[A].Special = 0;
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            if(NPC[A].Type == NPCID_BULLY)
                NPC[A].Special2 = 1;
            else
                NPC[A].Projectile = true;
        }
        else
        {
            if(NPC[A].Type == NPCID_BULLY && (B == 3 || B == 4 || B == 8))
            {
                if(B == 3 && NPC[C].Type != NPCID_PLR_FIREBALL && NPC[C].Type != NPCID_PET_FIRE)
                {
                    NPC[A].Location.SpeedY = -7;
                    NPC[A].Killed = B;
                }
            }
            else
                NPC[A].Killed = B;
        }
    }
    // SMB2 Bomb
    else if(NPC[A].Type == NPCID_BOMB)
    {
        if(B == 9)
            NPC[A].Killed = B;
        else if(B != 8 && B != 7)
            NPC[A].Special = 10000;
    }
    // Heart bomb
    else if(NPC[A].Type == NPCID_CHAR3_HEAVY)
    {
        if(B == 9)
            NPC[A].Killed = B;
        else if(B != 1 && B != 8 && B != 7 && B != 10)
        {
            if(NPC[A].HoldingPlayer > 0)
            {
                Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                NPC[A].HoldingPlayer = 0;
            }
            NPC[A].Special4 = 1;
        }
    }
    // SMB2 Bob-omb
    else if(NPC[A].Type == NPCID_WALK_BOMB_S2)
    {
        if(B == 9)
            NPC[A].Killed = B;
        else if(B == 7)
        {
            NPC[A].Projectile = true;
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            NPC[A].Location.SpeedY = -5;
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            if(NPC[A].Special < 520)
                NPC[A].Special = 520;
        }
        else if(B != 8)
            NPC[A].Special = 10000;
    }
    // Thwomps
    else if(NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_STATUE_S4 || NPC[A].Type == NPCID_STONE_S4)
    {
        if(B == 6)
            NPC[A].Killed = B;
    }
    // Zelda NPCs
    else if(NPC[A].Type == NPCID_KNIGHT)
        NPC[A].Killed = B;
    // Zelda Bots
    else if(NPCIsABot(NPC[A]))
        NPC[A].Killed = B;
    // Switch Platforms
    else if(NPC[A].Type == NPCID_YEL_PLATFORM || NPC[A].Type == NPCID_BLU_PLATFORM || NPC[A].Type == NPCID_GRN_PLATFORM || NPC[A].Type == NPCID_RED_PLATFORM)
    {
        if(B == 9)
            NPC[A].Killed = B;
    }
    // Veggies
    else if(NPCIsVeggie(NPC[A]))
    {
        if(B == 5)
        {
            Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
            NPC[A].CantHurtPlayer = NPC[A].HoldingPlayer;
            NPC[A].HoldingPlayer = 0;
            NPC[A].CantHurt = 1000;
            NPC[A].Location.SpeedX = double(3 * -NPC[A].Direction);
            NPC[A].Location.SpeedY = -3;
            NPC[A].Projectile = true;
        }
        // Because C++, second part of this condition never gets checked
        // in VB6, it does check and causes a crash, because C is an index of block when B is 4
        else if(B == 4 && (NPC[C].Type != NPC[A].Type || A == C))
        {
            if(NPC[C].Type != NPCID_SICK_BOSS_BALL && NPC[C].Type != NPCID_SICK_BOSS)
            {
                if(NPC[A].Location.SpeedY > -4)
                    NPC[A].Location.SpeedY = -4;
                if(NPC[A].Location.SpeedX == 0.0)
                {
                    if(iRand(2) == 0)
                        NPC[A].Location.SpeedX = 2;
                    else
                        NPC[A].Location.SpeedX = -2;
                }
            }
        }
        else if(B == 6)
            NPC[A].Killed = B;
    }
    // SMB3 Bomb
    else if(NPC[A].Type == NPCID_LIT_BOMB_S3)
    {
        if(B == 1)
        {
            if(NPC[A].CantHurt == 0)
            {
                NPC[A].CantHurt = 10;
                NPC[A].CantHurtPlayer = C;
                PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                NPC[A].Projectile = true;
                if(Player[C].Location.X + Player[C].Location.Width / 2.0 > NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                    NPC[A].Direction = -1;
                else
                    NPC[A].Direction = 1;
                NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
                NPC[A].Location.SpeedY = -2;
            }
        }
        else if(B == 2)
        {
            if(NPC[A].Location.SpeedY > -4)
            {
                PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                NPC[A].Projectile = true;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
            }
        }
        else if(B == 7 || B == 10)
        {
            NPC[A].Type = NPCID_LIT_BOMB_S3;
            NPC[A].Special = 0;
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            NPC[A].Projectile = true;
        }
        else
        {
            if(B == 3)
            {
                if(NPC[C].Type != NPCID_PLR_FIREBALL && NPC[C].Type != NPCID_PET_FIRE)
                    NPC[A].Killed = B;
            }
            else if(B != 8)
                NPC[A].Killed = B;
        }
    }
    // SMB3 Bob-om
    else if(NPC[A].Type == NPCID_WALK_BOMB_S3)
    {
        if(B == 1)
        {
            NPC[A].CantHurt = 11;
            NPC[A].Type = NPCID_LIT_BOMB_S3;
            NPC[A].Special = 0;
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Projectile = true;
        }
        else if(B == 2)
        {
            if(NPC[A].Location.SpeedY > -4)
            {
                PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                NPC[A].Location.SpeedY = -5;
                NPC[A].Projectile = true;
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
            }
            NPC[A].Type = NPCID_LIT_BOMB_S3;
            NPC[A].Special = 0;
        }
        else if(B == 7 || B == 10)
        {
            NPC[A].Type = NPCID_LIT_BOMB_S3;
            NPC[A].Special = 0;
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.SpeedX = 3 * Player[C].Direction;
            NPC[A].Projectile = true;
        }
        else
        {
            if(B == 3 || B == 5)
            {
                if(NPC[C].Type != NPCID_PLR_FIREBALL && NPC[C].Type != NPCID_PET_FIRE)
                    NPC[A].Killed = B;
            }
            else if(B != 8)
                NPC[A].Killed = B;
        }
        if(NPC[A].Type == NPCID_LIT_BOMB_S3)
            NPC[A].Location.Height = 28;
    }
    // Friendly NPCs (Toad, Peach, Link, Luigi, Etc.)
    else if(NPCIsToad(NPC[A]))
    {
        if(B == 2 && NPC[A].Location.SpeedY > -4)
        {
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
        }
        else if(B == 3 || B == 4 || B == 5 || B == 6 || B == 9)
        {
            NPC[A].Killed = B;
            if(B == 3)
                NPC[A].Location.SpeedX = 2 * NPC[B].Direction;
        }
    }
    // SMB3 Red Paragoomba
    else if(NPC[A].Type == NPCID_RED_FLY_FODDER)
    {
        if(B == 1)
        {
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = 0;
            NPC[A].Type = NPCID_RED_FODDER;
            NPC[A].Frame = 0;
        }
        else
            NPC[A].Killed = B;
    }
    // SML2 Paragoomba
    else if(NPC[A].Type == NPCID_FLY_FODDER_S5)
    {
        if(B == 1)
        {
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = 0;
            NPC[A].Type = NPCID_FODDER_S5;
            NPC[A].Frame = 0;
        }
        else
            NPC[A].Killed = B;
    }
    // SMB3 Brown Paragoomba
    else if(NPC[A].Type == NPCID_FLY_FODDER_S3)
    {
        if(B == 1)
        {
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            if(NPC[A].Location.SpeedY < 0)
                NPC[A].Location.SpeedY = 0;
            NPC[A].Type = NPCID_FODDER_S3;
            NPC[A].Frame = 0;
        }
        else
            NPC[A].Killed = B;
    }
    // SMB3 Ice Block
    else if(NPC[A].Type == NPCID_SLIDE_BLOCK)
    {
        if(B == 1 || (B == 10 && !NPC[A].Projectile))
        {
            NPC[A].Special = 1;
            PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            NPC[A].Location.SpeedX = Physics.NPCShellSpeed * Player[C].Direction;
            NPC[A].CantHurt = Physics.NPCCanHurtWait;
            NPC[A].CantHurtPlayer = C;
        }
        else if(B == 6 || B == 2 || B == 5 || B == 4 || (B == 3 && NPC[A].Special == 1) || B == 9 || (B == 10 && NPC[A].Projectile))
        {
            if(B == 10)
                B = 3;
            if(B == 4)
            {
                if(C == A)
                {
                    NPC[A].Killed = B;
                    NewEffect(EFFID_WHACK, NPC[A].Location);
                    Effect[numEffects].Location.X += NPC[A].Location.SpeedX;
                    Effect[numEffects].Location.Y += NPC[A].Location.SpeedY;
                }
            }
            else if(B == 3)
            {
                if(NPC[C].Type != NPCID_PLR_FIREBALL)
                    NPC[A].Killed = B;
                if(NPC[C].Killed == 0)
                    NPCHit(C, 3, A);
            }
            else
                NPC[A].Killed = B;
        }
    }
    // Bower Statues
    else if(NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_STATUE_S4)
    {
        if(B == 2)
        {
            NPC[A].Location.Y -= 1;
            NPC[A].Location.SpeedY = -1;
        }
    }
    // Things With Shells (Koopa Troopa, Buzzy Beetle, Etc.)
    else if(NPC[A].Type == NPCID_GRN_TURTLE_S3 || NPC[A].Type == NPCID_RED_TURTLE_S3 || NPC[A].Type == NPCID_GLASS_TURTLE || NPC[A].Type == NPCID_BIG_TURTLE || NPCIsAParaTroopa(NPC[A]) || (NPC[A].Type >= NPCID_GRN_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_TURTLE_S4) || NPC[A].Type == NPCID_GRN_TURTLE_S1 || NPC[A].Type == NPCID_RED_TURTLE_S1 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S1 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S1)
    {
        if(B == 1)
        {
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Location.Y += NPC[A].Location.Height;
            NPC[A].Location.X += NPC[A].Location.Width / 2.0;
            if(NPC[A].Type == NPCID_GRN_TURTLE_S3)
                NPC[A].Type = NPCID_GRN_SHELL_S3;
            else if(NPC[A].Type == NPCID_RED_TURTLE_S3)
                NPC[A].Type = NPCID_RED_SHELL_S3;
            else if(NPC[A].Type == NPCID_BIG_TURTLE)
                NPC[A].Type = NPCID_BIG_SHELL;
            else if(NPC[A].Type == NPCID_GRN_FLY_TURTLE_S3) // winged green koopa
                NPC[A].Type = NPCID_GRN_TURTLE_S3;
            else if(NPC[A].Type == NPCID_RED_FLY_TURTLE_S3) // winged red koopa
                NPC[A].Type = NPCID_RED_TURTLE_S3;
            else if(NPC[A].Type == NPCID_GRN_FLY_TURTLE_S1) // smb1 winged green koopa
                NPC[A].Type = NPCID_GRN_TURTLE_S1;
            else if(NPC[A].Type == NPCID_RED_FLY_TURTLE_S1) // smb winged red koopa
                NPC[A].Type = NPCID_RED_TURTLE_S1;
            else if(NPC[A].Type == NPCID_GRN_TURTLE_S1) // smb1 green koopa
            {
                NPC[A].Type = NPCID_GRN_SHELL_S1;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == NPCID_RED_TURTLE_S1) // smb red koopa
            {
                NPC[A].Type = NPCID_RED_SHELL_S1;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == NPCID_GLASS_TURTLE)
                NPC[A].Type = NPCID_GLASS_SHELL;
            else if(NPC[A].Type >= NPCID_GRN_FLY_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_FLY_TURTLE_S4) // smw winged koopas
            {
                NPC[A].Type = NPCID(NPC[A].Type - 12);
                NPC[A].Special = 0;
            }
            else
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Location = NPC[A].Location;
                NPC[numNPCs].Location.Y -= 32.0;
                NPC[numNPCs].Type = NPCID(NPC[A].Type + 8);
                NPC[numNPCs].Projectile = true;
                NPC[numNPCs].Direction = Player[C].Direction;
                NPC[numNPCs].Location.SpeedY = 0;
                NPC[numNPCs].Location.SpeedX = double(Physics.NPCShellSpeed * NPC[numNPCs].Direction);
                NPC[numNPCs].Location.X += -16.0 + NPC[numNPCs].Location.SpeedX;
                CheckSectionNPC(numNPCs);
                NPC[numNPCs].CantHurtPlayer = C;
                NPC[numNPCs].CantHurt = 6;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                syncLayers_NPC(numNPCs);
                NPC[A].Type = NPCID(NPC[A].Type + 4);
            }
            NPC[A].Location.Height = NPC[A]->THeight;
            NPC[A].Location.Width = NPC[A]->TWidth;

            NPCQueues::Unchecked.push_back(A);

            NPC[A].Location.Y -= NPC[A].Location.Height;
            NPC[A].Location.X += -(NPC[A].Location.Width / 2.0) - double(NPC[A].Direction * 2.f);
            NPC[A].Location.SpeedX = 0;
            NPC[A].Location.SpeedY = 0;
            NPC[A].RealSpeedX = 0;
            NPC[A].Special = 0;
            NPC[A].Frame = 0;
            pLogDebug("Shell stomp, X distance: [%g], Y=[%g]", std::abs(NPC[numNPCs].Location.X - NPC[A].Location.X), NPC[numNPCs].Location.Y);
            if(NPC[A].Type >= NPCID_GRN_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4)
                NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
        }
        else if(B == 2 || B == 7)
        {
            PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            NPC[A].Projectile = true;
            NPC[A].Location.Y += NPC[A].Location.Height;
            NPC[A].Location.X += NPC[A].Location.Width / 2.0;
            if(NPC[A].Type == NPCID_GRN_TURTLE_S3 || NPC[A].Type == NPCID_GRN_FLY_TURTLE_S3)
                NPC[A].Type = NPCID_GRN_SHELL_S3;
            else if(NPC[A].Type == NPCID_RED_TURTLE_S3 || NPC[A].Type == NPCID_RED_FLY_TURTLE_S3)
                NPC[A].Type = NPCID_RED_SHELL_S3;
            else if(NPC[A].Type == NPCID_BIG_TURTLE)
                NPC[A].Type = NPCID_BIG_SHELL;
            else if(NPC[A].Type == NPCID_RED_FLY_TURTLE_S3) // winged red koopa
                NPC[A].Type = NPCID_RED_TURTLE_S3;
            else if(NPC[A].Type == NPCID_GRN_FLY_TURTLE_S1) // smb1 winged green koopa
            {
                NPC[A].Type = NPCID_GRN_SHELL_S1;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == NPCID_RED_FLY_TURTLE_S1) // smb winged red koopa
            {
                NPC[A].Type = NPCID_RED_SHELL_S1;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == NPCID_GRN_TURTLE_S1) // smb1 green koopa
            {
                NPC[A].Type = NPCID_GRN_SHELL_S1;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == NPCID_RED_TURTLE_S1) // smb red koopa
            {
                NPC[A].Type = NPCID_RED_SHELL_S1;
                NPC[A].Location.Height = 28;
            }
            else if(NPC[A].Type == NPCID_GLASS_TURTLE)
                NPC[A].Type = NPCID_GLASS_SHELL;
            else if(NPC[A].Type >= NPCID_GRN_FLY_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_FLY_TURTLE_S4)
            {
                NPC[A].Type = NPCID(NPC[A].Type - 12);
                NPC[A].Special = 0;
            }
            else
                NPC[A].Type = NPCID(NPC[A].Type + 4);

            if(B == 7 && NPC[A].Type >= NPCID_GRN_SHELL_S4 && NPC[A].Type <= NPCID_GRN_HIT_TURTLE_S4)
            {
                numNPCs++;
                NPC[numNPCs] = NPC_t();
                NPC[numNPCs].Location = NPC[A].Location;
                NPC[numNPCs].Location.Y -= 32.0;
                NPC[numNPCs].Type = NPCID(NPC[A].Type + 4);
                NPC[numNPCs].Projectile = true;
                NPC[numNPCs].Direction = Player[C].Direction;
                NPC[numNPCs].Location.SpeedY = 0;
                NPC[numNPCs].Location.SpeedX = double(Physics.NPCShellSpeed * NPC[numNPCs].Direction);
                NPC[numNPCs].Location.X += -(16.0 + double(32.0f * NPC[numNPCs].Direction));
                CheckSectionNPC(numNPCs);
                NPC[numNPCs].CantHurtPlayer = C;
                NPC[numNPCs].CantHurt = 6;
                NPC[numNPCs].Active = true;
                NPC[numNPCs].TimeLeft = 100;
                syncLayers_NPC(numNPCs);
            }

            NPC[A].Location.Height = NPC[A]->THeight;
            NPC[A].Location.Width = NPC[A]->TWidth;
            NPC[A].Location.Y -= NPC[A].Location.Height;
            NPC[A].Location.X += -(NPC[A].Location.Width / 2.0) - double(NPC[A].Direction * 2.f);
            NPC[A].Location.SpeedX = 0;

            NPCQueues::Unchecked.push_back(A);

            NPC[A].Special = 0;
            NPC[A].Frame = 0;
            NPC[A].Location.SpeedY = -5;
            if(B == 2)
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
        }
        else
        {
            if(B == 3)
            {
                if(!(NPC[A].Type == NPCID_GLASS_TURTLE && (NPC[C].Type == NPCID_PLR_FIREBALL || NPC[C].Type == NPCID_PET_FIRE)))
                    NPC[A].Killed = B;
            }
            else
                NPC[A].Killed = B;
        }
        if(NPC[A]->IsAShell)
            NPC[A].Stuck = false;
    }
    // SMB3 Bowser
    else if(NPC[A].Type == NPCID_VILLAIN_S3)
    {
        if(B == 9)
        {
            NPC[A].Killed = 6;
            if(NPC[A].Legacy)
            {
                bgMusic[NPC[A].Section] = 0;
                StopMusic();
            }
        }
        if(B == 1)
        {
            // PlaySound 2
        }
        else if(B == 3)
        {
            if(NPC[C].Type != NPCID_VILLAIN_S3)
            {
                NPC[A].Immune = 10;
                if(NPC[C].Type == NPCID_BULLET)
                {
                    NPC[C].Location.SpeedX = -NPC[C].Location.SpeedX;
                    NPCHit(C, 4, C);
                }
                else
                    NPCHit(C, 3, A);
                if(NPC[C].Type != NPCID_PLR_FIREBALL)
                {
                    PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                    NPC[A].Damage += 10;
                }
                else
                {
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    NPC[A].Damage += 1;
                }
            }
        }
        else if(B == 6)
            NPC[A].Killed = B;
        else if(B == 10)
        {
            NPC[A].Immune = 10;
            PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
            NPC[A].Damage += 10;
        }
        if(NPC[A].Damage >= 200)
        {
            NPC[A].Location.SpeedY = -13;
            NPC[A].Location.SpeedX = double(4 * NPC[C].Direction);
            NPC[A].Killed = B;
            if(NPC[A].Legacy)
            {
                bgMusic[NPC[A].Section] = 0;
                StopMusic();
            }
        }
    }
    // SMW Rainbow Shell
    else if(NPC[A].Type == NPCID_RAINBOW_SHELL)
    {
        if(B == 1)
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
        else if(B == 2 || B == 7)
        {
            PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            NPC[A].Location.SpeedY = -5;
            if(B == 7)
            {
                NPC[A].Type = NPCID_FLIPPED_RAINBOW_SHELL;
                NPC[A].Location.SpeedX = 0;
                MoreScore(6, NPC[A].Location, Player[C].Multiplier);
            }
        }
        else if(B == 6)
        {
            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 2;
            tempLocation.X = NPC[A].Location.X - 4 + dRand() * (NPC[A].Location.Width + 8) - 4;
            NewEffect(EFFID_SKID_DUST, tempLocation);
        }
        else if(B == 8)
        {
            NPC[A].Killed = 8;
            PlaySoundSpatial(SFX_Smash, NPC[A].Location);
        }
    }
    // Shells
    else if(NPC[A].Type == NPCID_GRN_SHELL_S3 || NPC[A].Type == NPCID_RED_SHELL_S3 || NPC[A].Type == NPCID_GLASS_SHELL || NPC[A].Type == NPCID_BIG_SHELL || (NPC[A].Type >= NPCID_GRN_SHELL_S4 && NPC[A].Type <= NPCID_YEL_SHELL_S4) || NPC[A].Type == NPCID_RED_SHELL_S1 || NPC[A].Type == NPCID_GRN_SHELL_S1 || NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL)
    {
        if(B == 1)
        {
            if(NPC[A].Effect == NPCEFF_DROP_ITEM)
                NPC[A].Effect = NPCEFF_NORMAL;

            if(Player[C].Dismount <= 0 && Player[C].Mount != 2)
            {
                if(NPC[A].Location.SpeedX == 0.0 && NPC[A].CantHurtPlayer != C)
                {
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                    NPC[A].Location.SpeedX = double(Physics.NPCShellSpeed * Player[C].Direction);
                    NPC[A].CantHurt = Physics.NPCCanHurtWait;
                    NPC[A].CantHurtPlayer = C;
                    NPC[A].Projectile = true;
                    NPC[A].Location.SpeedY = 0;
                }
                else if(NPC[A].CantHurtPlayer != C || (NPC[A].Slope == 0 && Player[C].Vine == 0))
                {
                    PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                    NPC[A].Location.SpeedX = 0;
                    NPC[A].Location.SpeedY = 0;
                    if(NPC[A].Wet > 0)
                    {
                        NPC[A].RealSpeedX = 0;
                        NPC[A].Projectile = false;
                    }
                }
            }
        }
        else if(B == 2 || B == 7)
        {
            PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            NPC[A].Projectile = true;
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.SpeedX = 0;
        }
        else if(B == 6)
        {
            if(NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL)
            {
                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 2;
                tempLocation.X = NPC[A].Location.X - 4 + dRand() * (NPC[A].Location.Width + 8) - 4;
                NewEffect(EFFID_SKID_DUST, tempLocation);
            }
            else
                NPC[A].Killed = B;
        }
        else if(B != 4)
        {
            if(B == 3)
            {
                if(!(NPC[A].Type == NPCID_GLASS_SHELL && (NPC[C].Type == NPCID_PLR_FIREBALL || NPC[C].Type == NPCID_PET_FIRE)))
                    NPC[A].Killed = B;
            }
            else
                NPC[A].Killed = B;
        }
        else if(B == 4)
        {
            if(NPC[C].Projectile && !(NPC[C].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[C].Type <= NPCID_YEL_HIT_TURTLE_S4))
            {
                // FIXME: Why the condition is here if it always assigns B? [PVS Studio]
#if 0
                if(!(NPC[A].Type == NPCID_GLASS_SHELL && NPC[C].Type == NPCID_PLR_FIREBALL))
                    NPC[A].Killed = B;
                else
                    NPC[A].Killed = B;
#endif
                NPC[A].Killed = B;
            }
        }

        if(NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL)
        {
            NPC[A].Killed = 0;
            if(B == 5)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NPC[A].Location.SpeedX = Physics.NPCShellSpeed * -NPC[A].Direction;
                NPC[A].Location.X += NPC[A].Location.SpeedX;
                NPC[A].CantHurt = Physics.NPCCanHurtWait;
                NPC[A].CantHurtPlayer = C;

                // NEW code: does not alter behavior, but keeps the value in uint8_t range
                if(C > maxPlayers)
                    NPC[A].CantHurtPlayer = maxPlayers + 1;

                NPC[A].Projectile = true;
                NPC[A].Location.SpeedY = 0;
                Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                NPC[A].HoldingPlayer = 0;
            }
        }
    }
    // Big Piranha Plant
    else if(NPC[A].Type == NPCID_LONG_PLANT_UP || NPC[A].Type == NPCID_LONG_PLANT_DOWN)
    {
        if(!(B == 1 || B == 2 || B == 6))
        {
            if(NPC[A].Special3 == 0.0)
            {
                if(B == 3)
                {
                    if(NPC[C].Type == NPCID_PLR_FIREBALL)
                    {
                        PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                        NPC[A].Damage += 1;
                        NPC[A].Special3 = 10;
                        if(NPC[A].Special2 == 2)
                            NPC[A].Special = 50;
                    }
                    else
                    {
                        NPC[A].Damage += 3;
                        PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                        NPC[A].Special3 = 30;
                        if(NPC[A].Special2 == 2)
                            NPC[A].Special = 50;
                    }
                }
                else if(B == 10)
                {
                    NPC[A].Damage += 2;
                    PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                    NPC[A].Special3 = 10;
                    if(NPC[A].Special2 == 2)
                        NPC[A].Special = 50;
                }
            }
            if(NPC[A].Damage >= 6)
                NPC[A].Killed = B;
        }
    }

    // Piranha Plants
    else if(NPC[A].Type == NPCID_PLANT_S3 || NPC[A].Type == NPCID_QUAD_SPITTER || NPC[A].Type == NPCID_PLANT_S1 || NPC[A].Type == NPCID_BOTTOM_PLANT || NPC[A].Type == NPCID_SIDE_PLANT || NPC[A].Type == NPCID_BIG_PLANT || NPC[A].Type == NPCID_FIRE_PLANT || NPC[A].Type == NPCID_JUMP_PLANT)
    {
        if(!(B == 1 || B == 2 || B == 6))
        {
            if(B != 8 && B != 10)
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            NPC[A].Killed = B;
        }
    }
    // Podoboo
    else if(NPC[A].Type == NPCID_LAVABUBBLE)
    {
        if(B == 9)
        {
        }
        else if(B == 8)
            NPC[A].Killed = B;
        else if(B == 3 || B == 5 || B == 7)
        {
            PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            NPC[A].Killed = B;
        }
        else if(B == 4 && C > 0)
        {
            if(NPC[C].Projectile)
            {
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                NPC[A].Killed = 3;
            }
        }
    }
    // Player Fireball
    else if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL)
    {
        if(B != 7 && B != 9 && B != 2)
        {
            if(B == 3 || B == 4)
            {
                if(NPC[C].Type != NPCID_PLR_HEAVY)
                {
                    // if(B != 6) // Always true
                    if(fEqual(NPC[A].Special, 5) && HasSound(SFX_HeroFireRod))
                        PlaySoundSpatial(SFX_HeroFire, NPC[A].Location);
                    else
                        PlaySoundSpatial(SFX_BlockHit, NPC[A].Location);
                    NPC[A].Killed = B;
                }
            }
            else
            {
                if(B != 6)
                {
                    if(fEqual(NPC[A].Special, 5) && HasSound(SFX_HeroFireRod))
                        PlaySoundSpatial(SFX_HeroFire, NPC[A].Location);
                    else
                        PlaySoundSpatial(SFX_BlockHit, NPC[A].Location);
                }
                NPC[A].Killed = B;
            }
        }
    }
    // Yoshi Fireball
    else if(NPC[A].Type == NPCID_PET_FIRE)
    {
        if(B == 3 || B == 5)
        {
            PlaySoundSpatial(SFX_BlockHit, NPC[A].Location);
            NPC[A].Killed = B;
        }
    }
    // Hammer Bros.
    else if(NPC[A].Type == NPCID_HEAVY_THROWER)
        NPC[A].Killed = B;
    // Hammer Bros. Hammer
    else if(NPC[A].Type == NPCID_HEAVY_THROWN)
    {
        if(B == 3)
            NPC[A].Killed = B;
    }
    // Boom Boom
    else if(NPC[A].Type == NPCID_MINIBOSS)
    {
        if(NPC[A].Special != 4)
        {
            if((B == 1 || B == 10) && NPC[A].Special == 0.0)
            {
                NPC[A].Damage += 3;
                if(B == 1)
                    PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                else
                    PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                NPC[A].Special = 4;
                NPC[A].Special2 = 0;
                NPC[A].Location.SpeedX = 0;
                NPC[A].Location.SpeedY = 0;
            }
            else if(B == 3)
            {
                NPC[A].Immune = 20;
                if(C > 0)
                    NPCHit(C, 3, A);
                NPC[A].Damage++;
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            }
            else if(B == 6)
            {
                NPC[A].Killed = B;
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            }
            else if(B == 10)
            {
#if XTECH_ENABLE_WEIRD_GFX_UPDATES
                UpdateGraphics(true);
#endif
                NPC[A].Immune = 10;
                NPC[A].Damage++;
                PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
            }
        }

        if(NPC[A].Damage >= 9)
            NPC[A].Killed = B;
    }
    // Bullet Bills
    else if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET)
    {
        if(B == 1 || B == 3 || B == 4 || B == 5 || B == 7 || B == 8 || B == 10)
        {
            if(!((B == 3 || B == 4) && (NPC[C].Type == NPCID_PLR_FIREBALL || NPC[C].Type == NPCID_LAVABUBBLE)))
            {
                if(!(B == 7 && NPC[A].Projectile))
                {
                    if(!(B == 3 && NPC[A].CantHurt > 0) && !(B == 3 && NPC[C].Type == NPCID_PET_FIRE))
                    {
                        if(!(NPC[A].Type == NPCID_BIG_BULLET && B == 4))
                        {
                            if(!(B == 10 && NPC[A].Projectile)) // Link can't stab friendly bullets
                            {
                                NPC[A].Killed = B;
                                if(A != C && B != 8 && B != 10)
                                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                            }
                        }
                    }
                }
            }
        }
    }
    // Birdo
    else if(NPC[A].Type == NPCID_SPIT_BOSS)
    {
        if(NPC[A].Special >= 0)
        {
            if(B == 3)
            {
                if(NPC[C].Type != NPCID_PLR_FIREBALL)
                {
                    NPC[A].Special = -30;
                    NPC[A].Damage += 1;
                    NPC[A].Direction = -NPC[A].Direction;
                    PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
                }
            }
            else if(B == 4)
                NPC[A].Damage = 3;
            else if(B == 10)
            {
                NPC[A].Special = -30;
                NPC[A].Damage += 1;
                NPC[A].Direction = -NPC[A].Direction;
                PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
            }

            if(NPC[A].Damage >= 3)
                NPC[A].Killed = B;
        }

        if(B == 6)
        {
            NPC[A].Killed = B;
            PlaySoundSpatial(SFX_SpitBossHit, NPC[A].Location);
        }
    }
    // Eggs
    else if(NPC[A].Type == NPCID_SPIT_BOSS_BALL)
    {
        if(B == 3)
        {
            if(NPC[C].Type == NPCID_PLR_FIREBALL)
                B = 0;
        }

        if(B == 3 || B == 4 || B == 5 || B == 7 || B == 10)
        {
            if(C == A)
                NPC[A].Special = 1;
            else
            {
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.6;
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            }

            NPC[A].Killed = B;
        }
    }
    // Indestructable Objects
    else if(NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_SPRING || NPC[A].Type == NPCID_KEY || NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_TNT || NPC[A].Type == NPCID_GRN_BOOT || NPC[A].Type == NPCID_RED_BOOT || NPC[A].Type == NPCID_BLU_BOOT || NPC[A].Type == NPCID_TOOTHYPIPE || NPCIsYoshi(NPC[A]) || NPC[A].Type == NPCID_ITEM_POD || (NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D) || NPC[A].Type == NPCID_TIMER_S2 || NPC[A].Type == NPCID_EARTHQUAKE_BLOCK || NPC[A].Type == NPCID_FLY_BLOCK || NPC[A].Type == NPCID_FLY_CANNON)
    {
        if(NPC[A].Type == NPCID_EARTHQUAKE_BLOCK && (B == 4 || B == 5 || B == 10))
        {
            NPC[A].Killed = 4;
            PowBlock();
        }

        if(NPC[A].Type == NPCID_ITEM_POD && (B == 4 || B == 5))
        {
            if(NPC[C].Type != NPC[A].Type)
                NPC[A].Killed = B;
        }
        else if(NPC[A].Type == NPCID_ITEM_POD && B == 10)
        {
            PlaySoundSpatial(SFX_HeroKill, NPC[A].Location);
            NPC[A].Special2 = 1;
        }
        else
        {
            if(B == 1 && NPC[A].Type == NPCID_SPRING)
                PlaySoundSpatial(SFX_Spring, NPC[A].Location);

            if(B == 1 && (NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_TNT))
            {
                NPC[A].Killed = 1;
                if(NPC[A].Type == NPCID_COIN_SWITCH)
                {
                    PSwitchTime = Physics.NPCPSwitch;
                    PSwitchPlayer = C;
                }
                else if(NPC[A].Type == NPCID_TIME_SWITCH)
                {
                    PSwitchStop = Physics.NPCPSwitch;
                    FreezeNPCs = true;
                    PSwitchPlayer = C;
                }
            }

            if(B == 2 && NPC[A].Location.SpeedY > -4 && NPC[A].CantHurt == 0)
            {
                NPC[A].CantHurt = 10;
                PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                NPC[A].Location.SpeedY = -5;
                // If .Type = 96 Then .Location.SpeedY = -4
                NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
            }
            else if(B == 6)
            {
                if(NPC[A].Type == NPCID_RED_BOOT)
                {
                    tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 2;
                    // tempLocation.X = .Location.X + .Location.Width / 2 - 4 '+ 4 * .Direction
                    tempLocation.X = NPC[A].Location.X - 4 + dRand() * (NPC[A].Location.Width + 8) - 4;
                    NewEffect(EFFID_SKID_DUST, tempLocation);
                }
                else
                {
                    if(NPC[A].Type == NPCID_SPRING)
                        NPC[A].Location.Y -= 16;
                    NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                    if(!NPC[A].NoLavaSplash)
                        NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
                    PlaySoundSpatial(SFX_Lava, NPC[A].Location);
                    NPC[A].Location = static_cast<Location_t>(NPC[A].DefaultLocation);

                    if(NPC[A].Active)
                    {
                        NPC[A].Active = false;
                        NPCQueues::update(A);
                    }
                    else
                    {
                        NPCQueues::Unchecked.push_back(A);
                    }

                    // TODO: any other hooks for inactive?

                    NPC[A].TimeLeft = 0;
                    NPC[A].Projectile = false;
                    NPC[A].Direction = NPC[A].DefaultDirection;
                    NPC[A].CantHurt = 0;
                    NPC[A].CantHurtPlayer = 0;
                    NPC[A].Reset[1] = false;
                    NPC[A].Reset[2] = false;
                    NPCQueues::NoReset.push_back(A);
                }
            }
        }
    }
    // Misc. Things With No Jump Death (SMB2 Shy Guys, SMB2 Ninji, SMB2 Pokey)
    else if(NPC[A].Type == NPCID_BLU_GUY || NPC[A].Type == NPCID_RED_GUY || NPC[A].Type == NPCID_STACKER || NPC[A].Type == NPCID_JUMPER_S3 || NPC[A].Type == NPCID_RED_FISH_S1 || NPC[A].Type == NPCID_SPIKY_S3 || NPC[A].Type == NPCID_SPIKY_S4 || NPC[A].Type == NPCID_SPIKY_BALL_S4 || NPC[A].Type == NPCID_SPIKY_THROWER || NPC[A].Type == NPCID_ITEM_THROWER || NPC[A].Type == NPCID_SPIKY_BALL_S3 || NPC[A].Type == NPCID_CRAB || NPC[A].Type == NPCID_FLY || (NPC[A].Type >= NPCID_BIRD && NPC[A].Type <= NPCID_GRY_SPIT_GUY) || NPC[A].Type == NPCID_SATURN || NPC[A].Type == NPCID_SQUID_S3 || NPC[A].Type == NPCID_SQUID_S1 || NPC[A].Type == NPCID_WALK_PLANT || NPC[A].Type == NPCID_VINE_BUG)
    {
        if(B == 10 && NPC[A].Type != NPCID_SATURN)
            NPC[A].Killed = B;
        else if(B != 1)
        {
            if(B == 6)
                NPC[A].Killed = B;
            else if(B == 2 && NPC[A].Type == NPCID_SATURN)
            {
                if(NPC[A].CantHurt == 0)
                {
                    NPC[A].CantHurt = 10;
                    PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                    NPC[A].Location.SpeedY = -5;
                    NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
                    NPC[A].Projectile = true;
                    NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.5;
                }
            }
            else if(NPC[A].Type == NPCID_SATURN && B == 5)
            {
                Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                NPC[A].Projectile = true;
                NPC[A].Location.SpeedX = 3 * -Player[NPC[A].HoldingPlayer].Direction;
                NPC[A].Location.SpeedY = -4;
                NPC[A].WallDeath = 0;
                NPC[A].HoldingPlayer = 0;
            }
            else if(NPC[A].Type == NPCID_SATURN && B == 3)
            {
                if(NPC[C].HoldingPlayer == 0 && NPC[C].Type != NPC[A].Type)
                {
                    NPC[A].Immune = 30;
                    NPC[A].Projectile = true;
                    NPC[A].Location.SpeedY = -5;
                    NPC[A].Location.SpeedX = (NPC[C].Location.SpeedX + NPC[A].Location.SpeedX) * 0.5;
                    if(NPC[A].Location.SpeedX < 1.2 && NPC[A].Location.SpeedX > -1.2)
                    {
                        if(NPC[C].Direction == -1)
                            NPC[A].Location.SpeedX = 3;
                        else
                            NPC[A].Location.SpeedX = -3;
                    }
                    PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
                }
            }
            else if(NPC[A].Type == NPCID_SATURN && B == 10)
            {
                NPC[A].Immune = 30;
                NPC[A].Projectile = true;
                NPC[A].Location.SpeedY = -5;
                NPC[A].Location.SpeedX = Player[C].Location.SpeedX + 4 * Player[C].Direction;
                PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
            }
            else if(!(NPC[A].Type == NPCID_SATURN && (B == 4 || B == 8 || (B == 3 && NPC[C].Type == NPCID_PLR_FIREBALL))))
            {
                if(NPC[A].Type == NPCID_SATURN && B == 7)
                {
                    NPC[A].Direction = Player[C].Direction;
                    NPC[A].Location.SpeedX = std::abs(NPC[A].Location.SpeedX) * NPC[A].Direction;
                    NPC[A].TurnAround = false;
                    NPC[A].Location.SpeedY = -6;
                    NPC[A].Projectile = true;
                    PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
                }
                else
                    NPC[A].Killed = B;
            }
        }
        else if(B == 1 && NPC[A].Type == NPCID_RED_FISH_S1)
        {
            NPC[A].Killed = B;
            NPC[A].Location.SpeedY = 0;
            NPC[A].Location.SpeedX = 0;
        }
        else if(B == 1 && !NPC[A]->CanWalkOn && !NPC[A]->JumpHurt)
        {
            NPC[A].Killed = B;
            NPC[A].Location.SpeedY = 0.123;
            NPC[A].Location.SpeedX = 0;
        }

        if(B == 1 && NPC[A].Type == NPCID_SATURN)
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
        if((B == 1 || B == 8) && (NPC[A].Type == NPCID_SPIKY_THROWER || NPC[A].Type == NPCID_ITEM_THROWER))
            NPC[A].Killed = B;
    }
    // Exits
    else if(NPCIsAnExit(NPC[A]))
    {
        if(B == 6)
        {
            NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
            NewEffect(EFFID_LAVA_SPLASH, NPC[A].Location);
            PlaySoundSpatial(SFX_Lava, NPC[A].Location);
            NPC[A].Location = static_cast<Location_t>(NPC[A].DefaultLocation);
            NPCQueues::Unchecked.push_back(A);
        }
    }
    // Coins
    else if(NPC[A]->IsACoin)
    {
        if(LevelEditor)
            PlaySoundSpatial(SFX_ShellHit, NPC[A].Location);
        if(B == 2)
        {
            if(NPC[A].Type == NPCID_GEM_1 || NPC[A].Type == NPCID_GEM_5 || NPC[A].Type == NPCID_GEM_20)
            {
                PlaySoundSpatial(SFX_HeroRupee, NPC[A].Location);
                NewEffect(EFFID_COIN_COLLECT, NPC[A].Location);
                MoreScore(1, NPC[A].Location);
            }
            else if(NPC[A].Type == NPCID_COIN_5 || NPC[A].Type == NPCID_RED_COIN)
            {
                PlaySoundSpatial(SFX_Coin, NPC[A].Location);
                NewEffect(EFFID_COIN_COLLECT, NPC[A].Location);
                MoreScore(1, NPC[A].Location);
            }
            else
            {
                NPC[A].Location.Y += NPC[A].Location.Height;
                // useless self-assignment code [PVS-Studio]
                //NPC[A].Location.X = NPC[A].Location.X; // - (32 - .Location.Width) / 2
                NPC[A].Location.Height = 0;
                PlaySoundSpatial(SFX_Coin, NPC[A].Location);
                NewEffect(EFFID_COIN_BLOCK_S3, NPC[A].Location);
            }

            NPC[A].Killed = 9;

            if(NPC[A].Type == NPCID_GEM_5 || NPC[A].Type == NPCID_COIN_5)
                Coins += 5;
            else if(NPC[A].Type == NPCID_GEM_20)
                Coins += 20;
            else
                Coins += 1;

            if(Coins >= 100)
                Got100Coins();
        }
        else if(B == 5 || B == 3 || B == 4 || B == 6)
            NPC[A].Killed = B;
        else if(B == 10)
        {
            if(C > 0)
                TouchBonus(C, A);
        }
    }
    // Bonus Items
    else if(NPC[A]->IsABonus)
    {
        if(B == 2 && NPC[A].Location.SpeedY > -4)
        {
            PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            NPC[A].Location.SpeedY = -5;
            NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height - 0.01;
        }
        // B == 6 - touched a lava block, C is a block, not NPC!!!
        else if(B == 6 && g_compatibility.fix_powerup_lava_bug)
        {
            NPC[A].Killed = B;
        }
        else if(B == 6 && C > maxNPCs)
        {
            pLogWarning("SMBX64 engine would have crashed on illegal index to NPC %d (maximum legal index %d)", C, maxNPCs);
            NPC[A].Killed = B;
        }
        // B == 6 - old behavior, access index C as an NPC
        else if(B == 6 || B == 5 || B == 4)
        {
            if(!(NPC[C].Type == NPCID_PLR_FIREBALL || NPC[C].Type == NPCID_PET_FIRE || NPC[C].Type == NPCID_PLR_HEAVY || NPCIsVeggie(NPC[C])))
                NPC[A].Killed = B;
        }
        else if(B == 7)
        {
            if(NPC[A].Type == NPCID_POWER_S3 || NPC[A].Type == NPCID_SWAP_POWER || NPC[A].Type == NPCID_FIRE_POWER_S3 ||
               NPC[A].Type == NPCID_LIFE_S3 || NPC[A].Type == NPCID_POISON || NPC[A].Type == NPCID_STATUE_POWER ||
               (NPC[A].Type >= NPCID_FIRE_POWER_S1 && NPC[A].Type <= NPCID_3_LIFE) || NPC[A].Type == NPCID_HEAVY_POWER)
            {
                NPC[A].Direction = Player[C].Direction;
                NPC[A].Location.SpeedX = std::abs(NPC[A].Location.SpeedX) * double(NPC[A].Direction);
                NPC[A].TurnAround = false;
                NPC[A].Location.SpeedY = -6;
                PlaySoundSpatial(SFX_Stomp, NPC[A].Location);
            }
        }
        else if(B == 10 && NPC[A].Type != NPCID_POISON)
        {
            if(C > 0 && NPC[A].Effect == 0)
                TouchBonus(C, A);
        }
    }

    if(NPC[A].Killed == 10)
        NPC[A].Direction = Player[C].Direction;

    bool tempBool = false;
    if(NPC[A].Killed == 3)
    {
        if(NPC[C].Type == NPCID_PLR_FIREBALL && NPC[C].Special == 5.0 && NPC[A].Type != NPCID_PLR_FIREBALL)
        {
            NPC[A].Direction = NPC[C].Direction;
            NPC[A].Killed = 10;
            tempBool = true;
        }
    }

    if(NPC[A].Killed == 10)
    {
        if(tempBool)
            MoreScore(NPC[A]->Score, NPC[A].Location, NPC[C].Multiplier);
        else
            MoreScore(NPC[A]->Score, NPC[A].Location, Player[C].Multiplier);
    }
    // Calculate Score
    Player[0].Multiplier = 0;
    if((B == 1 || B == 8) && C <= numPlayers && !NPC[A]->IsABonus)
    {
        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_TNT)
        {
        }
        else if(oldNPC.Type == 137 || NPC[A].Type == NPCID_SLIDE_BLOCK || oldNPC.Type == 166 || NPC[A].Type == NPCID_BULLY || (oldNPC->IsAShell && B != 8))
        {
            if(B != 8) // (.Type = 45 And B = 8) Then
            {
                if(NPC[A].Type != NPCID_FLIPPED_RAINBOW_SHELL)
                {
                    // MoreScore 1, .Location
                    if(Player[C].Multiplier > NPC[A].Multiplier)
                        NPC[A].Multiplier = Player[C].Multiplier;
                }
            }
        }
        else if(NPC[A].Location.SpeedX != oldNPC.Location.SpeedX || NPC[A].Location.SpeedY != oldNPC.Location.SpeedY ||
                /*NPC[A].Projectile != NPC[A].Projectile ||*/ // FIXME: Wrong condition, always false [PVS Studio]
                NPC[A].Killed != oldNPC.Killed ||
                NPC[A].Type != oldNPC.Type || NPC[A].Inert != oldNPC.Inert)
        {
            if(NPC[A].Type == NPCID_MINIBOSS && NPC[A].Killed == 0)
            {
                MoreScore(2, NPC[A].Location, Player[C].Multiplier);
                if(Player[C].Multiplier > NPC[A].Multiplier)
                    NPC[A].Multiplier = Player[C].Multiplier;
            }
            else
            {
                MoreScore(NPC[A]->Score, NPC[A].Location, Player[C].Multiplier);
                if(Player[C].Multiplier > NPC[A].Multiplier)
                    NPC[A].Multiplier = Player[C].Multiplier;
            }
        }
    }

    if((B == 2 || B == 7) && !NPC[A]->IsABonus && oldNPC.Type != 194 && NPC[A].Type != NPCID_EARTHQUAKE_BLOCK)
    {
        if(NPC[A].Killed != 0 || NPC[A].Type != oldNPC.Type)
        {
            MoreScore(NPC[A]->Score, NPC[A].Location);
            if(B == 2)
                NewEffect(EFFID_WHACK, newLoc(NPC[A].Location.X, NPC[A].Location.Y + NPC[A].Location.Height - 16));
        }
        else if(NPC[A].Location.SpeedX != oldNPC.Location.SpeedX ||
                NPC[A].Location.SpeedY != oldNPC.Location.SpeedY /*||
                NPC[A].Projectile != NPC[A].Projectile*/) // FIXME: Wrong condition, always false [PVS Studio]
        {
            // MoreScore 1, .Location
            if(B == 2)
                NewEffect(EFFID_WHACK, newLoc(NPC[A].Location.X, NPC[A].Location.Y + NPC[A].Location.Height - 16));
        }
    }

    if(B == 4 && NPC[A].Killed == 4 && !NPC[A]->IsACoin && C != A && NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_PLR_ICEBALL && NPC[A].Type != NPCID_PET_FIRE && NPC[A].Type != NPCID_EARTHQUAKE_BLOCK)
    {
        if(NPC[C].Type != NPCID_BOSS_CASE)
        {
            if(!(NPC[A].Type == NPCID_BULLET && NPC[A].CantHurt > 0))
            {
                if(NPC[C].Multiplier < NPC[A].Multiplier)
                    NPC[C].Multiplier = NPC[A].Multiplier;
                MoreScore(NPC[A]->Score, NPC[A].Location, NPC[C].Multiplier);
            }
        }
    }

    if(B == 5 && NPC[A].Killed == 5)
    {
        if(NPC[A].Multiplier < NPC[C].Multiplier)
            NPC[A].Multiplier = NPC[C].Multiplier;
        MoreScore(NPC[A]->Score, NPC[A].Location, NPC[A].Multiplier);
    }

    if(B == 6 && NPC[A].Killed == 6 && (NPC[A].Type == NPCID_BOSS_FRAGILE || NPC[A].Type == NPCID_VILLAIN_S1 || NPC[A].Type == NPCID_SICK_BOSS || NPC[A].Type == NPCID_MINIBOSS || NPC[A].Type == NPCID_SPIT_BOSS || NPC[A].Type == NPCID_VILLAIN_S3))
    {
        if(!NPC[A]->WontHurt && !NPC[A]->IsABonus && NPC[A].Type != NPCID_PLR_FIREBALL)
            MoreScore(NPC[A]->Score, NPC[A].Location);
    }

    if(!NPC[A]->IsACoin && B == 3 && C != A &&
       (NPC[A].Killed == B || NPC[A].Damage != oldNPC.Damage) &&
        NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_PET_FIRE && NPC[A].Type != NPCID_SLIDE_BLOCK &&
        NPC[A].Type != NPCID_HOMING_BALL && NPC[A].Type != NPCID_EARTHQUAKE_BLOCK)
    {
        if(NPC[A].Killed == B)
        {
            if(NPC[C].Multiplier < NPC[A].Multiplier)
                NPC[C].Multiplier = NPC[A].Multiplier;
            MoreScore(NPC[A]->Score, NPC[A].Location, NPC[C].Multiplier);
        }
        if(NPC[A].Type != NPCID_BOSS_CASE && NPC[A].Type != NPCID_BOSS_FRAGILE)
        {
            if(NPC[A].Location.Width >= 64 || NPC[A].Location.Height >= 64)
            {
                tempLocation = NPC[C].Location;
                tempLocation.X = NPC[C].Location.X + NPC[C].Location.Width / 2.0 - 16 + NPC[C].Location.SpeedX;
                tempLocation.Y = NPC[C].Location.Y + NPC[C].Location.Height / 2.0 - 16 + NPC[C].Location.SpeedY;
            }
            else
            {
                tempLocation = NPC[A].Location;
                tempLocation.Y = (NPC[C].Location.Y + tempLocation.Y + NPC[C].Location.Height / 2.0 + tempLocation.Height / 2.0) / 2 - 16;
                tempLocation.X = (NPC[C].Location.X + tempLocation.X + NPC[C].Location.Width / 2.0 + tempLocation.Width / 2.0) / 2 - 16;
            }
            NewEffect(EFFID_WHACK, tempLocation);
        }
    }

    if(NPC[A].Killed == 6)
    {
        if(BlockKills2[Block[C].Type])
            NPC[A].NoLavaSplash = true;
    }

    if(NPC[A].Killed == 0 && NPC[A].Location.SpeedX == 0.0 && oldNPC.Location.SpeedX != 0.0)
        NPC[A].RealSpeedX = 0;

    if(NPC[A].Type != oldNPC.Type)
    {
        NPC[A].Location.Y += NPC[A].Location.Height;
        NPC[A].Location.Height = NPC[A]->THeight;
        NPC[A].Location.Y -= NPC[A].Location.Height;
        NPC[A].Location.X += (NPC[A].Location.Width / 2.0);
        NPC[A].Location.Width = NPC[A]->TWidth;
        NPC[A].Location.X -= (NPC[A].Location.Width / 2.0);
    }

    if(NPC[A].Location.Width != oldNPC.Location.Width)
    {
        treeNPCUpdate(A);
        if(NPC[A].tempBlock > 0)
            treeNPCSplitTempBlock(A);

        NPCQueues::Unchecked.push_back(A);
    }
    else if(NPC[A].Location.Height != oldNPC.Location.Height
        || NPC[A].Location.X != oldNPC.Location.X
        || NPC[A].Location.Y != oldNPC.Location.Y)
    {
        treeNPCUpdate(A);
        if(NPC[A].tempBlock > 0)
            treeNPCSplitTempBlock(A);
    }

    StopHit = 0;
}
