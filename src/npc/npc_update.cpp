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

#include <algorithm>
#include <array>

#include <Logger/logger.h>

#include "../globals.h"
#include "../npc.h"
#include "../sound.h"
#include "../graphics.h"
#include "../collision.h"
#include "../effect.h"
#include "../layers.h"
#include "../editor.h"
#include "../blocks.h"
#include "../sorting.h"
#include "../compat.h"
#include "../config.h"
#include "../main/trees.h"
#include "../npc_id.h"
#include "../eff_id.h"
#include "../layers.h"

#include "game_main.h"
#include "npc_traits.h"

#include "npc/npc_queues.h"
#include "npc/section_overlap.h"

#include <Utils/maths.h>

// moved into the function, as a static array
// static RangeArr<int, 0, maxNPCs> newAct;
// Why this array is here? to don't reallocate it every call of UpdateNPCs()

void CheckNPCWidth(NPC_t& n)
{
    if(fEqual(n.Location.Width, 32.0))
    {
        if(n.Type != NPCID_CONVEYOR && n.Type != NPCID_STATUE_S3)
        {
            // If .Type = 58 Or .Type = 21 Then
            if(!(NPCIsAnExit(n) || n.Type == NPCID_PLANT_S3 || n.Type == NPCID_BOTTOM_PLANT ||
                 n.Type == NPCID_SIDE_PLANT || n.Type == NPCID_BIG_PLANT || n.Type == NPCID_LONG_PLANT_UP ||
                 n.Type == NPCID_LONG_PLANT_DOWN || n.Type == NPCID_PLANT_S1 || n.Type == NPCID_FIRE_PLANT))
            {
                n.Location.X += 0.015;
            }

            n.Location.Width -= 0.03;
        }
    }
    else if(fEqual(n.Location.Width, 256.0))
        n.Location.Width = 255.9;
    else if(fEqual(n.Location.Width, 128.0))
        n.Location.Width = 127.9;

    // unset a bunch of variables that would normally decay for inactive NPCs
    if(!n.Active)
    {
        n.tempBlock = 0;

        n.TailCD = 0;
        n.Immune = 0;

        // Failing to update Location.SpeedX causes inaccurate initial movement speed of some NPCs, including squids
        //   (cross-ref UpdateNPCs condition RealSpeedX != 0 for Active NPCs.)
        // Technically, the final values should depend on the value of speedVar,
        //   which invalidly exchanged information between loop iterations,
        //   but in the valid case this is the outcome.
        //   (cross-ref UpdateNPCs clause updating RealSpeedX for Active, formerly all, NPCs)
        if(n.RealSpeedX != 0)
        {
            n.Location.SpeedX = n.RealSpeedX;
            n.RealSpeedX = 0;
        }

        if(!n.Projectile || n.Type == NPCID_TOOTHY || n.Type == NPCID_TANK_TREADS)
            n.Multiplier = 0;
    }
}

void UpdateNPCs()
{
    // this is 1 of the 2 clusterfuck subs in the code, be weary

    // misc variables used mainly for arrays
    // int A = 0;
    // int B = 0;
//    float C = 0;
//    float D = 0;
//    double E = 0;
//    float F = 0;

//    std::string tempStr;
    // int oldSlope = 0; // previous sloped block the npc was on
//    NPC_t tempNPC;
    // int HitSpot = 0; // used for collision detection
    // double tempHit = 0;
    // double tempHitOld = 0;
    // Block_t tmpBlock;
    // int tempHitBlock = 0;
    // int tempHitOldBlock = 0;
    // int tempHitIsSlope = 0;
    // float tempSpeedA = 0;
//    float tempSpeedB = 0;
    // bool tempTurn = false; // used for turning the npc around
    // Location_t tempLocation;
    // Location_t tempLocation2;
    // Location_t preBeltLoc;
    // float beltCount = 0;
    // int tempBlockHit[3] = {0}; // Hit block from below code
    // int winningBlock = 0; // More code for hitting the block from below
    int numTempBlock = 0;
    // float speedVar = 0; // percent of the NPC it should actually moved. this helps when underwater

    // bool tempBool = false;
//    bool tempBool2 = false;
//    bool tempBool3 = false;
    // float newY = 0; // fully unused
//    bool straightLine = false;
    const Block_t blankBlock;
//    bool noBelt = false;
    // float oldBeltSpeed = 0;
//    float beltFixX = 0;
    // int oldDirection = 0;

    // used for collision detection
    // int64_t fBlock = 0;
    // int64_t lBlock = 0;
    // int64_t fBlock2 = 0;
    // int64_t lBlock2 = 0;
    // int bCheck2 = 0;
    // int bCheck = 0;
    // float addBelt = 0;
    // int numAct = 0;
    // bool beltClear = false; // stops belt movement when on a wall
    // bool resetBeltSpeed = false;
    // double PlrMid = 0;
    // double Slope = 0;
    // bool SlopeTurn = false;
//    std::string timeStr;

    double lyrX = 0; // for attaching to layers
    double lyrY = 0; // for attaching to layers

    // newAct.fill(0);

    NPC[0].Location.SpeedX = 0;
    NPC[0].Location.SpeedY = 0;

    if(LevelMacro > LEVELMACRO_OFF)
    {
        if(PSwitchTime > 0)
            PSwitchTime = 100;
        if(PSwitchStop > 0)
            PSwitchStop = 100;
    }

    // used for the p switches
    if(PSwitchStop > 0) // time stop
    {
        if(PSwitchStop == Physics.NPCPSwitch)
        {
            StopMusic();
            StartMusic(-1);
            PlaySound(SFX_PSwitch);
        }
        if(PSwitchTime > 2)
            PSwitchTime = 2;
        PSwitchStop -= 1;
        if(PSwitchStop <= 0)
        {
            FreezeNPCs = false;
            StopMusic();
            StartMusic(Player[PSwitchPlayer].Section);
        }
    }

    if(FreezeNPCs) // When time is paused
    {
        StopHit = 0;
        for(int A = numNPCs; A >= 1; A--) // check to see if NPCs should be killed
        {
            if(NPCIsBoot(NPC[A]) || NPCIsYoshi(NPC[A]))
            {
                if(NPC[A].CantHurt > 0)
                {
                    NPC[A].CantHurt -= 1;
                    if(NPC[A].CantHurt == 0)
                        NPC[A].CantHurtPlayer = 0;
                }
            }

            if(NPC[A].TimeLeft > 0)
                NPC[A].TimeLeft -= 1;

            if(NPC[A].Immune > 0)
                NPC[A].Immune -= 1;

            NPC[A].Chat = false;

            if(NPC[A].TimeLeft == 0)
            {
                Deactivate(A);
                if(g_compatibility.fix_FreezeNPCs_no_reset)
                    NPC[A].TimeLeft = -1;
            }

            if(NPC[A].JustActivated)
            {
                NPC[A].JustActivated = 0;
                NPCQueues::update(A);
            }

            if(NPC[A].Killed > 0)
            {
                if(NPC[A].Location.SpeedX == 0.0)
                {
                    NPC[A].Location.SpeedX = dRand() * 2 - 1;
                    if(NPC[A].Location.SpeedX < 0)
                        NPC[A].Location.SpeedX -= 0.5;
                    else
                        NPC[A].Location.SpeedX += 0.5;
                }
                KillNPC(A, NPC[A].Killed);
            }
        }

        NPCQueues::Killed.clear();

        CharStuff();
        return;
    }


    if(CoinMode) // this is a cheat code
    {
        if(!g_compatibility.modern_lives_system && Lives >= 99 && Coins >= 99)
            CoinMode = false;
        else
        {
            PlaySound(SFX_Coin);
            Coins += 1;
            if(Coins >= 100)
            {
                if(g_compatibility.modern_lives_system)
                    CoinMode = false;

                Got100Coins();
            }
        }
    }



    // need this complex loop syntax because RespawnDelay can be modified within it
    for(auto it = NPCQueues::RespawnDelay.begin(); it != NPCQueues::RespawnDelay.end();)
    {
        int A = *(it++);

        if(NPC[A].RespawnDelay > 0)
        {
            NPC[A].Reset[1] = false;
            NPC[A].Reset[2] = false;
            NPC[A].RespawnDelay -= 1;

            NPCQueues::NoReset.push_back(A);
        }

        if(NPC[A].RespawnDelay == 0)
            NPCQueues::RespawnDelay.erase(A);
    }

    for(int A = maxNPCs - 100 + 1; A <= numNPCs; A++)
    {
        // if(A > maxNPCs - 100)
        NPC[A].Killed = 9;
        NPCQueues::Killed.push_back(A);
    }

    for(int A : NPCQueues::Unchecked)
        CheckNPCWidth(NPC[A]);

    int numNPCsMax = numNPCs;

    // need this complex loop syntax because Active can be modified within it
    for(int A : NPCQueues::Active.may_erase)
    {
        if(A > numNPCsMax)
            break;

        if(NPC[A].Hidden)
        {
            Deactivate(A);
            CheckNPCWidth(NPC[A]);
        }
        else if(NPC[A].TailCD > 0)
            NPC[A].TailCD -= 1;

        // generator code
        if(NPC[A].Generator)
        {
            NPC[A].Active = false;

            if(!NPC[A].Hidden)
            {
                NPC[A].TimeLeft = 0;

                // Old timer logic
                // this did not achieve anything other than keeping the value from growing large, but was likely the reason Redigit thought floats were necessary
#if 0
                NPC[A].GeneratorTime += 1;

                if(NPC[A].GeneratorTime >= NPC[A].GeneratorTimeMax * 6.5f)
                    NPC[A].GeneratorTime = NPC[A].GeneratorTimeMax * 6.5f;
#endif

                if(NPC[A].GeneratorTime * 10 < NPC[A].GeneratorTimeMax * 65)
                    NPC[A].GeneratorTime += 1;

                if(NPC[A].GeneratorActive)
                {
                    NPC[A].GeneratorActive = false;
                    if(NPC[A].GeneratorTime * 10 >= NPC[A].GeneratorTimeMax * 65)
                    {
                        bool tempBool = false;

                        if(numNPCs == maxNPCs - 100)
                            tempBool = true;

                        if(NPC[A].Type != NPCID_ITEM_BURIED && !tempBool)
                        {
                            for(int B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].TimeToLive == 0)
                                {
                                    if(CheckCollision(NPC[A].Location, Player[B].Location))
                                    {
                                        tempBool = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if(NPC[A].Type != NPCID_ITEM_BURIED && !tempBool)
                        {
                            for(int B : treeBlockQuery(NPC[A].Location, SORTMODE_NONE))
                            {
                                if(!Block[B].Hidden && !BlockIsSizable[Block[B].Type])
                                {
                                    if(CheckCollision(NPC[A].Location,
                                                      newLoc(Block[B].Location.X + 0.1, Block[B].Location.Y + 0.1,
                                                             Block[B].Location.Width - 0.2, Block[B].Location.Height - 0.2)))
                                    {
                                        tempBool = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if(!tempBool)
                        {
                            for(int B : treeNPCQuery(NPC[A].Location, SORTMODE_NONE))
                            {
                                if(B != A && NPC[B].Active && NPC[B].Type != NPCID_CONVEYOR)
                                {
                                    if(CheckCollision(NPC[A].Location, NPC[B].Location))
                                    {
                                        tempBool = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if(tempBool)
                            NPC[A].GeneratorTime = NPC[A].GeneratorTimeMax;
                        else
                        {
                            NPC[A].GeneratorTime = 0;
                            numNPCs++;
                            NPC[numNPCs] = NPC[A];

                            if(NPC[A].GeneratorEffect == 1) // Warp NPC
                            {
                                NPC[numNPCs].Layer = NPC[A].Layer;
                                NPC[numNPCs].Effect3 = NPC[A].GeneratorDirection;
                                NPC[numNPCs].Effect = NPCEFF_WARP;
                                NPC[numNPCs].Location.SpeedX = 0;
                                NPC[numNPCs].TimeLeft = 100;
                                if(NPC[A].GeneratorDirection == 1)
                                {
                                    if(NPC[A]->HeightGFX > NPC[A].Location.Height)
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A]->HeightGFX;
                                        NPC[numNPCs].Effect2 = NPC[numNPCs].Location.Y - (NPC[A]->HeightGFX - NPC[A].Location.Height);
                                    }
                                    else
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                                        NPC[numNPCs].Effect2 = NPC[numNPCs].Location.Y;
                                    }
                                }
                                else if(NPC[A].GeneratorDirection == 3)
                                {
                                    if(NPC[A]->HeightGFX > NPC[A].Location.Height)
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                                        NPC[numNPCs].Effect2 = NPC[numNPCs].Location.Y + NPC[A].Location.Height + (NPC[A]->HeightGFX - NPC[A].Location.Height);
                                    }
                                    else
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                                        NPC[numNPCs].Effect2 = NPC[numNPCs].Location.Y + NPC[A].Location.Height;
                                    }
                                }
                                else if(NPC[A].GeneratorDirection == 2)
                                {
                                    NPC[numNPCs].Location.Y -= 4;
                                    NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width;
                                    NPC[numNPCs].Effect2 = NPC[numNPCs].Location.X;
                                }
                                else if(NPC[A].GeneratorDirection == 4)
                                {
                                    NPC[numNPCs].Location.Y -= 4;
                                    NPC[numNPCs].Location.X = NPC[A].Location.X - NPC[A].Location.Width;
                                    NPC[numNPCs].Effect2 = NPC[numNPCs].Location.X + NPC[A].Location.Width;
                                }
                            }
                            else if(NPC[A].GeneratorEffect == 2) // projectile
                            {
                                NPC[numNPCs].Layer = LAYER_SPAWNED_NPCS;
                                PlaySoundSpatial(SFX_Bullet, NPC[A].Location);
                                NPC[numNPCs].Projectile = true;
                                if(NPC[numNPCs].Type == NPCID_BULLET) // Normal Bullet Bills
                                    NPC[numNPCs].Projectile = false;

                                if(NPC[numNPCs].Type == NPCID_SLIDE_BLOCK)
                                    NPC[numNPCs].Special = 1;
                                if(NPC[A].GeneratorDirection == 1)
                                {
                                    NPC[numNPCs].Location.SpeedY = -10;
                                    NPC[numNPCs].Location.SpeedX = 0;
                                    NewEffect(EFFID_SMOKE_S3, newLoc(NPC[A].Location.X, NPC[A].Location.Y + 16, 32, 32));
                                    if(NPCIsVeggie(NPC[numNPCs]))
                                        NPC[numNPCs].Location.SpeedX = dRand() * 2 - 1;
                                    // NPC(numNPCs).Location.SpeedY = -1
                                }
                                else if(NPC[A].GeneratorDirection == 2)
                                {
                                    NPC[numNPCs].Location.SpeedX = -Physics.NPCShellSpeed;
                                    NewEffect(EFFID_SMOKE_S3, newLoc(NPC[A].Location.X + 16, NPC[A].Location.Y, 32, 32));
                                }
                                else if(NPC[A].GeneratorDirection == 3)
                                {
                                    NPC[numNPCs].Location.SpeedY = 8;
                                    NPC[numNPCs].Location.SpeedX = 0;
                                    NewEffect(EFFID_SMOKE_S3, newLoc(NPC[A].Location.X, NPC[A].Location.Y - 16, 32, 32));
                                }
                                else
                                {
                                    NPC[numNPCs].Location.SpeedX = Physics.NPCShellSpeed;
                                    SoundPause[3] = 1;
                                    NewEffect(EFFID_SMOKE_S3, newLoc(NPC[A].Location.X - 16, NPC[A].Location.Y, 32, 32));
                                }
                            }

                            NPC[numNPCs].Direction = NPC[numNPCs].DefaultDirection;
                            NPC[numNPCs].Frame = EditorNPCFrame(NPC[numNPCs].Type, NPC[numNPCs].Direction);
                            NPC[numNPCs].DefaultDirection = NPC[numNPCs].Direction;
                            NPC[numNPCs].DefaultType = NPCID_NULL;
                            NPC[numNPCs].Generator = false;
                            NPC[numNPCs].Active = true;
                            NPC[numNPCs].TimeLeft = 100;
                            NPC[numNPCs].TriggerActivate = NPC[A].TriggerActivate;
                            NPC[numNPCs].TriggerDeath = NPC[A].TriggerDeath;
                            NPC[numNPCs].TriggerLast = NPC[A].TriggerLast;
                            NPC[numNPCs].TriggerTalk = NPC[A].TriggerTalk;
                            CheckSectionNPC(numNPCs);
                            if(NPC[numNPCs].TriggerActivate != EVENT_NONE)
                                ProcEvent(NPC[numNPCs].TriggerActivate, 0);
                            if(NPC[numNPCs].Type == NPCID_RANDOM_POWER)
                                NPC[numNPCs].Type = RandomBonus();
                            syncLayers_NPC(numNPCs);
                            CheckNPCWidth(NPC[numNPCs]);
                        }
                    }
                }
            }

            continue;
        }

        // Force-active NPCs
        if(NPC[A].Type == NPCID_CONVEYOR && !NPC[A].Hidden)
        {
            CheckSectionNPC(A);
            bool sameSection = false;
            for(int B = 1; B <= numPlayers; B++)
            {
                if(Player[B].Section == NPC[A].Section)
                    sameSection = true;
            }

            if(sameSection)
            {
                if(!NPC[A].Active)
                    NPCQueues::Active.insert(A);

                NPC[A].TimeLeft = 100;
                NPC[A].Active = true;
                NPC[A].JustActivated = 0;
            }
            else if(!NPC[A].Active)
                NPCQueues::Active.erase(A);
        }
        else if(NPC[A].Type == NPCID_YEL_PLATFORM || NPC[A].Type == NPCID_BLU_PLATFORM || NPC[A].Type == NPCID_GRN_PLATFORM || NPC[A].Type == NPCID_RED_PLATFORM)
        {
            NPC[A].Active = true;
            NPC[A].TimeLeft = 100;
        }

        if(NPC[A].TimeLeft == 1 || NPC[A].JustActivated != 0)
        {
            if(NPC[A].Type == NPCID_STATUE_POWER || NPC[A].Type == NPCID_HEAVY_POWER)
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction, A);
        }

        // process chain activations
        if(NPC[A].JustActivated != 0)
        {
            static std::array<int, maxNPCs> newAct;
            int numAct = 0;

            if(NPC[A].Active && NPC[A].TimeLeft > 1 &&
               NPC[A].Type != NPCID_CONVEYOR && NPC[A].Type != NPCID_FALL_BLOCK_RED &&
               NPC[A].Type != NPCID_FALL_BLOCK_BROWN && !NPC[A]->IsACoin) // And .Type <> NPCID_SPIKY_THROWER
            {
                // if activated by a shared screen, don't make the event player-specific
                const vScreen_t& activ_vscreen = vScreen[NPC[A].JustActivated];
                const Screen_t& activ_screen = Screens[activ_vscreen.screen_ref];
                bool shared_screen = (activ_screen.player_count > 1) && (activ_screen.active_end() - activ_screen.active_begin() == 1);

                int activ_player = (shared_screen) ? 0 : activ_vscreen.player;

                if(NPC[A].TriggerActivate != EVENT_NONE)
                    ProcEvent(NPC[A].TriggerActivate, activ_player);

                Location_t tempLocation = NPC[A].Location;
                tempLocation.Y -= 32;
                tempLocation.X -= 32;
                tempLocation.Width += 64;
                tempLocation.Height += 64;

                for(int B : treeNPCQuery(tempLocation, SORTMODE_ID))
                {
                    if(!NPC[B].Active && B != A && NPC[B].Reset[1] && NPC[B].Reset[2])
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location))
                        {
                            SDL_assert_release(numAct < maxNPCs);
                            newAct[numAct] = B;
                            numAct++;

                            NPC[B].Active = true;
                            NPC[B].TimeLeft = NPC[A].TimeLeft;
                            NPC[B].Section = NPC[A].Section;

                            if(g_compatibility.modern_npc_camera_logic)
                                NPC[B].JustActivated = NPC[A].JustActivated;
                            else
                                NPC[B].JustActivated = 1;

                            if(B < A)
                            {
                                if(NPC[B].TriggerActivate != EVENT_NONE)
                                    ProcEvent(NPC[B].TriggerActivate, activ_player);
                            }

                            NPCQueues::Active.insert(B);
                        }
                    }
                    else if(B != A && NPC[B].Active && NPC[B].TimeLeft < NPC[A].TimeLeft - 1)
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location))
                            NPC[B].TimeLeft = NPC[A].TimeLeft - 1;
                    }
                }

                // int C = 0;

                // while(numAct > C)
                // {
                //     C++;
                for(int C = 0; C < numAct; C++)
                {
                    if(NPC[newAct[C]].Type != NPCID_CONVEYOR && NPC[newAct[C]].Type != NPCID_FALL_BLOCK_RED &&
                       NPC[newAct[C]].Type != NPCID_FALL_BLOCK_BROWN && NPC[newAct[C]].Type != NPCID_SPIKY_THROWER &&
                       !NPC[newAct[C]]->IsACoin)
                    {
                        Location_t tempLocation2 = NPC[newAct[C]].Location;
                        tempLocation2.Y -= 32;
                        tempLocation2.X -= 32;
                        tempLocation2.Width += 64;
                        tempLocation2.Height += 64;

                        for(int B : treeNPCQuery(tempLocation2, SORTMODE_ID))
                        {
                            if(!NPC[B].Active &&
                              (!NPC[B].Hidden || !g_compatibility.fix_npc_activation_event_loop_bug) &&
                               B != A && NPC[B].Reset[1] && NPC[B].Reset[2])
                            {
                                if(CheckCollision(tempLocation2, NPC[B].Location))
                                {
                                    SDL_assert_release(numAct < maxNPCs);
                                    newAct[numAct] = B;
                                    numAct++;

                                    NPC[B].Active = true;
                                    NPC[B].TimeLeft = NPC[newAct[C]].TimeLeft;
                                    NPC[B].Section = NPC[newAct[C]].Section;

                                    if(g_compatibility.modern_npc_camera_logic)
                                        NPC[B].JustActivated = NPC[A].JustActivated;
                                    else
                                        NPC[B].JustActivated = 1;

                                    if(B < A)
                                    {
                                        if(NPC[B].TriggerActivate != EVENT_NONE)
                                            ProcEvent(NPC[B].TriggerActivate, activ_player);
                                    }

                                    NPCQueues::Active.insert(B);
                                }
                            }
                        }
                    }
                }
            } // NPC[A].Active ...

            if(NPC[A].Type == NPCID_BOSS_CASE)
            {
                for(int B : NPCQueues::Active.no_change)
                {
                    if(NPC[B].Type != NPCID_BOSS_CASE && NPC[B].Effect == 0 && NPC[B].Active)
                    {
                        if(!NPC[B]->NoClipping)
                        {
                            // TRIES to check if A contains B, but actually checks the Y axis only
                            // cross-reference the NPC Effect 208 code, which fixes this but wastes a frame
                            if(NPC[A].Location.Y < NPC[B].Location.Y)
                            {
                                if(NPC[A].Location.Y + NPC[A].Location.Height > NPC[B].Location.Y + NPC[B].Location.Height)
                                {
                                    if(NPC[A].Location.Y < NPC[B].Location.Y)
                                    {
                                        if(NPC[A].Location.Y + NPC[A].Location.Height > NPC[B].Location.Y + NPC[B].Location.Height)
                                        {
                                            NPC[B].Frame = EditorNPCFrame(NPC[B].Type, NPC[B].Direction);
                                            NPC[B].Effect = NPCEFF_ENCASED;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } // NPCID_BOSS_CASE
        } // .JustActivated != 0

#if 0
        // this code became the function CheckNPCWidth above
        if(fEqual(NPC[A].Location.Width, 32.0))
        {
            if(NPC[A].Type != NPCID_CONVEYOR && NPC[A].Type != NPCID_STATUE_S3)
            {
                // If .Type = 58 Or .Type = 21 Then
                if(!(NPCIsAnExit(NPC[A]) || NPC[A].Type == NPCID_PLANT_S3 || NPC[A].Type == NPCID_BOTTOM_PLANT ||
                     NPC[A].Type == NPCID_SIDE_PLANT || NPC[A].Type == NPCID_BIG_PLANT || NPC[A].Type == NPCID_LONG_PLANT_UP ||
                     NPC[A].Type == NPCID_LONG_PLANT_DOWN || NPC[A].Type == NPCID_PLANT_S1 || NPC[A].Type == NPCID_FIRE_PLANT))
                {
                    NPC[A].Location.X += 0.015;
                }

                NPC[A].Location.Width -= 0.03;
            }
        }
        else if(fEqual(NPC[A].Location.Width, 256.0))
        {
            NPC[A].Location.Width = 255.9;
        }
        else if(fEqual(NPC[A].Location.Width, 128.0))
        {
            NPC[A].Location.Width = 127.9;
        }
#endif

        // construct tempBlock tree
        NPC[A].tempBlock = 0;
        NPC[A].tempBlockInTree = false;

        if(NPC[A].Active && NPC[A].TimeLeft > 1)
        {
            if(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1)
            {
                if(NPC[A].Projectile)
                    NPC[A].Special2 = 0;
                else
                {
                    NPC[A].Special2 += 1;
                    if(NPC[A].Special2 >= 450)
                    {
                        NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                        NPC[A].Killed = 9;
                        NPCQueues::Killed.push_back(A);
                    }
                }
            }

            if(NPC[A]->IsABlock || NPC[A]->IsAHit1Block || (NPC[A]->CanWalkOn && !(NPC[A]->IsFish && NPC[A].Special == 2)))
            {
                if(
                    (
                        !NPC[A].Projectile && NPC[A].HoldingPlayer == 0 &&
                        NPC[A].Effect == 0 && !(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1) &&
                       !((NPC[A].Type == NPCID_FALL_BLOCK_RED || NPC[A].Type == NPCID_FALL_BLOCK_BROWN) && NPC[A].Special == 1)
                    ) || NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG ||
                    NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG
                )
                {
                    numBlock++;
                    Block[numBlock] = blankBlock;
                    Block[numBlock].Type = 0;
                    Block[numBlock].Location = NPC[A].Location;
                    Block[numBlock].Location.Y = static_cast<int>(floor(static_cast<double>(Block[numBlock].Location.Y + 0.02)));
                    Block[numBlock].tempBlockVehiclePlr = NPC[A].vehiclePlr;
                    Block[numBlock].tempBlockVehicleYOffset = NPC[A].vehicleYOffset;
                    Block[numBlock].tempBlockNpcIdx = A;
                    if(NPC[A].Type == NPCID_VEHICLE)
                        Block[numBlock].Type = 25;
                    if(NPC[A]->IsAHit1Block || (NPC[A]->CanWalkOn && !NPC[A]->IsABlock))
                        Block[numBlock].Type = 26;

                    // moved to Block_t::tempBlockNoProjClipping(), defined in npc_traits.h
                    // if(NPC[A]->CanWalkOn && !NPC[A]->IsAHit1Block && !NPC[A]->IsABlock)
                    //     Block[numBlock].noProjClipping = true;

                    if(NPC[A].Type == NPCID_SPRING && Block[numBlock].Location.Height != 32)
                    {
                        Block[numBlock].Location.Y -= 16;
                        Block[numBlock].Location.Height += 16;
                    }
                    Block[numBlock].Location.SpeedX += NPC[A].BeltSpeed;
                    Block[numBlock].tempBlockNpcType = NPC[A].Type;
                    // not syncing the block layer here because we'll sync all of them together later
                    numTempBlock++;
                    NPC[A].tempBlock = numBlock;
                }
            }
        }
    }

    // only need to add NPC temp blocks to the quadtree when they are at a diff loc than the NPC
    treeTempBlockEnable();

    for(int A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 2)
        {
            numBlock++;
            Block[numBlock] = blankBlock;
            Block[numBlock].Type = 25;
            Block[numBlock].Location = Player[A].Location;
            Block[numBlock].Location.X = static_cast<int>(floor(static_cast<double>(Block[numBlock].Location.X))) + 1;
            Block[numBlock].Location.Y = static_cast<int>(floor(static_cast<double>(Block[numBlock].Location.Y))) + 1;
            Block[numBlock].Location.Width = static_cast<int>(floor(static_cast<double>(Block[numBlock].Location.Width))) + 1;
            Block[numBlock].tempBlockVehiclePlr = A;

            // delay add to below if it will be sorted
            if(!g_compatibility.emulate_classic_block_order)
                treeTempBlockAdd(numBlock);

            numTempBlock++;
        }
    }

    // need to sort the temp blocks in strict compatibility mode, to fully emulate the specific way that switched block clipping works in X64
    if(g_compatibility.emulate_classic_block_order)
    {
        if(numTempBlock > 1)
            qSortBlocksX(numBlock + 1 - numTempBlock, numBlock);

        // restore tempBlock tracking
        for(int A = numBlock + 1 - numTempBlock; A <= numBlock; A++)
        {
            if(Block[A].tempBlockNpcIdx > 0)
                NPC[Block[A].tempBlockNpcIdx].tempBlock = A;
            else if(Block[A].tempBlockVehiclePlr > 0)
                treeTempBlockAdd(A);
        }
    }

    for(int A : NPCQueues::Unchecked)
    {
        if(NPC[A].Active)
            continue;

        CheckNPCWidth(NPC[A]);

        if(NPCQueues::check_active(NPC[A]))
            continue;

        // check for inactive NPCs that are falling off
        if(NPC[A].JustActivated == 0 && !(NPC[A]->IsFish && Maths::iRound(NPC[A].Special) == 2) && NPC[A].Type != NPCID_LAVABUBBLE)
        {
            if(!GameMenu && NPC[A].Location.Y > level[NPC[A].Section].Height + 16)
            {
                StopHit = 0;
                NPCHit(A, 9);
            }
        }
    }

    NPCQueues::Unchecked.clear();


    // need this complex loop syntax because Active can be modified within it
    for(int A : NPCQueues::Active.may_insert_erase)
    {
        if(NPC[A].AttLayer != LAYER_NONE)
        {
            lyrX = NPC[A].Location.X;
            lyrY = NPC[A].Location.Y;
        }

        // all of these are moved into the Active section
#if 0
        Physics.NPCGravity = Physics.NPCGravityReal;

        lyrX = NPC[A].Location.X;
        lyrY = NPC[A].Location.Y;

        if(NPC[A].RealSpeedX != 0)
        {
            NPC[A].Location.SpeedX = NPC[A].RealSpeedX;
            NPC[A].RealSpeedX = 0;
        }


        StopHit = 0;
        if(!NPC[A].Projectile || NPC[A].Type == NPCID_TOOTHY || NPC[A].Type == NPCID_TANK_TREADS)
            NPC[A].Multiplier = 0;
        if(NPC[A].Immune > 0)
            NPC[A].Immune -= 1;
        if(NPC[A].Type == NPCID_VEHICLE && NPC[A].TimeLeft > 1)
            NPC[A].TimeLeft = 100;
#endif

        // Activation collisions
        if(NPC[A].JustActivated != 0)
        {
            if(NPC[A].Active)
            {
                if(NPC[A].Type == NPCID_GOALTAPE)
                {
                    Location_t tempLocation = NPC[A].Location;
                    tempLocation.Height = 8000;
                    int C = 0;
                    for(int B : treeBlockQuery(tempLocation, SORTMODE_COMPAT))
                    {
                        if(CheckCollision(tempLocation, Block[B].Location))
                        {
                            if(C == 0)
                                C = B;
                            else
                            {
                                if(Block[B].Location.Y < Block[C].Location.Y)
                                    C = B;
                            }
                        }
                    }
                    if(C > 0)
                    {
                        NPC[A].Special2 = Block[C].Location.Y + 4;
                        NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height;
                        NPC[A].Special = 1;

                        treeNPCUpdate(A);
                        if(NPC[A].tempBlock > 0)
                            treeNPCSplitTempBlock(A);
                    }
                }
                else if(NPC[A].Type == NPCID_LAVA_MONSTER) // blaarg
                {
                    NPC[A].Location.Y = NPC[A].DefaultLocation.Y + NPC[A].Location.Height + 36;
                    treeNPCUpdate(A);
                    if(NPC[A].tempBlock > 0)
                        treeNPCSplitTempBlock(A);
                }



                else if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET || (NPC[A]->IsFish && NPC[A].Special == 2) || NPC[A].Type == NPCID_GHOST_FAST) // Special Start for Jumping Fish and Bullet Bills
                {
                    if(NPC[A].TimeLeft <= 1)
                    {
                        NPCQueues::Active.erase(A);
                        NPC[A].Active = false;
                        NPC[A].TimeLeft = 0;
                    }
                    else if(NPC[A].Direction == -1 && NPC[A].Location.X < Player[NPC[A].JustActivated].Location.X)
                    {
                        NPCQueues::Active.erase(A);
                        NPC[A].Active = false;
                        NPC[A].TimeLeft = 0;
                    }
                    else if(NPC[A].Direction == 1 && NPC[A].Location.X > Player[NPC[A].JustActivated].Location.X)
                    {
                        NPCQueues::Active.erase(A);
                        NPC[A].Active = false;
                        NPC[A].TimeLeft = 0;
                    }
                    else if(NPC[A]->IsFish && Maths::iRound(NPC[A].Special) == 2)
                    {
                        NPC[A].Location.Y = level[Player[NPC[A].JustActivated].Section].Height - 0.1;
                        NPC[A].Location.SpeedX = (1 + (NPC[A].Location.Y - NPC[A].DefaultLocation.Y) * 0.005) * NPC[A].Direction;
                        NPC[A].Special5 = 1;
                        treeNPCUpdate(A);
                        if(NPC[A].tempBlock > 0)
                            treeNPCSplitTempBlock(A);
                    }
                    else if(NPC[A].Type != NPCID_GHOST_FAST)
                        PlaySoundSpatial(SFX_Bullet, NPC[A].Location);
                }
                else if(NPC[A].Type == NPCID_CANNONENEMY)
                    NPC[A].Special = 100;
            }
            else if(!NPC[A].Generator && !NPCQueues::check_active(NPC[A]))
                NPCQueues::Active.erase(A);

            if(NPC[A].Type == NPCID_STATUE_S3 || NPC[A].Type == NPCID_STATUE_S4)
                NPC[A].Special = iRand(200);
            NPC[A].JustActivated = 0;
            NPC[A].CantHurt = 0;
            NPC[A].CantHurtPlayer = 0;
            if(NPC[A].Type == NPCID_CANNONENEMY)
                NPC[A].Projectile = false;
            if(NPC[A].Type == NPCID_CANNONITEM)
                NPC[A].Projectile = false;
        }
        // check for active NPCs that are falling off
        else if(NPC[A].Location.Y > level[NPC[A].Section].Height && NPC[A].Location.Y > level[NPC[A].Section].Height + 16)
        {
            if(!GameMenu && !(NPC[A]->IsFish && Maths::iRound(NPC[A].Special) == 2) && NPC[A].Type != NPCID_LAVABUBBLE)
                NPCHit(A, 9);
        }



        // Normal operations start here



        if(NPC[A]->IsAVine)
        {
            // .Location.SpeedX = 0
            // .Location.SpeedY = 0
            if(NPC[A].Type == NPCID_GRN_VINE_S3 || NPC[A].Type == NPCID_RED_VINE_S3)
                NPC[A].Frame = BlockFrame[5];
            else if(NPC[A].Type >= NPCID_GRN_VINE_S2 && NPC[A].Type <= NPCID_BLU_VINE_BASE_S2)
                NPC[A].Frame = SpecialFrame[7];
        }
/////////////// BEGIN ACTIVE CODE /////////////////////////////////////////////////////////////
        else if(NPC[A].Active && NPC[A].Killed == 0 && !NPC[A].Generator)
        {
            // don't worry about updating A's tree within this block -- it is done at the end if needed.
            double prevX = NPC[A].Location.X;
            double prevY = NPC[A].Location.Y;
            double prevW = NPC[A].Location.Width;
            double prevH = NPC[A].Location.Height;

            // all this cleanup code was moved here from the top of the loop
            Physics.NPCGravity = Physics.NPCGravityReal;

            StopHit = 0;

            if(NPC[A].RealSpeedX != 0)
            {
                NPC[A].Location.SpeedX = NPC[A].RealSpeedX;
                NPC[A].RealSpeedX = 0;
            }

            if(!NPC[A].Projectile || NPC[A].Type == NPCID_TOOTHY || NPC[A].Type == NPCID_TANK_TREADS)
                NPC[A].Multiplier = 0;
            if(NPC[A].Immune > 0)
                NPC[A].Immune -= 1;
            if(NPC[A].Type == NPCID_VEHICLE && NPC[A].TimeLeft > 1)
                NPC[A].TimeLeft = 100;

            float speedVar = 1; // percent of the NPC it should actually moved. this helps when underwater
            if(NPC[A].Slope > 0 && !(NPC[A]->IsAShell || (NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1)))
            {
                if((NPC[A].Location.SpeedX > 0 && BlockSlope[Block[NPC[A].Slope].Type] == -1) ||
                   (NPC[A].Location.SpeedX < 0 && BlockSlope[Block[NPC[A].Slope].Type] == 1))
                {
                    if(!NPC[A]->CanWalkOn || NPC[A]->IsABlock || NPC[A].Type == NPCID_TANK_TREADS)
                        speedVar = (float)(1 - Block[NPC[A].Slope].Location.Height / Block[NPC[A].Slope].Location.Width * 0.4);
                }
            }
            speedVar = 1;

            if(!NPC[A].Projectile)
                speedVar = speedVar * NPC[A]->Speedvar;

            // water check

            // Things immune to water's effects
            if(NPC[A].Type == NPCID_LAVABUBBLE || NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET || NPC[A].Type == NPCID_HEAVY_THROWN || NPC[A].Type == NPCID_GHOST_S3 || NPC[A].Type == NPCID_GHOST_FAST || NPC[A].Type == NPCID_GHOST_S4 || NPC[A].Type == NPCID_BIG_GHOST || NPC[A].Type == NPCID_STATUE_FIRE || NPC[A].Type == NPCID_VILLAIN_FIRE || NPC[A].Type == NPCID_PET_FIRE || NPC[A].Type == NPCID_PLR_HEAVY || NPC[A].Type == NPCID_CHAR4_HEAVY || NPC[A].Type == NPCID_GOALTAPE || NPC[A].Type == NPCID_SICK_BOSS_BALL || NPC[A].Type == NPCID_HOMING_BALL || NPC[A].Type == NPCID_RED_VINE_TOP_S3 || NPC[A].Type == NPCID_GRN_VINE_TOP_S3 || NPC[A].Type == NPCID_GRN_VINE_TOP_S4 || NPC[A].Type == NPCID_SPIKY_THROWER || NPC[A].Type == NPCID_ITEM_THROWER || NPC[A].Type == NPCID_SAW || NPC[A].Type == NPCID_JUMP_PLANT || NPC[A].Type == NPCID_MAGIC_BOSS_BALL || (NPC[A]->IsACoin && NPC[A].Special == 0) || NPC[A].Type == NPCID_SWORDBEAM || NPC[A].Type == NPCID_FIRE_DISK || NPC[A].Type == NPCID_FIRE_CHAIN)
            {
                NPC[A].Wet = 0;
                NPC[A].Quicksand = 0;
            }
            else
            {
                if(NPC[A].Wet > 0)
                    NPC[A].Wet -= 1;

                if(NPC[A].Quicksand > 0)
                    NPC[A].Quicksand -= 1;

                if(UnderWater[NPC[A].Section])
                    NPC[A].Wet = 2;

                for(int B : treeWaterQuery(NPC[A].Location, SORTMODE_NONE))
                {
                    if(!Water[B].Hidden)
                    {
                        if(CheckCollision(NPC[A].Location, Water[B].Location))
                        {
                            if(NPC[A].Wet == 0 && !NPC[A]->IsACoin)
                            {
                                if(NPC[A].Location.SpeedY >= 1 && (!g_compatibility.fix_submerged_splash_effect || !CheckCollisionIntersect(NPC[A].Location, static_cast<Location_t>(Water[B].Location))))
                                {
                                    Location_t tempLocation;
                                    tempLocation.Width = 32;
                                    tempLocation.Height = 32;
                                    tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                                    tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - tempLocation.Height;
                                    NewEffect(EFFID_WATER_SPLASH, tempLocation);
                                }

                                if(!(NPC[A]->IsFish && NPC[A].Special == 1) && NPC[A].Type != NPCID_LEAF_POWER && NPC[A].Type != NPCID_PLR_FIREBALL)
                                {
                                    if(NPC[A].Location.SpeedY > 0.5)
                                        NPC[A].Location.SpeedY = 0.5;
                                    if(NPC[A].Location.SpeedY < -0.5)
                                        NPC[A].Location.SpeedY = -0.5;
                                }
                                else
                                {
                                    if(NPC[A].Location.SpeedY > 2)
                                        NPC[A].Location.SpeedY = 2;
                                    if(NPC[A].Location.SpeedY < -2)
                                        NPC[A].Location.SpeedY = -2;
                                }

                                if(NPC[A].Type == NPCID_PLATFORM_S3)
                                    NPC[A].Special = NPC[A].Location.SpeedY;
                            }

                            if(Water[B].Quicksand)
                                NPC[A].Quicksand = 2;

                            NPC[A].Wet = 2;
                        }
                    }
                }
            }

            if(NPC[A].Wet == 1 && NPC[A].Location.SpeedY < -1)
            {
                Location_t tempLocation;
                tempLocation.Width = 32;
                tempLocation.Height = 32;
                tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - tempLocation.Height;
                NewEffect(EFFID_WATER_SPLASH, tempLocation);
            }


            if(NPC[A].Wet > 0)
            {
                if(NPC[A].Type == NPCID_ICE_CUBE)
                {
                    NPC[A].Projectile = true;
                    Physics.NPCGravity = -Physics.NPCGravityReal * 0.2;
                }
                else
                    Physics.NPCGravity = Physics.NPCGravityReal * 0.2;

                if(NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL && NPC[A].Special4 == 1)
                    NPC[A].Special5 = 0;
                else if(!NPC[A]->IsFish && NPC[A].Type != NPCID_RAFT && NPC[A].Type != NPCID_WALL_BUG && NPC[A].Type != NPCID_WALL_SPARK && NPC[A].Type != NPCID_WALL_TURTLE)
                    speedVar = (float)(speedVar * 0.5);
                else if(NPC[A]->IsFish && NPC[A].Special == 2 && NPC[A].Location.SpeedY > 0)
                    speedVar = (float)(speedVar * 0.5);

                if(NPC[A].Location.SpeedY >= 3) // Terminal Velocity in water
                    NPC[A].Location.SpeedY = 3;

                if(NPC[A].Location.SpeedY < -3)
                    NPC[A].Location.SpeedY = -3;
            }
            else if(!(NPC[A].Type != NPCID_RAFT && !NPC[A]->IsFish))
            {
                NPC[A].WallDeath += 2;

                if(NPC[A].WallDeath >= 10)
                    NPC[A].WallDeath = 10;
            }

            if(NPC[A].Quicksand > 0 && !NPC[A]->NoClipping)
            {
                NPC[A].Location.SpeedY += 1;

                if(NPC[A].Location.SpeedY < -1)
                    NPC[A].Location.SpeedY = -1;
                else if(NPC[A].Location.SpeedY > 0.5)
                    NPC[A].Location.SpeedY = 0.5;

                speedVar = (float)(speedVar * 0.3);
            }




            if(NPC[A].Type == NPCID_VEGGIE_RANDOM)
            {
                int B = iRand(9);
                NPC[A].Type = NPCID(NPCID_VEGGIE_2 + B);
                if(NPC[A].Type == NPCID_VEGGIE_RANDOM)
                    NPC[A].Type = NPCID_VEGGIE_1;
                NPC[A].Location.X += NPC[A].Location.Width / 2.0;
                NPC[A].Location.Y += NPC[A].Location.Height / 2.0;
                NPC[A].Location.Width = NPC[A]->TWidth;
                NPC[A].Location.Height = NPC[A]->THeight;
                NPC[A].Location.X += -NPC[A].Location.Width / 2.0;
                NPC[A].Location.Y += -NPC[A].Location.Height / 2.0;
                NPCQueues::Unchecked.push_back(A);
            }

            if(NPC[A].Text != STRINGINDEX_NONE)
            {
                NPC[A].Chat = false;
                Location_t tempLocation = NPC[A].Location;
                tempLocation.Y -= 25;
                tempLocation.Height += 50;
                tempLocation.X -= 25;
                tempLocation.Width += 50;
                for(int B = 1; B <= numPlayers; B++)
                {
                    if(CheckCollision(tempLocation, Player[B].Location))
                        NPC[A].Chat = true;
                }
            }

            // oldDirection = NPC[A].Direction;

            if(NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_BIG_BULLET)
            {
                if(NPC[A].CantHurt > 0)
                {
                    NPC[A].CantHurt = 10000;
                    if(NPC[A].Type == NPCID_BIG_BULLET)
                        NPC[A].Location.SpeedX = 4 * NPC[A].Direction;
                }
                if(NPC[A].TimeLeft > 3 && !BattleMode)
                    NPC[A].TimeLeft = 3;
            }

            if(NPC[A].Type == NPCID_MAGIC_BOSS || NPC[A].Type == NPCID_MAGIC_BOSS_SHELL || NPC[A].Type == NPCID_FIRE_BOSS || NPC[A].Type == NPCID_FIRE_BOSS_SHELL) // koopalings
            {
                if(NPC[A].TimeLeft > 1)
                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
            }

            CheckSectionNPC(A);

            if((NPC[A].Type == NPCID_VILLAIN_S3 || NPC[A].Type == NPCID_FIRE_DISK || NPC[A].Type == NPCID_FIRE_CHAIN) && NPC[A].TimeLeft > 1)
                NPC[A].TimeLeft = 100;

            if(!(NPC[A].Type == NPCID_PLR_FIREBALL || (NPC[A]->IsFish && fiEqual(NPC[A].Special, 2)) ||
                 NPC[A].Type == NPCID_TOOTHY || NPC[A].Type == NPCID_VEHICLE || NPC[A].Type == NPCID_YEL_PLATFORM || NPC[A].Type == NPCID_BLU_PLATFORM ||
                 NPC[A].Type == NPCID_GRN_PLATFORM || NPC[A].Type == NPCID_RED_PLATFORM || NPC[A].Type == NPCID_VILLAIN_S3 || NPCIsYoshi(NPC[A])) &&
                 NPC[A].HoldingPlayer == 0)
            {
                int C = 0;
                for(int B = 1; B <= numPlayers; B++)
                {
                    if(Player[B].Section == NPC[A].Section)
                        C = 1;
                }
                if(C == 0 && NPC[A].TimeLeft > 1)
                    NPC[A].TimeLeft = 0;
            }
            if((NPC[A].Type == NPCID_RED_VINE_TOP_S3 || NPC[A].Type == NPCID_GRN_VINE_TOP_S3 || NPC[A].Type == NPCID_GRN_VINE_TOP_S4) && NPC[A].TimeLeft > 10)
                NPC[A].TimeLeft = 100;
            if(NPC[A].TimeLeft > 10 && NoTurnBack[NPC[A].Section])
                NPC[A].TurnBackWipe = true;
            if(NPC[A].TimeLeft < 1)
                Deactivate(A);
            NPC[A].TimeLeft -= 1;
            if(NPC[A].Effect == NPCEFF_NORMAL)
            {



                // this code is for NPCs that are being held by a player

                if(NPC[A].HoldingPlayer > 0) // NPC is held
                {
                    NPC[A].vehiclePlr = 0;
                    if(NPC[A].Type == NPCID_VEHICLE)
                    {
                        Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                        NPC[A].HoldingPlayer = 0;
                    }
                    if(Player[NPC[A].HoldingPlayer].HoldingNPC == A && Player[NPC[A].HoldingPlayer].TimeToLive == 0 && !Player[NPC[A].HoldingPlayer].Dead) // Player and NPC are on the same page
                    {
                        NPC[A].Multiplier = 0;

                        if(NPC[A].Type == NPCID_LIFT_SAND)
                        {
                            Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                            NPC[A].HoldingPlayer = 0;
                            NPC[A].Killed = 9;
                            NPCQueues::Killed.push_back(A);
                            NewEffect(EFFID_SMOKE_S3, NPC[A].Location);
                        }

                        if(NPCIsYoshi(NPC[A]))
                        {
                            NPC[A].Special = NPC[A].Type;
                            NPC[A].Type = NPCID_ITEM_POD;
                        }

                        if(NPC[A].Type == NPCID_ITEM_BURIED)
                        {
                            if(NPC[A].Special == 0.0)
                                NPC[A].Special = NPCID_VEGGIE_RANDOM;

                            if(NPC[A].Generator)
                            {
                                NPC[A].Generator = false;
                                NPCQueues::update(A);
                            }

                            NPC[A].Frame = 0;
                            NPC[A].Type = NPCID(NPC[A].Special);
                            NPC[A].Special = 0;

                            if(NPCIsYoshi(NPC[A]))
                            {
                                NPC[A].Special = NPC[A].Type;
                                NPC[A].Type = NPCID_ITEM_POD;
                            }

                            if(!(NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_SPRING || NPC[A].Type == NPCID_KEY ||
                                 NPC[A].Type == NPCID_COIN_SWITCH || NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_TNT || NPC[A].Type == NPCID_BLU_BOOT ||
                                 NPC[A].Type == NPCID_RED_BOOT || NPC[A].Type == NPCID_GRN_BOOT ||
                                 // Duplicated segment [PVS Studio]
                                 // NPC[A].Type == NPCID_BLU_BOOT ||
                                 NPC[A].Type == NPCID_TOOTHYPIPE || NPCIsAnExit(NPC[A])))
                            {
                                NPC[A].DefaultType = NPCID_NULL;
                            }

                            NPC[A].Location.Height = NPC[A]->THeight;
                            NPC[A].Location.Width = NPC[A]->TWidth;

                            if(NPC[A].Type == NPCID_VEGGIE_RANDOM)
                            {
                                int B = iRand(9);
                                NPC[A].Type = NPCID(NPCID_VEGGIE_2 + B);
                                if(NPC[A].Type == NPCID_VEGGIE_RANDOM)
                                    NPC[A].Type = NPCID_VEGGIE_1;
                                NPC[A].Location.X += NPC[A].Location.Width / 2.0;
                                NPC[A].Location.Y += NPC[A].Location.Height / 2.0;
                                NPC[A].Location.Width = NPC[A]->TWidth;
                                NPC[A].Location.Height = NPC[A]->THeight;
                                NPC[A].Location.X += -NPC[A].Location.Width / 2.0;
                                NPC[A].Location.Y += -NPC[A].Location.Height / 2.0;
                            }

                            NPCQueues::Unchecked.push_back(A);
                        }

                        if(NPC[A].Type == NPCID_SLIDE_BLOCK)
                            NPC[A].Special = 1;

                        if(NPC[A].Type == NPCID_SPIT_GUY_BALL)
                        {
                            NPC[A].Location.X += NPC[A].Location.Width / 2.0;
                            NPC[A].Location.Y += NPC[A].Location.Height / 2.0;
                            NPC[A].Type = NPCID_COIN_S2;
                            NPC[A].Location.Height = NPC[A]->THeight;
                            NPC[A].Location.Width = NPC[A]->TWidth;
                            NPC[A].Location.X += -NPC[A].Location.Width / 2.0;
                            NPC[A].Location.Y += -NPC[A].Location.Height / 2.0;

                            NPCQueues::Unchecked.push_back(A);
                        }

                        NPC[A].TimeLeft = 100;
                        NPC[A].BeltSpeed = 0;

                        if(NPC[A].Type == (NPC[A]->IsFish && NPC[A].Special == 2))
                            NPC[A].Special5 = 0;

                        NPC[A].Direction = Player[NPC[A].HoldingPlayer].Direction; // Face the player
                        NPC[A].Location.SpeedY = Player[NPC[A].HoldingPlayer].Location.SpeedY;
                        NPC[A].Location.SpeedX = 0;

                        if(!(NPC[A].Type == NPCID_FLIPPED_RAINBOW_SHELL || NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_SPRING || NPC[A].Type == NPCID_COIN_SWITCH ||
                             NPC[A].Type == NPCID_TIME_SWITCH || NPC[A].Type == NPCID_TNT || NPC[A].Type == NPCID_BLU_BOOT || NPC[A].Type == NPCID_GRN_BOOT || NPC[A].Type == NPCID_RED_BOOT ||
                             // Duplicated segment [PVS Studio]
                             // NPC[A].Type == NPCID_BLU_BOOT ||
                             NPC[A].Type == NPCID_TOOTHYPIPE || NPC[A].Type == NPCID_BOMB || (NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D) ||
                             NPC[A].Type == NPCID_KEY || NPC[A].Type == NPCID_TIMER_S2 || NPC[A].Type == NPCID_FLY_BLOCK || NPC[A].Type == NPCID_FLY_CANNON || NPC[A].Type == NPCID_CHAR4_HEAVY))
                        {
                            for(int B : treeNPCQuery(NPC[A].Location, SORTMODE_ID))
                            {
                                if(B != A && NPC[B].Active &&
                                   (NPC[B].HoldingPlayer == 0 || (BattleMode && NPC[B].HoldingPlayer != NPC[A].HoldingPlayer)) &&
                                   !NPC[B]->IsABonus &&
                                   (NPC[B].Type != NPCID_PLR_FIREBALL  || (BattleMode && NPC[B].CantHurtPlayer != NPC[A].HoldingPlayer)) &&
                                   (NPC[B].Type != NPCID_PLR_ICEBALL || (BattleMode && NPC[B].CantHurtPlayer != NPC[A].HoldingPlayer)) &&
                                    NPC[B].Type != NPCID_CANNONENEMY && NPC[B].Type != NPCID_CANNONITEM &&  NPC[B].Type != NPCID_SPRING && NPC[B].Type != NPCID_KEY &&
                                    NPC[B].Type != NPCID_COIN_SWITCH && NPC[B].Type != NPCID_TIME_SWITCH && NPC[B].Type != NPCID_TNT && NPC[B].Type != NPCID_RED_BOOT &&
                                    NPC[B].Type != NPCID_GRN_BOOT && !(NPC[B].Type == NPCID_BLU_BOOT && NPC[A].Type == NPCID_BLU_BOOT) &&
                                    NPC[B].Type != NPCID_STONE_S3 && NPC[B].Type != NPCID_STONE_S4 && NPC[B].Type != NPCID_GHOST_S3 &&
                                    NPC[B].Type != NPCID_SPIT_BOSS && !(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Special == 0.0) &&
                                    NPC[B].Type != NPCID_ITEM_BURIED && NPC[B].Type != NPCID_LIFT_SAND && NPC[B].Type != NPCID_FLIPPED_RAINBOW_SHELL &&
                                   !(NPC[B].Type == NPCID_HEAVY_THROWN && NPC[B].Projectile) && NPC[B].Type != NPCID_EARTHQUAKE_BLOCK && NPC[B].Type != NPCID_ICE_CUBE && NPC[B].Type != NPCID_CHAR3_HEAVY)
                                {
                                    if(NPC[A].CantHurtPlayer != NPC[B].CantHurtPlayer && NPC[B].Killed == 0 && (Player[NPC[A].HoldingPlayer].StandingOnNPC != B) && !NPC[B].Inert)
                                    {
                                        if(CheckCollision(NPC[A].Location, NPC[B].Location))
                                        {
                                            NPCHit(B, 3, A);

                                            if(NPC[B].Killed > 0)
                                            {
                                                NPC[B].Location.SpeedX = Physics.NPCShellSpeed * 0.5 * -Player[NPC[A].HoldingPlayer].Direction;
                                                NPCHit(A, 5, B);
                                            }

                                            if(NPC[A].Killed > 0)
                                                NPC[A].Location.SpeedX = Physics.NPCShellSpeed * 0.5 * Player[NPC[A].HoldingPlayer].Direction;

                                            if(!g_compatibility.fix_held_item_cancel || NPC[A].Killed || NPC[B].Killed)
                                                break;
                                        }
                                    }
                                }
                            }
                        }

                        if(NPC[A].Type == NPCID_ICE_BLOCK || NPC[A].Type == NPCID_ICE_CUBE) // Yoshi Ice
                        {
                            if(iRand(100) >= 93)
                            {
                                Location_t tempLocation;
                                tempLocation.Height = EffectHeight[EFFID_SPARKLE];
                                tempLocation.Width = EffectWidth[EFFID_SPARKLE];
                                tempLocation.X = NPC[A].Location.X - tempLocation.Width / 2.0 + dRand() * NPC[A].Location.Width - 4;
                                tempLocation.Y = NPC[A].Location.Y - tempLocation.Height / 2.0 + dRand() * NPC[A].Location.Height - 4;
                                NewEffect(EFFID_SPARKLE, tempLocation);
                            }
                        }
                    }
                    else // Player and NPC are not on the same page
                    {
                        Player[NPC[A].HoldingPlayer].HoldingNPC = 0;
                        if(NPC[A].Type == NPCID_VINE_BUG)
                            NPC[A].Projectile = true;
                        NPC[A].Location.SpeedX = 0;
                        NPC[A].Location.SpeedY = 0;
                        if(NPC[A].Type == NPCID_HEAVY_THROWER)
                        {
                            NPC[A].Killed = 3;
                            NPCQueues::Killed.push_back(A);
                            NPC[A].Direction = -NPC[A].Direction;
                        }
                        if(NPC[A].Type == NPCID_BULLET)
                        {
                            PlaySoundSpatial(SFX_Bullet, NPC[A].Location);
                            NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
                            NPC[A].Projectile = true;
                            NPC[A].CantHurt = 1000;
                            NPC[A].CantHurtPlayer = NPC[A].HoldingPlayer;
                        }
                        NPC[A].HoldingPlayer = 0;
                    }
                }
                else // NPC is not held
                {
                    if(NPC[A].CantHurt <= 0)
                        NPC[A].CantHurtPlayer = 0;
                    // tempHit = 0;
                    // tempHitBlock = 0;
                    // tempBlockHit[1] = 0;
                    // tempBlockHit[2] = 0;
                    // winningBlock = 0;

                    if((LevelWrap[NPC[A].Section] || LevelVWrap[NPC[A].Section]) && NPC[A].Type != NPCID_HEAVY_THROWN && NPC[A].Type != NPCID_PET_FIRE) // Level wraparound
                    {
                        if(LevelWrap[NPC[A].Section])
                        {
                            if(NPC[A].Location.X + NPC[A].Location.Width < level[NPC[A].Section].X)
                                NPC[A].Location.X = level[NPC[A].Section].Width - 1;
                            else if(NPC[A].Location.X > level[NPC[A].Section].Width)
                                NPC[A].Location.X = level[NPC[A].Section].X - NPC[A].Location.Width + 1;
                        }

                        if(LevelVWrap[NPC[A].Section])
                        {
                            if(NPC[A].Location.Y + NPC[A].Location.Height < level[NPC[A].Section].Y)
                                NPC[A].Location.Y = level[NPC[A].Section].Height - 1;
                            else if(NPC[A].Location.Y > level[NPC[A].Section].Height)
                                NPC[A].Location.Y = level[NPC[A].Section].Y - NPC[A].Location.Height + 1;
                        }
                    }

                    if(NoTurnBack[NPC[A].Section] && NPC[A].Location.X < level[NPC[A].Section].X - NPC[A].Location.Width - 32)
                        NPCHit(A, 9);

                    if(NPC[A].CantHurt > 0)
                    {
                        if(NPC[A].Type != NPCID_CANNONENEMY)
                            NPC[A].CantHurt -= 1;
                    }
                    else
                        NPC[A].CantHurtPlayer = 0;

                    if(NPC[A].Projectile)
                    {
                        if(NPC[A].CantHurtPlayer != 0)
                            NPC[A].BattleOwner = NPC[A].CantHurtPlayer;
                    }
                    else
                        NPC[A].BattleOwner = 0;

                    if(NPC[A]->IsAShell)
                    {
                        NPC[A].Special4 -= 1;
                        if(NPC[A].Special4 < 0)
                            NPC[A].Special4 = 0;
                    }

                    if(NPC[A].TurnAround)
                    {
                        if((NPC[A].Type == NPCID_MAGIC_BOSS || NPC[A].Type == NPCID_FIRE_BOSS) && NPC[A].Special == 0) // larry koopa
                        {
                            if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                            {
                                if(NPC[A].Special2 < 0)
                                    NPC[A].Special3 += 30;
                                NPC[A].Special2 = -1;
                            }
                            else
                            {
                                if(NPC[A].Special2 > 0)
                                    NPC[A].Special3 += 30;
                                NPC[A].Special2 = 1;
                            }

                        }

                        if(NPC[A].Type == NPCID_PLR_ICEBALL)
                            NPCHit(A, 3, A);

                        if(NPC[A]->IsAShell && NPC[A].Location.SpeedX != 0 && NPC[A].Special4 == 0)
                        {
                            NPC[A].Special4 = 5;
                            Location_t tempLocation;
                            tempLocation.Height = 0;
                            tempLocation.Width = 0;
                            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - 16;
                            tempLocation.X = NPC[A].Location.X - 16;
                            if(NPC[A].Direction == 1)
                                tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width - 16;
                            NewEffect(EFFID_STOMP_INIT, tempLocation);
                        }

                        if(NPC[A].Type == NPCID_SAW)
                            NPC[A].Special2 = -NPC[A].Special2;

                        if(!NPC[A]->IsAShell && NPC[A].Type != NPCID_PLR_FIREBALL && NPC[A].Type != NPCID_TANK_TREADS &&
                           NPC[A].Type != NPCID_BULLET && NPC[A].Type != NPCID_VILLAIN_S3 && !NPCIsABot(NPC[A]) &&
                           NPC[A].Type != NPCID_SPIT_BOSS_BALL && NPC[A].Type != NPCID_SPIT_GUY_BALL && !NPCIsVeggie(NPC[A]) &&
                           NPC[A].Type != NPCID_ROCKET_WOOD && NPC[A].Type != NPCID_WALL_SPARK && NPC[A].Type != NPCID_WALL_BUG &&
                           NPC[A].Type != NPCID_WALL_TURTLE && NPC[A].Type != NPCID_PLR_ICEBALL && NPC[A].Type != NPCID_SWORDBEAM)  // Don't turn around if a shell or a fireball
                        {
                            NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                            if(NPC[A].tempBlock > 0)
                                Block[NPC[A].tempBlock].Location.SpeedX = -Block[NPC[A].tempBlock].Location.SpeedX;
                        }

                        NPC[A].TurnAround = false;
                    }

                    if(NPC[A].Type == NPCID_SAW) // play saw sound
                        PlaySoundSpatial(SFX_Saw, NPC[A].Location);



                    // NPC Movement Code
                    // Probably make a single function pointer for this whole block (until RESUME UNIFIED CODE),
                    //   but create subroutines for the most commonly repeated code


                    // POSSIBLE SUBROUTINE: setSpeed

                    // Default Movement Code
                    if((NPCDefaultMovement(NPC[A]) || (NPC[A]->IsFish && NPC[A].Special != 2)) && !((NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4) && NPC[A].Special > 0) && NPC[A].Type != NPCID_ITEM_BURIED)
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
                    else if(NPC[A].Type == NPCID_TANK_TREADS)
                    {
                        NPC[A].Projectile = true;
                        NPC[A].Direction = NPC[A].DefaultDirection;
                        NPC[A].Location.SpeedX = 1 * NPC[A].DefaultDirection;
                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(!(Player[B].Effect == 0 || Player[B].Effect == 3))
                            {
                                NPC[A].Location.SpeedX = 0;
                                NPC[A].Location.SpeedY = 0;
                            }
                        }
                    }
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
                                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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
                                if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                {
                                    C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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
                        // Yoshi Fireball
                    }
                    else if(NPC[A].Type == NPCID_PET_FIRE)
                    {
                        NPC[A].Projectile = true;
                        if(NPC[A].Location.SpeedX == 0)
                            NPC[A].Location.SpeedX = 5 * NPC[A].Direction;
                        // bully
                    }
                    else if(NPC[A].Type == NPCID_BULLY)
                    {
                        if(!NPC[A].Projectile && NPC[A].Special2 == 0.0)
                        {
                            double C = 0;
                            for(int B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                                {
                                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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

                        // Big Koopa Movement Code
                    }
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
                        // spiney eggs
                    }
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
                                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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
                        if(NPC[A].Special == 0.0)
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
                    if((NPCDefaultMovement(NPC[A]) || (NPC[A]->IsFish && NPC[A].Special != 2)) && !((NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4) && NPC[A].Special > 0) && !NPC[A].Projectile)
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
                                if(NPC[A].Wet == 0)
                                {
                                    if(NPC[A].Special5 >= 0)
                                        NPC[A].Special2 -= 1;
                                }
                                else
                                {
                                    NPC[A].Special2 = 6;
                                    NPC[A].Special3 = NPC[A].Location.SpeedX;
                                }
                                if(NPC[A].Special2 <= 0)
                                {
                                    NPC[A].Special3 = NPC[A].Special3 * 0.99;
                                    if(NPC[A].Special3 > -0.1 && NPC[A].Special3 < 0.1)
                                        NPC[A].Special3 = 0;
                                    NPC[A].Location.SpeedX = NPC[A].Special3;
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
                            if(Maths::iRound(NPC[A].Special5) == 1)
                            {
                                if(NPC[A].Location.Y > NPC[A].DefaultLocation.Y)
                                    NPC[A].Location.SpeedY = -4 - (NPC[A].Location.Y - NPC[A].DefaultLocation.Y) * 0.02;
                                else
                                    NPC[A].Special5 = 0;
                            }
                            else
                            {
                                // If .Location.SpeedY < 2 + (.Location.Y - .DefaultLocation.Y) * 0.02 Then
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
                                    if((NPC[A].Location.X < NPC[A].DefaultLocation.X - 100 && NPC[A].Direction == -1) || (NPC[A].Location.X > NPC[A].DefaultLocation.X + 100 && NPC[A].Direction == 1))
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
                                        if(NPC[A].Location.Y > NPC[A].DefaultLocation.Y + 25)
                                            NPC[A].Special4 = 1;
                                        else if(NPC[A].Location.Y < NPC[A].DefaultLocation.Y - 25)
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
                                else if(NPC[A].Type == NPCID_FLY_BLOCK || (g_compatibility.fix_flamethrower_gravity && NPC[A].Type == NPCID_FLY_CANNON))
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


                    // POSSIBLE SUBROUTINE: preMovement

                    if(NPC[A].Location.SpeedY >= 8 && NPC[A].Type != NPCID_FIRE_DISK && NPC[A].Type != NPCID_FIRE_CHAIN)
                        NPC[A].Location.SpeedY = 8;
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
                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
                            {
                                NPC[A].Location.SpeedX = 0;
                                break;
                            }
                        }
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
                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
                            {
                                NPC[A].Location.SpeedX = 0;
                                NPC[A].Location.SpeedY = 0;
                            }
                        }

                        // the following is all new code!

                        if((NPC[A].Special == 2 || NPC[A].Special == 3) && (NPC[A].Special3 != 0))
                        {
                            NPC[A].Location.X = NPC[A].Special3; // Finish alignment
                            NPC[A].Special3 = 0;
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
                                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
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
                        if(!(NPC[A].Location.X == NPC[A].DefaultLocation.X && NPC[A].Location.Y == NPC[A].DefaultLocation.Y) || NPC[A].Type == NPCID_FIRE_POWER_S3)
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


                    // RESUME UNIFIED CODE

                    // Block Collision

                    if(NPC[A].Pinched.Bottom1 > 0)
                        NPC[A].Pinched.Bottom1 -= 1;
                    if(NPC[A].Pinched.Left2 > 0)
                        NPC[A].Pinched.Left2 -= 1;
                    if(NPC[A].Pinched.Top3 > 0)
                        NPC[A].Pinched.Top3 -= 1;
                    if(NPC[A].Pinched.Right4 > 0)
                        NPC[A].Pinched.Right4 -= 1;
                    if(NPC[A].Pinched.Moving > 0)
                        NPC[A].Pinched.Moving -= 1;

                    float oldBeltSpeed = NPC[A].BeltSpeed;
                    bool resetBeltSpeed = false;
                    bool beltClear = false; // "stops belt movement when on a wall" (Redigit)
                    NPC[A].BeltSpeed = 0;
                    float beltCount = 0;
                    float addBelt = 0;
                    NPC[A].onWall = false;
                    int oldSlope = NPC[A].Slope; // previous sloped block the npc was on
                    bool SlopeTurn = false;
                    NPC[A].Slope = 0;
                    if(NPC[A].Location.X < -(FLBlocks - 1) * 32)
                        NPC[A].Location.X = -(FLBlocks - 1) * 32;
                    if(NPC[A].Location.X + NPC[A].Location.Width > (FLBlocks + 1) * 32)
                        NPC[A].Location.X = (FLBlocks + 1) * 32 - NPC[A].Location.Width;

                    if(!(NPC[A]->IsACoin && NPC[A].Special == 0) && !(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 0) &&
                       NPC[A].Type != NPCID_CONVEYOR && NPC[A].Type != NPCID_STATUE_FIRE && NPC[A].Type != NPCID_ITEM_BURIED && NPC[A].Type != NPCID_STAR_EXIT &&
                       NPC[A].Type != NPCID_STAR_COLLECT && !(NPC[A].Type >= NPCID_PLATFORM_S3 && NPC[A].Type <= NPCID_PLATFORM_S1) &&
                       !(NPCIsAnExit(NPC[A]) && ((NPC[A].DefaultLocation.X == NPC[A].Location.X &&
                       NPC[A].DefaultLocation.Y == NPC[A].Location.Y) || NPC[A].Inert)) &&
                       NPC[A].Type != NPCID_LIFT_SAND && NPC[A].Type != NPCID_CHECKPOINT && NPC[A].Type != NPCID_SICK_BOSS_BALL &&
                       !(NPC[A].Type == NPCID_PLANT_FIRE || NPC[A].Type == NPCID_LOCK_DOOR || NPC[A].Type == NPCID_FIRE_DISK || NPC[A].Type == NPCID_FIRE_CHAIN))
                    {
                        double tempHit = 0; // height of block NPC is walking on
                        int tempHitBlock = 0; // index of block NPC is walking on
                        int tempBlockHit[3] = {0}; // keeps track of up to two blocks hit from below
                        int tempHitIsSlope = 0;
                        float tempSpeedA = 0; // speed of ground the NPC is possibly standing on

                        if((!NPC[A]->NoClipping || NPC[A].Projectile) &&
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
                                                    if(NPC[A].coinSwitchBlockType != B && NPC[A].tempBlock != B &&
                                                       !(NPC[A].Projectile && Block[B].tempBlockNoProjClipping()) &&
                                                       !BlockNoClipping[Block[B].Type] && !Block[B].Hidden)
                                                    {
                                                        if(Block[B].tempBlockNpcType == NPCID_TANK_TREADS && !NPC[A]->NoClipping && NPC[A].Type != NPCID_BULLET)
                                                            NPCHit(A, 8);

                                                        // traits of the block's NPC (if it is actually an NPC)
                                                        const NPCTraits_t& blk_npc_tr = NPCTraits[Block[B].tempBlockNpcType];

                                                        int HitSpot;
                                                        if(Block[B].tempBlockNpcType != NPCID_CONVEYOR && (blk_npc_tr.IsABlock || blk_npc_tr.IsAHit1Block || blk_npc_tr.CanWalkOn))
                                                            HitSpot = NPCFindCollision(NPC[A].Location, Block[B].Location);
                                                        else
                                                            HitSpot = FindCollisionBelt(NPC[A].Location, Block[B].Location, oldBeltSpeed);

                                                        if(NPC[A]->IsFish)
                                                        {
                                                            if(NPC[A].Wet == 0)
                                                            {
                                                                if(NPC[A].WallDeath >= 9)
                                                                    HitSpot = 0;
                                                            }
                                                        }

                                                        if(NPC[A].Type == NPCID_PLR_HEAVY || NPC[A].Type == NPCID_SWORDBEAM || NPC[A].Type == NPCID_CHAR4_HEAVY)
                                                        {
                                                            if(Block[B].Type == 457)
                                                                KillBlock(B);
                                                            HitSpot = 0;
                                                        }

                                                        if(NPC[A].Type == NPCID_SWORDBEAM)
                                                            HitSpot = 0;

                                                        if(Block[B].tempBlockVehiclePlr > 0 && ((!NPC[A]->StandsOnPlayer && NPC[A].Type != NPCID_PLR_FIREBALL) || NPC[A].Inert))
                                                            HitSpot = 0;

                                                        if((NPC[A]->IsFish && NPC[A].Special == 2) && HitSpot != 3)
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
                                                        }

                                                        if(NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG)
                                                        {
                                                            if(Block[B].tempBlockVehiclePlr > 0 || Block[B].tempBlockNpcType == NPCID_VEHICLE)
                                                            {
                                                                HitSpot = 0;
                                                                NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                            }
                                                        }

                                                        if(NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M && HitSpot != 1)
                                                            HitSpot = 0;

                                                        if(NPC[A].Type == NPCID_SPIKY_BALL_S3 && (Block[B].tempBlockNpcType == NPCID_CANNONITEM || Block[B].tempBlockNpcType == NPCID_TOOTHYPIPE)) // spiney eggs don't walk on special items
                                                            HitSpot = 0;

                                                        if(NPC[A].Type == NPCID_RAFT) // Skull raft
                                                        {
                                                            if(Block[B].tempBlockNpcType > 0)
                                                                HitSpot = 0;

                                                            if(g_compatibility.fix_skull_raft) // reached a solid wall
                                                            {
                                                                auto bt = Block[B].Type;
                                                                if(Block[B].tempBlockNpcType <= 0 && NPC[A].Special == 1 &&
                                                                  (HitSpot == COLLISION_LEFT || HitSpot == COLLISION_RIGHT) &&
                                                                   BlockSlope[bt] == SLOPE_FLOOR && BlockSlope2[bt] == SLOPE_CEILING &&
                                                                   !BlockOnlyHitspot1[bt] && !BlockIsSizable[bt])
                                                                {
                                                                    if(npcHasFloor(NPC[A]))
                                                                    {
                                                                        SkullRideDone(A, Block[B].Location);
                                                                        NPC[A].Special = 3; // 3 - watcher, 2 - waiter
                                                                    }
                                                                }
                                                            }
                                                        }

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
                                                                if(fiEqual(NPC[A].Special4, 3))
                                                                {
                                                                    NPC[A].Frame = 10;
                                                                    NPC[A].Special3 = 21;
                                                                    NPC[A].Special = 1;
                                                                    NPC[A].Location.SpeedX = 0;
                                                                }
                                                            }
                                                        }

                                                        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL)
                                                        {
                                                            if(Block[B].Type == 626 && fiEqual(NPC[A].Special, 1))
                                                                HitSpot = 0;
                                                            if(Block[B].Type == 627 && fiEqual(NPC[A].Special, 2))
                                                                HitSpot = 0;
                                                            if(Block[B].Type == 628 && fiEqual(NPC[A].Special, 3))
                                                                HitSpot = 0;
                                                            if(Block[B].Type == 629 && fiEqual(NPC[A].Special, 4))
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
                                                                    NPC[numNPCs].DefaultLocation = static_cast<SpeedlessLocation_t>(NPC[numNPCs].Location);
                                                                    NPC[numNPCs].TimeLeft = 100;
                                                                    syncLayers_NPC(numNPCs);
                                                                    CheckSectionNPC(numNPCs);
                                                                }
                                                            }
                                                        }

                                                        if((NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4) && HitSpot != 1)
                                                            HitSpot = 0;

                                                        if(Block[B].tempBlockNpcType == NPCID_CONVEYOR && HitSpot == 5)
                                                        {
                                                            if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Block[B].Location.X + Block[B].Location.Width / 2.0)
                                                                HitSpot = 4;
                                                            else
                                                                HitSpot = 2;
                                                        }

                                                        if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL)
                                                        {
                                                            if(NPCTraits[Block[B].tempBlockNpcType].IsABonus)
                                                                HitSpot = 0;
                                                        }

                                                        if(NPC[A].Type == NPCID_MINIBOSS && HitSpot == 5)
                                                        {
                                                            if(NPC[A].WallDeath >= 5)
                                                            {
                                                                NPC[A].Killed = 3;
                                                                NPCQueues::Killed.push_back(A);
                                                            }
                                                            else
                                                                HitSpot = 3;
                                                        }

                                                        if(BlockIsSizable[Block[B].Type] && HitSpot != 1)
                                                            HitSpot = 0;

                                                        if(BlockIsSizable[Block[B].Type] || BlockOnlyHitspot1[Block[B].Type])
                                                        {
                                                            if(HitSpot != 1 || (NPCIsAParaTroopa(NPC[A]) && NPC[A].Special != 1))
                                                                HitSpot = 0;
                                                        }

                                                        if(NPC[A].Type == NPCID_SPIT_GUY_BALL && HitSpot > 0)
                                                        {
                                                            NPC[A].Killed = 4;
                                                            NPCQueues::Killed.push_back(A);
                                                        }

                                                        if(NPC[A].Type == NPCID_BOMB && NPC[A].Projectile && HitSpot != 0)
                                                            NPC[A].Special = 1000;

                                                        if(NPC[A].Shadow && HitSpot != 1 && !(Block[B].Special > 0 && NPC[A].Projectile))
                                                            HitSpot = 0;


                                                        // vine makers
                                                        if(NPC[A].Type == NPCID_RED_VINE_TOP_S3 || NPC[A].Type == NPCID_GRN_VINE_TOP_S3 || NPC[A].Type == NPCID_GRN_VINE_TOP_S4)
                                                        {
                                                            if(HitSpot == 3)
                                                                NPC[A].Special = 1;
                                                        }

                                                        if(NPC[A].Type == NPCID_GOALTAPE && Block[B].tempBlockNpcType > 0)
                                                            HitSpot = 0;




                                                        if(NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_TURTLE)
                                                        {
                                                            NPC[A].Special5 = 0;
                                                            if(HitSpot == 1)
                                                            {
                                                                if(NPC[A].Special == 4 && NPC[A].Location.X + 0.99 == Block[B].Location.X + Block[B].Location.Width)
                                                                    HitSpot = 0;
                                                                if(NPC[A].Special == 2 && NPC[A].Location.X + NPC[A].Location.Width - 0.99 == Block[B].Location.X)
                                                                    HitSpot = 0;
                                                            }

                                                            if(BlockIsSizable[Block[B].Type] || BlockOnlyHitspot1[Block[B].Type])
                                                                HitSpot = 0;

                                                            if(BlockSlope2[Block[B].Type] != 0 && HitSpot == 3)
                                                            {
                                                                if(NPC[A].Special == 4 && NPC[A].Special2 == -1)
                                                                {
                                                                    if(NPC[A].Location.X - 0.01 == Block[B].Location.X)
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
                                                                if(BlockSlope2[Block[B].Type] != 0)
                                                                {
                                                                    if(HitSpot == 2 || HitSpot == 4)
                                                                        HitSpot = 0;
                                                                }
                                                                else if(HitSpot == 2 || HitSpot == 4)
                                                                    HitSpot = 0;
                                                            }
                                                        }



                                                        if(BlockSlope2[Block[B].Type] != 0 && HitSpot > 0 && ((NPC[A].Location.Y > Block[B].Location.Y) || ((NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_TURTLE) && NPC[A].Special == 3)))
                                                        {
                                                            if(HitSpot == 5)
                                                            {
                                                                if(NPC[A].Special == 2 && NPC[A].Special2 == 1)
                                                                {
                                                                    NPC[A].Special2 = 1;
                                                                    NPC[A].Special = 3;
                                                                }
                                                                if(NPC[A].Special == 4 && NPC[A].Special2 == 1)
                                                                {
                                                                    NPC[A].Special2 = -1;
                                                                    NPC[A].Special = 3;
                                                                }
                                                            }

                                                            HitSpot = 0;
                                                            double PlrMid;
                                                            if(BlockSlope2[Block[B].Type] == 1)
                                                                PlrMid = NPC[A].Location.X + NPC[A].Location.Width;
                                                            else
                                                                PlrMid = NPC[A].Location.X;
                                                            double Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;
                                                            if(BlockSlope2[Block[B].Type] > 0)
                                                                Slope = 1 - Slope;
                                                            if(Slope < 0)
                                                                Slope = 0;
                                                            if(Slope > 1)
                                                                Slope = 1;

                                                            if(NPC[A].Location.Y < Block[B].Location.Y + Block[B].Location.Height - (Block[B].Location.Height * Slope) - 0.1)
                                                            {

                                                                if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_BULLET || NPC[A].Type == NPCID_PLR_ICEBALL)
                                                                    NPCHit(A, 3, A);

                                                                if(NPC[A].Type == NPCID_ICE_CUBE)
                                                                {
                                                                    if(NPC[A].Location.SpeedY < -2)
                                                                        NPCHit(A, 3, A);
                                                                }



                                                                if(fEqual(NPC[A].Location.SpeedY, double(Physics.NPCGravity)) || NPC[A].Slope > 0 || oldSlope > 0)
                                                                {

                                                                    if((NPC[A].Special == 2 || NPC[A].Special == 4) && NPC[A].Special2 == -1)
                                                                    {
                                                                        if(NPC[A].Special == 4)
                                                                            NPC[A].Special2 = 1;
                                                                        if(NPC[A].Special == 2)
                                                                            NPC[A].Special2 = -1;
                                                                        NPC[A].Special = 3;

                                                                    }


                                                                    PlrMid = NPC[A].Location.Y;
                                                                    Slope = (PlrMid - Block[B].Location.Y) / Block[B].Location.Height;
                                                                    if(Slope < 0)
                                                                        Slope = 0;
                                                                    if(Slope > 1)
                                                                        Slope = 1;
                                                                    if(BlockSlope2[Block[B].Type] < 0)
                                                                        NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width - (Block[B].Location.Width * Slope);
                                                                    else
                                                                        NPC[A].Location.X = Block[B].Location.X + (Block[B].Location.Width * Slope) - NPC[A].Location.Width;
                                                                    SlopeTurn = true;
                                                                    if(NPC[A].Location.SpeedX < 0)
                                                                        HitSpot = 2;
                                                                    else
                                                                        HitSpot = 4;
                                                                }
                                                                else
                                                                {
                                                                    NPC[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height - (Block[B].Location.Height * Slope);
                                                                    if(NPC[A].Type == NPCID_WALL_BUG || NPC[A].Type == NPCID_WALL_SPARK || NPC[A].Type == NPCID_WALL_TURTLE)
                                                                    {
                                                                        NPC[A].Location.Y += 1;
                                                                        tempBlockHit[1] = 0;
                                                                        tempBlockHit[2] = 0;
                                                                    }
                                                                    if(NPC[A].Location.SpeedY < -0.01)
                                                                        NPC[A].Location.SpeedY = -0.01 + Block[B].Location.SpeedY;

                                                                    if(NPCIsAParaTroopa(NPC[A]))
                                                                        NPC[A].Location.SpeedY += 2;

                                                                }
                                                            }
                                                        }



                                                        if(BlockSlope[Block[B].Type] != 0 && HitSpot > 0)
                                                        {
                                                            HitSpot = 0;
                                                            if(NPC[A].Location.Y + NPC[A].Location.Height <= Block[B].Location.Y + Block[B].Location.Height + NPC[A].Location.SpeedY + 4)
                                                            {
                                                                if(NPC[A].Location.X < Block[B].Location.X + Block[B].Location.Width && NPC[A].Location.X + NPC[A].Location.Width > Block[B].Location.X)
                                                                {

                                                                    double PlrMid;
                                                                    if(BlockSlope[Block[B].Type] == 1)
                                                                        PlrMid = NPC[A].Location.X;
                                                                    else
                                                                        PlrMid = NPC[A].Location.X + NPC[A].Location.Width;

                                                                    double Slope = (PlrMid - Block[B].Location.X) / Block[B].Location.Width;

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

                                                                    if(NPC[A].Location.Y >= Block[B].Location.Y + (Block[B].Location.Height * Slope) - NPC[A].Location.Height - 0.1)
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


                                                                            NPC[A].Location.Y = Block[B].Location.Y + (Block[B].Location.Height * Slope) - NPC[A].Location.Height - 0.1;
                                                                            if(NPC[A]->IsFish)
                                                                                NPC[A].TurnAround = true;
                                                                            NPC[A].Slope = B;
                                                                            HitSpot = 1;
                                                                            // Fireballs dont go up steep slopes
                                                                            if(Block[B].Location.Height / static_cast<double>(Block[B].Location.Width) >= 1 && ((BlockSlope[Block[B].Type] == -1 && NPC[A].Location.SpeedX > 0) || (BlockSlope[Block[B].Type] == 1 && NPC[A].Location.SpeedX < 0)))
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
                                                                                if(NPC[A].Location.SpeedY > NPC[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type])
                                                                                {
                                                                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[B].Location.Height / static_cast<double>(Block[B].Location.Width)) * BlockSlope[Block[B].Type];
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

                                                        if(NPC[A].Type == NPCID_PLR_FIREBALL && Block[B].tempBlockNpcType == NPCID_ICE_CUBE)
                                                            HitSpot = 0;

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
                                                        if(((g_compatibility.fix_npc55_kick_ice_blocks && NPC[A].Type == NPCID_EXT_TURTLE) || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4) && Block[B].tempBlockNpcType == NPCID_SLIDE_BLOCK)
                                                        {
                                                            if(HitSpot == 2 || HitSpot == 4)
                                                            {
                                                                if(NPC[A].Location.SpeedY == Physics.NPCGravity ||
                                                                   NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0 ||
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
                                                        if(NPC[A].Type == NPCID_SAW && Block[B].tempBlockNpcType > 0)
                                                            HitSpot = 0;
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
                                                            if(NPC[A].Location.SpeedX > -Physics.NPCShellSpeed * 0.8 && NPC[A].Location.SpeedX < Physics.NPCShellSpeed * 0.8)
                                                            {
                                                                if(NPC[A].Location.SpeedY > 5 || NPC[A].Location.SpeedY < -2)
                                                                    NPCHit(A, 3, A);
                                                            }
                                                        }

                                                        if(NPC[A]->IsACoin && NPC[A].Special == 0 && HitSpot > 0)
                                                            NPCHit(A, 3, A);

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
                                                                double C = 0;
                                                                int D = 0;

                                                                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Block[B].Location.X + Block[B].Location.Width / 2.0)
                                                                {
                                                                    C = (Block[B].Location.X + Block[B].Location.Width / 2.0) - (NPC[A].Location.X + NPC[A].Location.Width / 2.0);
                                                                    D = 2;
                                                                }
                                                                else
                                                                {
                                                                    C = (NPC[A].Location.X + NPC[A].Location.Width / 2.0) - (Block[B].Location.X + Block[B].Location.Width / 2.0);
                                                                    D = 4;
                                                                }

                                                                if(NPC[A].Location.Y + NPC[A].Location.Height / 2.0 < Block[B].Location.Y + Block[B].Location.Height / 2.0)
                                                                {
                                                                    if(C < (Block[B].Location.Y + Block[B].Location.Height / 2.0) - (NPC[A].Location.Y + NPC[A].Location.Height / 2.0))
                                                                        D = 1;
                                                                }
                                                                else
                                                                {
                                                                    if(C < (NPC[A].Location.Y + NPC[A].Location.Height / 2.0) - (Block[B].Location.Y + Block[B].Location.Height / 2.0))
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

                                                                if(Block[B].Location.SpeedX != 0.0 && (D == 2 || D == 4))
                                                                    NPC[A].Pinched.Moving = 2;
                                                                if(Block[B].Location.SpeedY != 0.0 && (D == 1 || D == 3))
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
                                                                    NPC[A].Damage += 10000;

                                                                    // NEW bounds check
                                                                    // this reduces the risk of a signed integer overflow, and SMBX 1.3 includes no comparisons to values above 20000
                                                                    // SMBX 1.3 uses a float for Damage, which effectively saturates at such high values
                                                                    if(NPC[A].Damage > 16000)
                                                                        NPC[A].Damage = 16000;

                                                                    NPC[A].Immune = 0;
                                                                    NPC[0].Multiplier = 0;
                                                                    NPCHit(A, 3, 0);
                                                                }
                                                            }
                                                        }

                                                        if(HitSpot == 1 && NPC[A].Type == NPCID_EARTHQUAKE_BLOCK && NPC[A].Location.SpeedY > 2)
                                                            NPCHit(A, 4, A);

                                                        // If a Pokey head stands on a top of another Pokey segment
                                                        if(HitSpot == 1 && NPC[A].Type == NPCID_STACKER && Block[B].tempBlockNpcType == NPCID_STACKER
                                                            && NPC[Block[B].tempBlockNpcIdx].Type == NPCID_STACKER) // Make sure Pokey didn't transformed into ice cube or anything also
                                                        {
                                                            NPC[Block[B].tempBlockNpcIdx].Special = -3;
                                                            NPC[A].Special2 = 0;
                                                        }

                                                        if((NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL) && NPC[A].Special == 5 && HitSpot > 0)
                                                            NPCHit(A, 3, A);
                                                        if(NPC[A].Type == NPCID_PLR_ICEBALL && HitSpot > 1)
                                                            NPCHit(A, 3, A);
                                                        if(NPC[A].Type == NPCID_ITEM_BUBBLE && !BlockIsSizable[Block[B].Type])
                                                            NPCHit(A, 3, A);
                                                        if(NPC[A].Type == NPCID_SPIKY_BALL_S4 && HitSpot == 1)
                                                            NPC[A].Special = 1;
                                                        if(NPC[A].Type == NPCID_DOOR_MAKER && HitSpot == 1)
                                                        {
                                                            NPC[A].Special3 = 1;
                                                            NPC[A].Projectile = false;
                                                        }
                                                        if(NPC[A].Type == NPCID_CHAR3_HEAVY && HitSpot > 0)
                                                            NPCHit(A, 3, A);

                                                        // safe to leave uninitialized, they're only read if g_compatibility.fix_npc_downward_clip is set
                                                        double tempHitOld;
                                                        int tempHitOldBlock;

                                                        if(g_compatibility.fix_npc_downward_clip)
                                                        {
                                                            tempHitOld = tempHit;
                                                            tempHitOldBlock = tempHitBlock;
                                                        }

                                                        // hitspot 1
                                                        if(HitSpot == 1) // Hitspot 1
                                                        {
                                                            if((NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL) && NPC[A].Location.SpeedX == 0)
                                                                NPCHit(A, 4, A);


                                                            if(NPC[A].Type == NPCID_GOALTAPE)
                                                                NPC[A].Special = 1;
                                                            if(NPC[A].Type == NPCID_SQUID_S3 || NPC[A].Type == NPCID_SQUID_S1)
                                                                NPC[A].Special4 = 1;
                                                            tempSpeedA = Block[B].Location.SpeedY;
                                                            if(tempSpeedA < 0)
                                                                tempSpeedA = 0;
                                                            if(Block[B].tempBlockNpcIdx > 0 && NPC[Block[B].tempBlockNpcIdx].Type != 57 && NPC[Block[B].tempBlockNpcIdx].Type != 60 && NPC[Block[B].tempBlockNpcIdx].Type != 62 && NPC[Block[B].tempBlockNpcIdx].Type != 64 && NPC[Block[B].tempBlockNpcIdx].Type != 66)
                                                            {
                                                                if(NPC[Block[B].tempBlockNpcIdx].TimeLeft < NPC[A].TimeLeft - 1)
                                                                    NPC[Block[B].tempBlockNpcIdx].TimeLeft = NPC[A].TimeLeft - 1;
                                                                else if(NPC[Block[B].tempBlockNpcIdx].TimeLeft - 1 > NPC[A].TimeLeft)
                                                                    NPC[A].TimeLeft = NPC[Block[B].tempBlockNpcIdx].TimeLeft - 1;
                                                            }
                                                            if(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1 && NPC[A].Location.SpeedX == 0 && NPC[A].Location.SpeedY > 7.95)
                                                                NPCHit(A, 4, A);
                                                            if(NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4)
                                                                NPC[A].Special = 2;

                                                            if((NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG) && NPC[A].Location.SpeedY > Physics.NPCGravity * 20)
                                                                PlaySoundSpatial(SFX_Stone, NPC[A].Location);
                                                            if(NPC[A].Type == NPCID_TANK_TREADS && NPC[A].Location.SpeedY > Physics.NPCGravity * 10)
                                                                PlaySoundSpatial(SFX_Stone, NPC[A].Location);

                                                            if(WalkingCollision3(NPC[A].Location, Block[B].Location, oldBeltSpeed) || NPC[A].Location.Width > 32)
                                                            {
                                                                resetBeltSpeed = true;

                                                                if(Block[B].tempBlockNpcType != 0)
                                                                {
                                                                    if(Block[B].Location.SpeedY > 0 && Block[B].tempBlockNpcType >= NPCID_YEL_PLATFORM && Block[B].tempBlockNpcType <= NPCID_RED_PLATFORM)
                                                                        tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01 + Block[B].Location.SpeedY;
                                                                    else
                                                                        tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                    tempHitBlock = B;
                                                                }
                                                                else
                                                                {
                                                                    tempHitBlock = B;
                                                                    tempHit = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                }
                                                                if(Block[B].tempBlockNpcType >= NPCID_YEL_PLATFORM && Block[B].tempBlockNpcType <= NPCID_RED_PLATFORM)
                                                                {
                                                                    NPC[A].BeltSpeed = 0;
                                                                    beltCount = 0;
                                                                }

                                                                double C = 0;

                                                                if(NPC[A].Location.X > Block[B].Location.X)
                                                                    C = NPC[A].Location.X - 0.01;
                                                                else
                                                                    C = Block[B].Location.X - 0.01;

                                                                if(NPC[A].Location.X + NPC[A].Location.Width < Block[B].Location.X + Block[B].Location.Width)
                                                                    C = (NPC[A].Location.X + NPC[A].Location.Width - C + 0.01);
                                                                else
                                                                    C = (Block[B].Location.X + Block[B].Location.Width - C + 0.01);

                                                                if(Block[B].tempBlockVehiclePlr == 0)
                                                                {
                                                                    if(Block[B].tempBlockNpcType > 0)
                                                                        NPC[A].BeltSpeed += float(Block[B].Location.SpeedX * C) * blk_npc_tr.Speedvar;
                                                                    else
                                                                        NPC[A].BeltSpeed += float(Block[B].Location.SpeedX * C);
                                                                    beltCount += static_cast<float>(C);
                                                                }
                                                            }

                                                            if(tempHitBlock == B)
                                                            {
                                                                if(NPC[A].Type == NPCID_SPIKY_BALL_S3)
                                                                {
                                                                    if(NPC[A].Location.SpeedY > 2)
                                                                    {
                                                                        NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY * 0.7 + Block[B].Location.SpeedY;
                                                                        if(NPC[A].Slope == 0)
                                                                            NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                        tempHit = 0;
                                                                        tempHitBlock = 0;
                                                                    }
                                                                }

                                                                if(((NPC[A]->StandsOnPlayer && !NPC[A].Projectile) || (NPC[A]->IsAShell && NPC[A].Location.SpeedX == 0.0)) && Block[B].tempBlockVehiclePlr > 0)
                                                                {
                                                                    NPC[A].vehicleYOffset = Block[B].tempBlockVehicleYOffset + NPC[A].Location.Height;
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
                                                                            NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
                                                                        tempHit = 0;
                                                                        tempHitOld = 0;
                                                                    }
                                                                    else if(NPC[A].Type == NPCID_PLR_ICEBALL)
                                                                    {
                                                                        NPC[A].Location.SpeedY = -7 + Block[B].Location.SpeedY;
                                                                        if(NPC[A].Slope == 0)
                                                                            NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
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
                                                                    else if(NPC[A].Bouce || NPC[A].Location.SpeedY > 5.8 || ((NPC[A].Type == NPCID_CANNONITEM || NPC[A].Type == NPCID_TOOTHYPIPE) && (NPC[A].Location.SpeedY > 2 || (NPC[A].Location.SpeedX > 1 || NPC[A].Location.SpeedX < -1))))
                                                                    {
                                                                        NPC[A].Bouce = false;
                                                                        if(NPC[A]->IsAShell || (NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1) || NPC[A].Type == NPCID_ICE_CUBE)
                                                                        {
                                                                            if(NPC[A].Slope == 0)
                                                                                NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY * 0.5;
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
                                                                            NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY * 0.6;
                                                                        if(NPC[A].Slope == 0)
                                                                            NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
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
                                                                        else if(NPC[A].Type == NPCID_BULLY)
                                                                        {
                                                                            if(NPC[A].Location.SpeedX > -0.1 && NPC[A].Location.SpeedX < 0.1)
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
                                                        else if(HitSpot == 2) // Hitspot 2
                                                        {
                                                            if(BlockSlope[Block[oldSlope].Type] == 1 && Block[oldSlope].Location.Y == Block[B].Location.Y)
                                                            {
                                                            }
                                                            else
                                                            {
                                                                beltClear = true;
                                                                if(NPC[A].Type == NPCID_VILLAIN_S3)
                                                                    NPC[A].Location.SpeedX = 0;
                                                                addBelt = NPC[A].Location.X;
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
                                                                if(NPC[A].Slope == 0 && !SlopeTurn)
                                                                    NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01;
                                                                if(!(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_TANK_TREADS || NPC[A].Type == NPCID_BULLET))
                                                                    NPC[A].TurnAround = true;
                                                                if(NPCIsAParaTroopa(NPC[A]))
                                                                    NPC[A].Location.SpeedX += -Block[B].Location.SpeedX * 1.2;
                                                                if(NPC[A]->IsAShell)
                                                                    NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                                addBelt = NPC[A].Location.X - addBelt;
                                                            }
                                                        }
                                                        else if(HitSpot == 4) // Hitspot 4
                                                        {
                                                            beltClear = true;
                                                            if(NPC[A].Type == NPCID_VILLAIN_S3)
                                                                NPC[A].Location.SpeedX = 0;
                                                            resetBeltSpeed = true;
                                                            addBelt = NPC[A].Location.X;
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
                                                            if(NPC[A].Slope == 0 && !SlopeTurn)
                                                                NPC[A].Location.X = Block[B].Location.X - NPC[A].Location.Width - 0.01;
                                                            if(!(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_TANK_TREADS || NPC[A].Type == NPCID_BULLET))
                                                                NPC[A].TurnAround = true;
                                                            if(NPCIsAParaTroopa(NPC[A]))
                                                                NPC[A].Location.SpeedX += -Block[B].Location.SpeedX * 1.2;
                                                            if(NPC[A]->IsAShell)
                                                                NPC[A].Location.SpeedX = -NPC[A].Location.SpeedX;
                                                            addBelt = NPC[A].Location.X - addBelt;
                                                        }
                                                        else if(HitSpot == 3) // Hitspot 3
                                                        {
                                                            if(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1)
                                                                NPCHit(A, 4, A);
                                                            if(NPC[A].Type == NPCID_MINIBOSS)
                                                                NPC[A].Special3 = 0;
                                                            if(tempBlockHit[1] == 0)
                                                                tempBlockHit[1] = B;
                                                            else
                                                                tempBlockHit[2] = B;
                                                            if(NPCIsAParaTroopa(NPC[A]))
                                                            {
                                                                NPC[A].Location.SpeedY = 2 + Block[B].Location.SpeedY;
                                                                NPC[A].Location.Y = Block[B].Location.Y + Block[B].Location.Height + 0.1;
                                                            }
                                                        }
                                                        else if(HitSpot == 5) // Hitspot 5
                                                        {
                                                            if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_SPIT_BOSS_BALL)
                                                                NPCHit(A, 4, A);
                                                            beltClear = true;

                                                            if(NPC[A].Type == NPCID_VILLAIN_S3)
                                                                NPC[A].Location.SpeedX = 0;

                                                            NPC[A].onWall = true;

                                                            if(NPC[A].WallDeath >= 5 && !NPC[A]->IsABonus && NPC[A].Type != NPCID_FLY_BLOCK &&
                                                               NPC[A].Type != NPCID_FLY_CANNON && NPC[A].Type != NPCID_RED_BOOT && NPC[A].Type != NPCID_CANNONENEMY && NPC[A].Type != NPCID_CANNONITEM &&
                                                               NPC[A].Type != NPCID_SPRING && NPC[A].Type != NPCID_HEAVY_THROWER && NPC[A].Type != NPCID_KEY && NPC[A].Type != NPCID_COIN_SWITCH &&
                                                               NPC[A].Type != NPCID_GRN_BOOT &&
                                                               // Duplicated segment [PVS Studio]
                                                               // NPC[A].Type != NPCID_RED_BOOT &&
                                                               NPC[A].Type != NPCID_BLU_BOOT && NPC[A].Type != NPCID_TOOTHYPIPE &&
                                                               NPC[A].Type != NPCID_BOMB && NPC[A].Type != NPCID_SATURN && NPC[A].Type != NPCID_FLIPPED_RAINBOW_SHELL && NPC[A].Type != NPCID_EARTHQUAKE_BLOCK &&
                                                               !((NPC[A].Type >= NPCID_CARRY_BLOCK_A && NPC[A].Type <= NPCID_CARRY_BLOCK_D))) // walldeath stuff
                                                            {
                                                                NPC[A].Location.SpeedX = Physics.NPCShellSpeed * 0.5 * NPC[A].Direction;
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
                                                            else if(NPC[A].Type != NPCID_SPIKY_BALL_S3 && !(NPC[A]->IsABlock && Block[B].tempBlockNpcType > 0) && Block[B].tempBlockNpcType != NPCID_CONVEYOR)
                                                            {
                                                                addBelt = NPC[A].Location.X;
                                                                if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 < Block[B].Location.X + Block[B].Location.Width * 0.5)
                                                                    NPC[A].Location.X = Block[B].Location.X - NPC[A].Location.Width - 0.01;
                                                                else
                                                                    NPC[A].Location.X = Block[B].Location.X + Block[B].Location.Width + 0.01;
                                                                addBelt = NPC[A].Location.X - addBelt;
                                                                if(NPC[A].Type == NPCID_MINIBOSS)
                                                                {
                                                                    NPC[A].Location.SpeedY = 0;
                                                                    NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.01;
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
                                                        if(g_compatibility.fix_npc_downward_clip && (tempHitBlock != tempHitOldBlock))
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
                        if(tempBlockHit[1] > 0) // find out which block was hit from below
                        {
                            int winningBlock = 0;

                            if(tempBlockHit[2] == 0)
                                winningBlock = tempBlockHit[1];
                            else
                            {
                                double C = Block[tempBlockHit[1]].Location.X + Block[tempBlockHit[1]].Location.Width * 0.5;
                                double D = Block[tempBlockHit[2]].Location.X + Block[tempBlockHit[2]].Location.Width * 0.5;
                                C -= (NPC[A].Location.X + NPC[A].Location.Width * 0.5);
                                D -= (NPC[A].Location.X + NPC[A].Location.Width * 0.5);

                                if(C < 0)
                                    C = -C;
                                if(D < 0)
                                    D = -D;

                                if(C < D)
                                    winningBlock = tempBlockHit[1];
                                else
                                    winningBlock = tempBlockHit[2];
                            }

                            // possible usually-nulled function pointer here: ceiling collision logic. Takes winningBlock. Returns true if we should set the NPC's speed / location by the ceiling, otherwise false.
                            if(NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL) // Kill the fireball
                                NPCHit(A, 4);
                            else if(NPC[A].Projectile || Block[winningBlock].Invis) // Hit the block hard if the NPC is a projectile
                            {
                                if(!(NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG || NPC[A].Type == NPCID_CANNONENEMY))
                                {
                                    if(NPC[A].Location.SpeedY < -0.05)
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

                            if(!NPCIsAParaTroopa(NPC[A]))
                            {
                                NPC[A].Location.Y = Block[winningBlock].Location.Y + Block[winningBlock].Location.Height + 0.01;

                                if(g_compatibility.fix_npc_ceiling_speed)
                                    NPC[A].Location.SpeedY = 0.01 + Block[winningBlock].Location.SpeedY;
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
                                        double right_border = NPC[A].Location.X + NPC[A].Location.Width;

                                        int first_after_block = -1;
                                        double first_after_x = 0.0, first_after_y = 0.0;

                                        // IF PSwitchTime != 0 but the blocks are sorted,
                                        // then we would have iterated over the NPC's entire FLBlock column without breaking,
                                        // and ended with B set to the first block after it.
                                        // To emulate, move the right_border to the end of its FLBlock column.
                                        // vb6 rounds its array indexes from doubles to integers
                                        if(PSwitchTime != 0)
                                            right_border = vb6Round(right_border / 32.0 + 1) * 32;

                                        // If the loop never invoked break and was not over a single column,
                                        // then the game would have accessed numBlock + 1 here, but we won't.
                                        // We'll assume it was properly deallocated and has SpeedY = 0.

                                        for(int block = fBlock; block <= lBlock; block++)
                                        {
                                            // Old code checked coordinates first, then properties without affecting the loop.
                                            // This means we don't need to touch any properties here.

                                            const Block_t& b = Block[block];

                                            double bx = b.Location.X;
                                            double by = b.Location.Y;

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
                                                    if(g_compatibility.emulate_classic_block_order && numTempBlock == 0)
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
                                        NPC[A].Location.SpeedY = 0.01 + Block[B].Location.SpeedY;
                                        // pLogDebug("NPC %d hits ceiling, set SpeedY using block %d with speed %f\n", A, B, Block[B].Location.SpeedY);
                                    }
                                    else
                                    {
                                        NPC[A].Location.SpeedY = 0.01;
                                        // pLogDebug("NPC %d hits ceiling, set SpeedY using OOB block with speed 0\n", A);
                                    }
                                }
                            }
                        }

                        if(NPC[A]->IsAShell)
                        {
                            if(NPC[A].Special > 0)
                            {
                                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.9;
                                NPC[A].Frame = 0;
                                NPC[A].FrameCount = 0;
                                if(NPC[A].Location.SpeedX > -0.3 && NPC[A].Location.SpeedX < 0.3)
                                {
                                    NPC[A].Location.SpeedX = 0;
                                    NPC[A].Special = 0;
                                    NPC[A].Projectile = false;
                                }
                            }
                        }

                        if(NPC[A].Type == NPCID_TANK_TREADS && NPC[A].Location.SpeedX != 0)
                            NPC[A].Location.SpeedX = 1 * NPC[A].DefaultDirection;

                        // beltspeed code
                        if(!resetBeltSpeed)
                        {
                            if(NPC[A].Type == NPCID_VILLAIN_S3 && NPC[A].Special == 1)
                                NPC[A].Special = 0;
                            if(oldBeltSpeed >= 1 || oldBeltSpeed <= -1)
                            {
                                NPC[A].BeltSpeed = oldBeltSpeed - NPC[A].oldAddBelt;
                                beltCount = 1;
                                if(NPC[A].BeltSpeed >= 2.1f)
                                    NPC[A].BeltSpeed -= 0.1f;
                                else if(NPC[A].BeltSpeed <= -2.1f)
                                    NPC[A].BeltSpeed += 0.1f;
                            }
                        }

                        if(NPC[A].BeltSpeed != 0.0f)
                        {
                            Location_t preBeltLoc = NPC[A].Location;
                            NPC[A].BeltSpeed = NPC[A].BeltSpeed / beltCount;
                            NPC[A].BeltSpeed = NPC[A].BeltSpeed * speedVar;
                            NPC[A].Location.X += double(NPC[A].BeltSpeed);
//                            D = NPC[A].BeltSpeed; // Idk why this is needed as this value gets been overriden and never re-used
                            Location_t tempLocation = NPC[A].Location;
                            tempLocation.Y += 1;
                            tempLocation.Height -= 2;
                            tempLocation.Width = tempLocation.Width / 2;

                            if(NPC[A].BeltSpeed > 0)
                                tempLocation.X += tempLocation.Width;

                            if(!(NPC[A].Type >= NPCID_SHORT_WOOD && NPC[A].Type <= NPCID_SLANT_WOOD_M) && !NPC[A].Inert)
                            {
                                for(int C : treeNPCQuery(tempLocation, SORTMODE_NONE))
                                {
                                    if(A != C && NPC[C].Active && !NPC[C].Projectile)
                                    {
                                        if(NPC[C].Killed == 0 && NPC[C].vehiclePlr == 0 && NPC[C].HoldingPlayer == 0 &&
                                           !NPC[C]->NoClipping && NPC[C].Effect == 0 && !NPC[C].Inert) // And Not NPCIsABlock(NPC(C).Type) Then
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
                                                    NPC[A].onWall = true;
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
                                NPC[A].BeltSpeed = 0;
                                addBelt = 0;
                                if(NPC[A].tempBlock > 0)
                                    Block[NPC[A].tempBlock].Location.SpeedX = 0;
                            }
                        }
                        if(!NPC[A].onWall)
                            NPC[A].BeltSpeed += addBelt;
                        NPC[A].oldAddBelt = addBelt;
                        if(beltClear)
                            NPC[A].BeltSpeed = 0;


                        // possible usually-nulled function pointer for logic between block and NPC collisions

                        if(NPC[A].Type == NPCID_STONE_S3 || NPC[A].Type == NPCID_STONE_S4)
                            NPC[A].BeltSpeed = 0;
                        // End Block Collision
                        // If .Type = 12 Then .Projectile = True 'Stop the big fireballs from getting killed from tha lava
                        if(NPC[A].Type == NPCID_RAINBOW_SHELL)
                            NPC[A].Projectile = true;

                        // restore saw height after block collision and belt logic
                        if(NPC[A].Type == NPCID_SAW)
                        {
                            NPC[A].Location.Height = NPC[A]->THeight;
                            NPC[A].Projectile = true;
                        }


                        // NPC Collision

                        if(!NPC[A].Inert && NPC[A].Type != NPCID_LIFT_SAND && NPC[A].Type != NPCID_CANNONITEM && NPC[A].Type != NPCID_SPRING &&
                                !(NPC[A].Type == NPCID_HEAVY_THROWN && !NPC[A].Projectile) && NPC[A].Type != NPCID_COIN_SWITCH && NPC[A].Type != NPCID_GRN_BOOT &&
                                !(NPC[A].Type == NPCID_SPIT_BOSS_BALL && !NPC[A].Projectile) &&
                                !((NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_YEL_PLATFORM || NPC[A].Type == NPCID_BLU_PLATFORM || NPC[A].Type == NPCID_GRN_PLATFORM ||
                           NPC[A].Type == NPCID_RED_PLATFORM || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT ||
                           NPC[A].Type == NPCID_VPIPE_LONG || NPC[A].Type == NPCID_CANNONENEMY) && !NPC[A].Projectile) &&
                                !(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 0) && !(NPC[A].Type == NPCID_SPIKY_BALL_S3 && !NPC[A].Projectile) &&
                           NPC[A].Type != NPCID_TOOTHYPIPE && NPC[A].Type != NPCID_FALL_BLOCK_RED && NPC[A].Type != NPCID_VEHICLE && NPC[A].Type != NPCID_CONVEYOR &&
                                !NPCIsYoshi(NPC[A]) && !(NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M) &&
                                !(NPC[A].Type == NPCID_ITEM_POD && !NPC[A].Projectile) && !(NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 &&
                           NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4 && NPC[A].Projectile && NPC[A].CantHurt > 0) &&
                                !(NPC[A]->IsAShell && !NPC[A].Projectile) &&
                                !(NPC[A].Projectile && NPC[A].Type >= NPCID_GRN_HIT_TURTLE_S4 && NPC[A].Type <= NPCID_YEL_HIT_TURTLE_S4) &&
                           NPC[A].Type != NPCID_SPIT_GUY_BALL && !(!NPCIsToad(NPC[A]) && !NPC[A].Projectile &&
                           NPC[A].Location.SpeedX == 0 && (NPC[A].Location.SpeedY == 0 || NPC[A].Location.SpeedY == Physics.NPCGravity)))
                        {
                            if(!NPC[A]->IsACoin && NPC[A].Type != NPCID_TIMER_S2 && NPC[A].Type != NPCID_FALL_BLOCK_BROWN &&
                                NPC[A].Type != NPCID_WALL_BUG && NPC[A].Type != NPCID_WALL_SPARK && NPC[A].Type != NPCID_WALL_TURTLE && NPC[A].Type != NPCID_RED_BOOT &&
                                NPC[A].Type != NPCID_BLU_BOOT && !(NPC[A]->IsFish && NPC[A].Special == 2) &&
                               !NPC[A].Generator && NPC[A].Type != NPCID_PLANT_FIRE && NPC[A].Type != NPCID_FIRE_CHAIN &&
                                NPC[A].Type != NPCID_QUAD_BALL && NPC[A].Type != NPCID_FLY_BLOCK && NPC[A].Type != NPCID_FLY_CANNON &&
                                NPC[A].Type != NPCID_FIRE_BOSS_FIRE && NPC[A].Type != NPCID_DOOR_MAKER && NPC[A].Type != NPCID_MAGIC_DOOR)
                            {

                                for(int B : treeNPCQuery(NPC[A].Location, SORTMODE_ID))
                                {
                                    if(B != A && NPC[B].Active)
                                    {
                                        if(!NPC[B]->IsACoin)
                                        {
                                            if(CheckCollision(NPC[A].Location, NPC[B].Location))
                                            {
                                                // if(B != A)
                                                {
                                                    if(!(NPC[B].Type == NPCID_MINIBOSS && NPC[B].Special == 4) && !(NPCIsToad(NPC[B])) &&
                                                       !(NPC[B].Type >= NPCID_PLATFORM_S3 && NPC[B].Type <= NPCID_PLATFORM_S1) && !(NPC[B].Type >= NPCID_CARRY_BLOCK_A && NPC[B].Type <= NPCID_CARRY_BLOCK_D) &&
                                                       NPC[B].Type != NPCID_LIFT_SAND && NPC[B].Type != NPCID_SICK_BOSS_BALL && !NPC[B]->IsAVine &&
                                                       NPC[B].Type != NPCID_PLR_ICEBALL && NPC[B].Type != NPCID_FIRE_CHAIN && NPC[B].Type != NPCID_CHAR3_HEAVY)
                                                    {
                                                        // If Not (NPC(B).Type = 133) And NPC(B).HoldingPlayer = 0 And .Killed = 0 And NPC(B).JustActivated = 0 And NPC(B).Inert = False And NPC(B).Killed = 0 Then
                                                        if(NPC[B].Type != NPCID_SPIT_GUY_BALL && !(NPCIsVeggie(NPC[B]) && NPCIsVeggie(NPC[A])) &&
                                                           NPC[B].HoldingPlayer == 0 && NPC[A].Killed == 0 &&
                                                           NPC[B].JustActivated == 0 && !NPC[B].Inert && NPC[B].Killed == 0)
                                                        {

                                                            if(NPC[B].Type != NPCID_CANNONITEM && NPC[B].Type != NPCID_SWORDBEAM && NPC[B].Type != NPCID_TOOTHYPIPE && NPC[B].Type != NPCID_SPRING &&
                                                               NPC[B].Type != NPCID_HEAVY_THROWN && NPC[B].Type != NPCID_KEY && NPC[B].Type != NPCID_COIN_SWITCH && NPC[B].Type != NPCID_GRN_BOOT &&
                                                               NPC[B].Type != NPCID_VEHICLE && NPC[B].Type != NPCID_TOOTHY && NPC[B].Type != NPCID_CONVEYOR && NPC[B].Type != NPCID_METALBARREL &&
                                                               NPC[B].Type != NPCID_RED_BOOT && NPC[B].Type != NPCID_BLU_BOOT && !NPC[B].Generator &&
                                                               !((NPC[A].Type == NPCID_PLR_FIREBALL || NPC[A].Type == NPCID_PLR_ICEBALL) && NPC[B].Type == NPCID_FLIPPED_RAINBOW_SHELL) &&
                                                               NPC[B].Type != NPCID_TIMER_S2 && NPC[B].Type != NPCID_FLY_BLOCK && NPC[B].Type != NPCID_FLY_CANNON && NPC[B].Type != NPCID_DOOR_MAKER &&
                                                               NPC[B].Type != NPCID_MAGIC_DOOR && NPC[B].Type != NPCID_CHAR3_HEAVY && NPC[B].Type != NPCID_PLR_HEAVY && NPC[B].Type != NPCID_CHAR4_HEAVY)
                                                            {
                                                                if(!(NPC[B].Type == NPCID_HPIPE_SHORT || NPC[B].Type == NPCID_YEL_PLATFORM || NPC[B].Type == NPCID_BLU_PLATFORM ||
                                                                   NPC[B].Type == NPCID_GRN_PLATFORM || NPC[B].Type == NPCID_RED_PLATFORM || NPC[B].Type == NPCID_HPIPE_LONG ||
                                                                   NPC[B].Type == NPCID_VPIPE_SHORT || NPC[B].Type == NPCID_VPIPE_LONG) && !(!NPC[A].Projectile &&
                                                                   NPC[B].Type == NPCID_SPIKY_BALL_S3) && !NPCIsYoshi(NPC[B]) && NPC[B].Type != NPCID_FALL_BLOCK_RED &&
                                                                   NPC[B].Type != NPCID_FALL_BLOCK_BROWN && !(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Special == 0) &&
                                                                   NPC[B].Type != NPCID_CONVEYOR && !(NPC[B].Type >= NPCID_TANK_TREADS && NPC[B].Type <= NPCID_SLANT_WOOD_M) &&
                                                                   NPC[B].Type != NPCID_STATUE_S3 && NPC[B].Type != NPCID_STATUE_FIRE && !(NPC[B].Type == NPCID_BULLET &&
                                                                   NPC[B].CantHurt > 0) && NPC[B].Type != NPCID_ITEM_BURIED && !(NPC[A].CantHurtPlayer == NPC[B].CantHurtPlayer &&
                                                                   NPC[A].CantHurtPlayer > 0) && !(NPC[B].Type == NPCID_ITEM_POD && !NPC[B].Projectile) &&
                                                                   NPC[B].Type != NPCID_PET_FIRE && NPC[B].Type != NPCID_PLANT_FIRE && NPC[B].Type != NPCID_QUAD_BALL &&
                                                                   NPC[B].Type != NPCID_FIRE_BOSS_FIRE && NPC[B].Type != NPCID_RED_VINE_TOP_S3 && NPC[B].Type != NPCID_GRN_VINE_TOP_S3 && NPC[B].Type != NPCID_GRN_VINE_TOP_S4)
                                                                {
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
                                                                           NPC[B].Type != NPCID_PLR_FIREBALL && NPC[B].Type != NPCID_PLR_ICEBALL && !(NPC[B].Type == NPCID_BULLET && NPC[B].CantHurt > 0) &&
                                                                           NPC[B].Type != NPCID_TOOTHY && NPC[B].Type != NPCID_PLR_HEAVY && NPC[B].Type != NPCID_CHAR4_HEAVY && NPC[B].Type != NPCID_FLIPPED_RAINBOW_SHELL)
                                                                        {
                                                                            NPCHit(A, 3, B);
                                                                            // HitSpot = 0;
                                                                        }
                                                                    }

                                                                    // turtle enters a shell
                                                                    if((NPC[A].Type == NPCID_GRN_HIT_TURTLE_S4 || NPC[A].Type == NPCID_RED_HIT_TURTLE_S4 || NPC[A].Type == NPCID_YEL_HIT_TURTLE_S4) && !NPC[A].Projectile &&
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
                                                                    else if(NPC[A].Projectile && !(NPC[B].Type == NPCID_SLIDE_BLOCK && NPC[B].Special == 0.0) && NPC[A].Type != NPCID_SWORDBEAM)
                                                                    {
                                                                        if(!(NPC[A].Projectile && NPC[B].Projectile && NPC[A].Type == NPCID_BULLET && NPC[B].Type == NPCID_BULLET && NPC[A].CantHurtPlayer != NPC[B].CantHurtPlayer))
                                                                        {
                                                                            if(!((NPC[A].Type == NPCID_PLR_FIREBALL && NPC[B]->IsABonus) || NPC[B].Type == NPCID_PLR_FIREBALL || NPC[B].Type == NPCID_VILLAIN_FIRE))
                                                                            {
                                                                                if(NPC[A]->IsAShell &&
                                                                                        (NPC[B].Type == NPCID_EXT_TURTLE || NPC[B].Type == NPCID_BLU_HIT_TURTLE_S4) &&
                                                                                        (int(NPC[A].Direction) != int(NPC[B].Direction) || NPC[A].Special > 0) && !NPC[B].Projectile)
                                                                                {
                                                                                    if(int(NPC[A].Direction) == -1)
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
                                                                                    if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > NPC[B].Location.X + NPC[B].Location.Width / 2.0)
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
                                                                            if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > NPC[B].Location.X + NPC[B].Location.Width / 2.0)
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
                                                                            if(((NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4) && NPC[B]->IsAShell) || ((NPC[B].Type == NPCID_EXT_TURTLE || NPC[B].Type == NPCID_BLU_HIT_TURTLE_S4) && NPC[A]->IsAShell)) // Nekkid koopa kicking a shell
                                                                            {
                                                                                if(NPC[A].Type == NPCID_EXT_TURTLE || NPC[A].Type == NPCID_BLU_HIT_TURTLE_S4)
                                                                                {
                                                                                    if(NPC[A].Location.SpeedY == Physics.NPCGravity || NPC[A].Slope > 0)
                                                                                    {
                                                                                        // If .Direction = 1 And .Location.X + .Location.Width < NPC(B).Location.X + 3 Or (.Direction = -1 And .Location.X > NPC(B).Location.X + NPC(B).Location.Width - 3) Then
                                                                                        if((NPC[A].Direction == 1  && NPC[A].Location.X + NPC[A].Location.Width < NPC[B].Location.X + 4) ||
                                                                                           (NPC[A].Direction == -1 && NPC[A].Location.X > NPC[B].Location.X + NPC[B].Location.Width - 4))
                                                                                        {
                                                                                            if(NPC[B].Location.SpeedX == 0.0 && NPC[B].Effect == 0)
                                                                                            {
                                                                                                NPC[A].Special = 10;
                                                                                                Player[numPlayers + 1].Direction = NPC[A].Direction;
                                                                                                NPC[A].Location.X += -NPC[A].Direction;
                                                                                                NPCHit(B, 1, numPlayers + 1);
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
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
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if(NPC[A].WallDeath > 0)
                        {
                            if(NPC[A]->IsFish)
                                NPC[A].WallDeath -= 1;
                            else
                                NPC[A].WallDeath = 0;
                        }

                        if(tempHit != 0) // Walking   // VERIFY ME
                        {
                            // tempSpeedA does not check for walking collisions in vanilla
                            if(g_compatibility.fix_npc_downward_clip)
                            {
                                tempSpeedA = Block[tempHitBlock].Location.SpeedY;
                                if(tempSpeedA < 0)
                                    tempSpeedA = 0;
                            }

                            if(NPC[A].Type == NPCID_RED_FLY_FODDER) // Walking code for Flying Goomba
                            {
                                if(NPC[A].Special <= 30)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = 0;
                                    if(NPC[A].Slope > 0)
                                    {
                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                        if(NPC[A].Location.SpeedY < 0)
                                            NPC[A].Location.SpeedY = 0;
                                    }
                                    if(tempSpeedA != 0)
                                        NPC[A].Location.SpeedY = tempSpeedA;
                                }
                                else if(NPC[A].Special == 31)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = -4;
                                }
                                else if(NPC[A].Special == 32)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = -4;
                                }
                                else if(NPC[A].Special == 33)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = -4;
                                }
                                else if(NPC[A].Special == 34)
                                {
                                    NPC[A].Special = 0;
                                    NPC[A].Location.SpeedY = -7;
                                }
                            }
                            else if(NPC[A].Type == NPCID_FLY_CARRY_FODDER) // Walking code for SMW Flying Goomba
                            {
                                if(NPC[A].Special <= 60)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = 0;
                                    if(NPC[A].Slope > 0)
                                    {
                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                        if(NPC[A].Location.SpeedY < 0)
                                            NPC[A].Location.SpeedY = 0;
                                    }
                                    if(tempSpeedA != 0)
                                        NPC[A].Location.SpeedY = tempSpeedA;
                                }
                                else if(NPC[A].Special == 61)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                else if(NPC[A].Special == 62)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                else if(NPC[A].Special == 63)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                else if(NPC[A].Special == 64)
                                {
                                    NPC[A].Special += 1;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                else if(NPC[A].Special == 65)
                                {
                                    NPC[A].Special = 0;
                                    NPC[A].Location.SpeedY = -7;
                                }
                            }
                            else if(NPC[A]->TurnsAtCliffs && !NPC[A].Projectile) // Walking code NPCs that turn
                            {
                                bool tempTurn = true; // used for turning the npc around
                                Location_t tempLocation = NPC[A].Location;
                                tempLocation.SpeedX = 0;
                                tempLocation.SpeedY = 0;
                                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 8;
                                tempLocation.Height = 16;
                                if(NPC[A].Slope > 0)
                                    tempLocation.Height = 32;
                                tempLocation.Width = 16;

                                bool isPokeyHead = (NPC[A].Type == NPCID_STACKER && NPC[A].Special2 == 0);

                                // If .Location.SpeedX > 0 Then
                                if(NPC[A].Direction > 0)
                                {
                                    tempLocation.X += NPC[A].Location.Width - 20;
                                    if(isPokeyHead)
                                        tempLocation.X += 16;
                                    // If .Type = 189 Then tempLocation.X += -10
                                }
                                else
                                {
                                    tempLocation.X += -tempLocation.Width + 20;
                                    if(isPokeyHead)
                                        tempLocation.X -= 16;
                                    // If .Type = 189 Then tempLocation.X += 10
                                }

                                for(int bCheck2 = 1; bCheck2 <= 2; bCheck2++)
                                {
                                    // if(bCheck2 == 1)
                                    // {
                                    //     // fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
                                    //     // lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                    //     blockTileGet(tempLocation, fBlock2, lBlock2);
                                    // }
                                    // else
                                    // {
                                    //     fBlock2 = numBlock - numTempBlock;
                                    //     lBlock2 = numBlock;
                                    // }
                                    auto collBlockSentinel2 = (bCheck2 == 1)
                                        ? treeFLBlockQuery(tempLocation, SORTMODE_NONE)
                                        : treeTempBlockQuery(tempLocation, SORTMODE_NONE);

                                    for(BlockRef_t block : collBlockSentinel2)
                                    {
                                        int B = block;
                                        //If BlockNoClipping(Block(B).Type) = False And Block(B).Invis = False And Block(B).Hidden = False And Not (BlockIsSizable(Block(B).Type) And Block(B).Location.Y < .Location.Y + .Location.Height - 3) Then

                                        // Don't collapse Pokey during walking on slopes and other touching surfaces
                                        if(g_compatibility.fix_npc247_collapse && isPokeyHead && Block[B].tempBlockNpcType != NPCID_STACKER)
                                            continue;

                                        if((tempLocation.X + tempLocation.Width >= Block[B].Location.X) &&
                                           (tempLocation.X <= Block[B].Location.X + Block[B].Location.Width) &&
                                           (tempLocation.Y + tempLocation.Height >= Block[B].Location.Y) &&
                                           (tempLocation.Y <= Block[B].Location.Y + Block[B].Location.Height) &&
                                           (!BlockNoClipping[Block[B].Type] && !Block[B].Invis && !Block[B].Hidden && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 3)))
                                        {
                                            // If CheckCollision(tempLocation, Block(B).Location) = True Then
                                            tempTurn = false;
                                            break;
                                            // End If
                                        }
                                        else
                                        {
                                            // not working
                                        }

                                        // End If
                                    }

                                    if(!tempTurn)
                                        break;
                                }

                                if(tempTurn)
                                    NPC[A].TurnAround = true;

                                NPC[A].Location.SpeedY = 0;
                                if(NPC[A].Slope > 0)
                                {
                                    NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                    if(NPC[A].Location.SpeedY < 0)
                                        NPC[A].Location.SpeedY = 0;
                                }
                                if(tempSpeedA != 0)
                                    NPC[A].Location.SpeedY = tempSpeedA;
                            }
                            else if(NPC[A].Type == NPCID_JUMPER_S4) // ninja code
                            {
                                bool tempTurn = true; // used for turning the npc around
                                Location_t tempLocation = NPC[A].Location;
                                tempLocation.SpeedX = 0;
                                tempLocation.SpeedY = 0;
                                tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height - 8;
                                tempLocation.Height = 16;
                                if(NPC[A].Slope > 0)
                                    tempLocation.Height = 32;
                                tempLocation.Width = 16;
                                if(NPC[A].Location.SpeedX > 0)
                                    tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width - 16;
                                else
                                    tempLocation.X = NPC[A].Location.X - tempLocation.Width + 16;

                                for(int bCheck2 = 1; bCheck2 <= 2; bCheck2++)
                                {
                                    // if(bCheck2 == 1)
                                    // {
                                    //     // fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
                                    //     // lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                    //     blockTileGet(tempLocation, fBlock2, lBlock2);
                                    // }
                                    // else
                                    // {
                                    //     fBlock2 = numBlock - numTempBlock;
                                    //     lBlock2 = numBlock;
                                    // }
                                    auto collBlockSentinel2 = (bCheck2 == 1)
                                        ? treeFLBlockQuery(tempLocation, SORTMODE_NONE)
                                        : treeTempBlockQuery(tempLocation, SORTMODE_NONE);

                                    for(BlockRef_t block : collBlockSentinel2)
                                    {
                                        int B = block;
                                        if(!BlockNoClipping[Block[B].Type] && !Block[B].Invis && !Block[B].Hidden && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 3))
                                        {
                                            if(CheckCollision(tempLocation, Block[B].Location))
                                            {
                                                tempTurn = false;
                                                break;
                                            }
                                        }

                                        if(!tempTurn)
                                            break;
                                    }
                                }

                                tempLocation = NPC[A].Location;
                                tempLocation.SpeedX = 0;
                                tempLocation.SpeedY = 0;
                                tempLocation.Y = NPC[A].Location.Y + 8;
                                tempLocation.Height -= 16;
                                tempLocation.Width = 32;
                                if(NPC[A].Location.SpeedX > 0)
                                    tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width;
                                else
                                    tempLocation.X = NPC[A].Location.X - tempLocation.Width;

                                // we are able to wrap this whole thing in the inner-loop check that (NPC[A].Slope <= 0)
                                // commenting for now to avoid inadvertently introducing any bugs
                                // if(NPC[A].Slope <= 0)
                                for(int bCheck2 = 1; bCheck2 <= 2; bCheck2++)
                                {
                                    // if(bCheck2 == 1)
                                    // {
                                    //     // fBlock2 = FirstBlock[(tempLocation.X / 32) - 1];
                                    //     // lBlock2 = LastBlock[((tempLocation.X + tempLocation.Width) / 32.0) + 1];
                                    //     blockTileGet(tempLocation, fBlock2, lBlock2);
                                    // }
                                    // else
                                    // {
                                    //     fBlock2 = numBlock - numTempBlock;
                                    //     lBlock2 = numBlock;
                                    // }
                                    auto collBlockSentinel2 = (bCheck2 == 1)
                                        ? treeFLBlockQuery(tempLocation, SORTMODE_NONE)
                                        : treeTempBlockQuery(tempLocation, SORTMODE_NONE);

                                    for(BlockRef_t block : collBlockSentinel2)
                                    {
                                        int B = block;
                                        if(!BlockNoClipping[Block[B].Type] && !Block[B].Invis && !Block[B].Hidden && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < NPC[A].Location.Y + NPC[A].Location.Height - 1))
                                        {
                                            if(CheckCollision(tempLocation, Block[B].Location))
                                            {
                                                if(NPC[A].Slope > 0)
                                                {

                                                }
                                                else if(BlockSlope[Block[B].Type] == 0)
                                                    tempTurn = true;
                                                break;
                                            }
                                        }

                                        if(tempTurn)
                                            break;
                                    }
                                }

                                if(tempTurn)
                                {
                                    NPC[A].Location.Y -= 0.1;
                                    NPC[A].Location.SpeedY = -6.55;
                                }
                                else
                                {
                                    NPC[A].Location.SpeedY = 0;
                                    if(NPC[A].Slope > 0)
                                    {
                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                        if(NPC[A].Location.SpeedY < 0)
                                            NPC[A].Location.SpeedY = 0;
                                    }
                                }

                                if(tempSpeedA != 0)
                                    NPC[A].Location.SpeedY += tempSpeedA;
                            }
                            else // Walking code for everything else
                            {
                                if(NPCIsAParaTroopa(NPC[A]))
                                {
                                    if(NPC[A].Special == 1)
                                        NPC[A].Location.SpeedY = -9;
                                    else
                                    {
                                        if(NPC[A].Location.SpeedY > 0)
                                            NPC[A].Location.SpeedY = -NPC[A].Location.SpeedY;
                                    }
                                }
                                else
                                {
                                    NPC[A].Location.SpeedY = 0;
                                    if(NPC[A].Slope > 0 && !NPC[A]->IsAShell && NPC[A].Type != NPCID_SLIDE_BLOCK)
                                    {
                                        NPC[A].Location.SpeedY = NPC[A].Location.SpeedX * (Block[NPC[A].Slope].Location.Height / static_cast<double>(Block[NPC[A].Slope].Location.Width)) * BlockSlope[Block[NPC[A].Slope].Type];
                                        if(NPC[A].Location.SpeedY < 0)
                                            NPC[A].Location.SpeedY = 0;
                                    }
                                }
                                if(NPC[A].Type == NPCID_BIRD)
                                {
                                    NPC[A].Special += 1;
                                    if(NPC[A].Special <= 3)
                                        NPC[A].Location.SpeedY = -3.5;
                                    else
                                    {
                                        NPC[A].Location.SpeedY = -5.5;
                                        NPC[A].Special = 0;
                                    }
                                }
                                if(NPC[A].Type == NPCID_KNIGHT)
                                {
                                    NPC[A].FrameCount += 1;
                                    if(NPC[A].FrameCount > 1)
                                        NPC[A].FrameCount = 0;
                                    NPC[A].Location.SpeedY = -3;
                                }
                                if(tempSpeedA != 0)
                                    NPC[A].Location.SpeedY = tempSpeedA;
                                if(NPC[A].Type == NPCID_SAW)
                                    NPC[A].Special = NPC[A].Location.SpeedY;

                            }
                            if(NPC[A].Slope == 0)
                                NPC[A].Location.Y = tempHit;
                            tempHit = 0;
                            tempHitBlock = 0;
                            if(Block[tempHitBlock].tempBlockNpcType > 0 && NPC[Block[tempHitBlock].tempBlockNpcIdx].Slope > 0)
                            {
                                // .Location.SpeedY = 0
                                NPC[A].Slope = NPC[Block[tempHitBlock].tempBlockNpcIdx].Slope;
                                // Stop
                            }
                        }
                    }
                }
                if(NPC[A].tempBlock > 0 && (NPC[A].Type < 60 || NPC[A].Type > 66) && NPC[A].Type != NPCID_CONVEYOR)
                {
                    if((NPC[A].Type < 78 || NPC[A].Type > 83) && NPC[A].Type != NPCID_SPRING)
                    {
                        Block[NPC[A].tempBlock].Location = NPC[A].Location;
                        if(NPC[A].Type == NPCID_SPRING)
                        {
                            // the NPC tree accounts for this; no need to split tempBlock
                            Block[NPC[A].tempBlock].Location.Y -= 16;
                            Block[NPC[A].tempBlock].Location.Height += 16;
                        }

                        // tempBlock comes back in sync with NPC
                        if(NPC[A].Location.X != prevX
                            || NPC[A].Location.Y != prevY
                            || NPC[A].Location.Width != prevW
                            || NPC[A].Location.Height != prevH)
                        {
                            prevX = NPC[A].Location.X;
                            prevY = NPC[A].Location.Y;
                            prevW = NPC[A].Location.Width;
                            prevH = NPC[A].Location.Height;

                            treeNPCUpdate(A);

                            if(NPC[A].tempBlockInTree)
                            {
                                NPC[A].tempBlockInTree = false;
                                treeTempBlockRemove(NPC[A].tempBlock);
                            }
                        }

                        // no longer needed; maintaining the sort
#if 0
                        while(Block[NPC[A].tempBlock].Location.X < Block[NPC[A].tempBlock - 1].Location.X && NPC[A].tempBlock > numBlock + 1 - numTempBlock)
                        {

                            tmpBlock = Block[NPC[A].tempBlock - 1];
                            Block[NPC[A].tempBlock - 1] = Block[NPC[A].tempBlock];
                            Block[NPC[A].tempBlock] = tmpBlock;

                            NPC[Block[NPC[A].tempBlock].tempBlockNpcIdx].tempBlock = NPC[A].tempBlock;
                            NPC[A].tempBlock -= 1;

                        }
                        while(Block[NPC[A].tempBlock].Location.X > Block[NPC[A].tempBlock + 1].Location.X && NPC[A].tempBlock < numBlock)
                        {


                            tmpBlock = Block[NPC[A].tempBlock + 1];
                            Block[NPC[A].tempBlock + 1] = Block[NPC[A].tempBlock];
                            Block[NPC[A].tempBlock] = tmpBlock;

                            NPC[Block[NPC[A].tempBlock].tempBlockNpcIdx].tempBlock = NPC[A].tempBlock;
                            NPC[A].tempBlock += 1;




                            // NPC(Block(.tempBlock).tempBlockNpcIdx).tempBlock = .tempBlock
                            // NPC(Block(.tempBlock + 1).tempBlockNpcIdx).tempBlock = .tempBlock + 1


                        }
#endif
                    }
                    Block[NPC[A].tempBlock].Location.SpeedX = NPC[A].Location.SpeedX + NPC[A].BeltSpeed;
                }

                if(NPC[A].Projectile)
                {
                    if(NPC[A].Type == NPCID_SAW || NPC[A].Type == NPCID_METALBARREL || NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_HPIPE_SHORT || NPC[A].Type == NPCID_HPIPE_LONG || NPC[A].Type == NPCID_VPIPE_SHORT || NPC[A].Type == NPCID_VPIPE_LONG || (NPC[A].Type >= NPCID_TANK_TREADS && NPC[A].Type <= NPCID_SLANT_WOOD_M))
                        NPC[A].Projectile = false;
                }

                // obsolete code commented out in SMBX64

                // Pinched code
                // If .Direction <> oldDirection Then
                // .PinchCount += 10
                // Else
                // If .PinchCount > 0 Then
                // .PinchCount += -1
                // If .Pinched = False Then .PinchCount += -1
                // End If
                // End If
                // If .PinchCount >= 14 And .Pinched = False Then
                // .Pinched = True
                // .PinchedDirection = .Direction
                // ElseIf .PinchCount >= 15 Then
                // .PinchCount = 15
                // ElseIf .PinchCount = 0 Then
                // .Pinched = False
                // End If

                // NPC[A].Pinched = false;    // never set to true since SMBX64, removed

                // Special Code for things that work while held
                if(NPC[A].Type == NPCID_BOMB) // SMB2 Bomb
                {
                    // If .Location.SpeedX < -2 Or .Location.SpeedX > 2 Or .Location.SpeedY < -2 Or .Location.SpeedY > 5 Then .Projectile = True
                    NPC[A].Special += 1;
                    if(NPC[A].Special > 250)
                        NPC[A].Special2 = 1;
                    if(NPC[A].Special >= 350 || NPC[A].Special < 0)
                    {
                        Bomb(NPC[A].Location, 2);
                        NPC[A].Killed = 9;
                        NPCQueues::Killed.push_back(A);
                    }
                }
                else if(NPC[A].Type == NPCID_WALK_BOMB_S2) // SMB2 Bob-om
                {
                    NPC[A].Special += 1;
                    if(NPC[A].Special > 450)
                        NPC[A].Special2 = 1;
                    if(NPC[A].Special >= 550 || NPC[A].Special < 0)
                    {
                        Bomb(NPC[A].Location, 2);
                        NPC[A].Killed = 9;
                        NPCQueues::Killed.push_back(A);
                    }
                }
                else if(NPC[A].Type == NPCID_LIT_BOMB_S3) // SMB3 Bomb
                {
                    if(!NPC[A].Inert)
                        NPC[A].Special += 1;
                    if(NPC[A].Special > 250)
                        NPC[A].Special2 = 1;
                    if(NPC[A].Special >= 350 || NPC[A].Special < 0)
                        Bomb(NPC[A].Location, 3);
                }
                else if(NPC[A].Type == NPCID_SKELETON)
                {
                    if(NPC[A].Special > 0)
                    {
                        NPC[A].Special2 += 1;
                        if(NPC[A].Special2 >= 400 && NPC[A].Special3 == 0)
                        {
                            NPC[A].Special = 0;
                            NPC[A].Special2 = 0;
                            NPC[A].Inert = false;
                            NPC[A].Stuck = false;
                        }
                        else if(NPC[A].Special2 >= 300)
                        {
                            if(NPC[A].Special3 == 0)
                            {
                                NPC[A].Location.X += 2;
                                NPC[A].Special3 = 1;
                            }
                            else
                            {
                                NPC[A].Location.X -= 2;
                                NPC[A].Special3 = 0;
                            }
                        }
                    }
                }
                else if(NPC[A].Type == NPCID_VILLAIN_S3) // smb3 bowser
                {
                    // special5 - the player
                    // special4 - what bowser is doing
                    // special3 - counter for what bowser is doing
                    // special2 - counter for what bowser needs to do
                    if(NPC[A].Legacy)
                    {
                        if(NPC[A].TimeLeft > 1)
                        {
                            NPC[A].TimeLeft = 100;
                            if(bgMusic[NPC[A].Section] != 21)
                            {
                                StopMusic();
                                bgMusic[NPC[A].Section] = 21;
                                StartMusic(NPC[A].Section);
                            }
                        }
                    }

                    if(NPC[A].Special4 == 0.0)
                    {
                        NPC[A].Special3 = 0; // reset counter when done
                        if(NPC[A].Direction < 0)
                            NPC[A].Frame = 0;
                        else
                            NPC[A].Frame = 5;
                    }

                    if(NPC[A].Special5 == 0.0) // find player
                    {
                        bool tempBool = false;
                        for(int B = 1; B <= numPlayers; B++)
                        {
                            if(!Player[B].Dead && Player[B].TimeToLive == 0)
                                tempBool = true;
                        }

                        if(!tempBool)
                        {
                            NPC[A].Special5 = 0;
                            NPC[A].Special4 = 2;
                        }
                        else
                        {
                            int B;
                            do
                                B = iRand(numPlayers) + 1;
                            while(Player[B].Dead || Player[B].TimeToLive > 0);
                            NPC[A].Special5 = B;
                        }
                    }

                    // see if facing the player
                    bool tempBool = false;
                    if(NPC[A].Special5 > 0)
                    {
                        if(Player[long(NPC[A].Special5)].Location.X + Player[long(NPC[A].Special5)].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                        {
                            if(NPC[A].Direction < 0)
                                tempBool = true;
                        }
                        else
                        {
                            if(NPC[A].Direction > 0)
                                tempBool = true;
                        }
                    }

                    NPC[A].Special2 += dRand();

                    if(fiEqual(NPC[A].Special4, 0) && tempBool)
                    {
                        if(NPC[A].Special2 >= 200 + dRand() * 400 - dRand() * 200) // hop on player
                        {
                            if(NPC[A].Special2 >= 200 + dRand() * 600)
                                NPC[A].Special2 = 0;
                            NPC[A].Special4 = 3;
                        }
                        else if((NPC[A].Special2 >= 80 && NPC[A].Special2 <= 130) || (NPC[A].Special2 >= 160 + dRand() * 300 && NPC[A].Special2 <= 180 + dRand() * 800)) // shoot fireball
                            NPC[A].Special4 = 4;
                    }

                    if(NPC[A].Inert)
                    {
                        if(fiEqual(NPC[A].Special4, 4) || fiEqual(NPC[A].Special4, 3))
                            NPC[A].Special4 = 0;
                    }

                    if(fiEqual(NPC[A].Special4, 0)) // when not doing anything turn to player
                    {
                        if(!tempBool)
                        {
                            if(NPC[A].Direction > 0)
                                NPC[A].Special4 = -1;
                            if(NPC[A].Direction < 0)
                                NPC[A].Special4 = 1;
                        }
                    }

                    if(NPC[A].Special4 == 0.0) // hop
                        NPC[A].Special4 = 2;

                    if(NPC[A].HoldingPlayer > 0)
                    {
                        if(NPC[A].Direction == -1)
                            NPC[A].Frame = 0;
                        else
                            NPC[A].Frame = 5;
                        NPC[A].Special4 = 9000;
                    }
                    else if(fiEqual(NPC[A].Special4, 9000))
                    {
                        NPC[A].Special5 = NPC[A].CantHurtPlayer;
                        NPC[A].Special4 = 0;
                        NPC[A].Location.SpeedX = 0;
                        NPC[A].Location.SpeedY = 0;
                    }

                    if(fiEqual(NPC[A].Special4, -1)) // turn left
                    {
                        NPC[A].Special3 -= 1;
                        if(NPC[A].Special3 > -5)
                            NPC[A].Frame = 9;
                        else if(NPC[A].Special3 > -10)
                            NPC[A].Frame = 8;
                        else if(NPC[A].Special3 > -15)
                            NPC[A].Frame = 12;
                        else if(NPC[A].Special3 > -20)
                            NPC[A].Frame = 3;
                        else if(NPC[A].Special3 > -25)
                            NPC[A].Frame = 4;
                        else
                        {
                            NPC[A].Special4 = 0;
                            NPC[A].Direction = -1;
                        }
                    }
                    else if(fiEqual(NPC[A].Special4, 1)) // turn right
                    {
                        NPC[A].Special3 += 1;
                        if(NPC[A].Special3 < 5)
                            NPC[A].Frame = 4;
                        else if(NPC[A].Special3 < 10)
                            NPC[A].Frame = 3;
                        else if(NPC[A].Special3 < 15)
                            NPC[A].Frame = 12;
                        else if(NPC[A].Special3 < 20)
                            NPC[A].Frame = 8;
                        else if(NPC[A].Special3 < 25)
                            NPC[A].Frame = 9;
                        else
                        {
                            NPC[A].Special4 = 0;
                            NPC[A].Direction = 1;
                        }
                    }
                    else if(fiEqual(NPC[A].Special4, -10)) // look left
                    {
                        NPC[A].Special3 -= 1;
                        if(NPC[A].Special3 > -5)
                            NPC[A].Frame = 3;
                        else if(NPC[A].Special3 > -10)
                            NPC[A].Frame = 4;
                        else
                        {
                            NPC[A].Special4 = 0;
                            NPC[A].Direction = -1;
                        }
                    }
                    else if(fiEqual(NPC[A].Special4, 10)) // look right
                    {
                        NPC[A].Special3 += 1;
                        if(NPC[A].Special3 < 5)
                            NPC[A].Frame = 8;
                        else if(NPC[A].Special3 < 10)
                            NPC[A].Frame = 9;
                        else
                        {
                            NPC[A].Special4 = 0;
                            NPC[A].Direction = 1;
                        }
                    }
                    else if(fiEqual(NPC[A].Special4, 2)) // hops
                    {
                        if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
                        {
                            if(NPC[A].Special3 < 5)
                            {
                                NPC[A].Special3 += 1;
                                if(NPC[A].Direction < 0)
                                    NPC[A].Frame = 1;
                                else
                                    NPC[A].Frame = 6;
                            }
                            else if(fiEqual(NPC[A].Special3, 5))
                            {
                                NPC[A].Special3 += 1;
                                NPC[A].Location.SpeedY = -3;
                                NPC[A].Location.Y -= 0.1;
                                if(NPC[A].Direction < 0)
                                    NPC[A].Frame = 0;
                                else
                                    NPC[A].Frame = 5;
                            }
                            else if(NPC[A].Special3 < 10)
                            {
                                NPC[A].Special3 += 1;
                                if(NPC[A].Direction < 0)
                                    NPC[A].Frame = 1;
                                else
                                    NPC[A].Frame = 6;
                            }
                            else
                                NPC[A].Special4 = 0;
                        }
                    }
                    else if(fiEqual(NPC[A].Special4, 3)) // jump on player
                    {
                        if(NPC[A].Special3 < -1)
                        {
                            if(NPC[A].Special > 1)
                                NPC[A].Special -= 1;
                            NPC[A].Special3 += 1;
                            if(fiEqual(NPC[A].Special3, -1))
                                NPC[A].Special3 = 6;
                        }
                        else if(NPC[A].Special3 < 5)
                        {
                            NPC[A].Special3 += 1;
                            if(NPC[A].Direction < 0)
                                NPC[A].Frame = 1;
                            else
                                NPC[A].Frame = 6;
                        }
                        else if(fiEqual(NPC[A].Special3, 5))
                        {
                            auto &sx = NPC[A].Location.SpeedX;
                            auto &pl = Player[int(NPC[A].Special5)].Location;
                            NPC[A].Special3 += 1;
                            NPC[A].Location.SpeedY = -12;
                            NPC[A].BeltSpeed = 0;
                            NPC[A].Location.Y -= 0.1;
                            // This formula got been compacted: If something will glitch, feel free to restore back this crap
                            //NPC[A].Location.SpeedX = (static_cast<int>(std::floor(static_cast<double>(((Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 - 16) + 1) / 32))) * 32 + 1 - NPC[A].Location.X) / 50;
                            double pCenter = pl.X + pl.Width / 2.0;
                            sx = std::floor((pCenter - 16.0 + 1.0) / 32.0) * 32.0 + 1.0;
                            sx -= NPC[A].Location.X;
                            sx /= 50;
                            if(sx > 15)
                                sx = 15;
                            else if(sx < -15)
                                sx = -15;
                            NPC[A].Special3 = -50;
                            NPC[A].Special = 10;
                            if(NPC[A].Direction < 0)
                                NPC[A].Frame = 0;
                            else
                                NPC[A].Frame = 5;
                        }
                        else if(fiEqual(NPC[A].Special3, 6))
                        {
                            if(NPC[A].Location.SpeedY > 0)
                            {
                                NPC[A].Frame = 10;
                                NPC[A].Location.SpeedX = 0;
                                NPC[A].Location.SpeedY = 0;
                                NPC[A].Special3 = 10;
                                NPC[A].Projectile = true;
                            }
                        }
                        else if(NPC[A].Special3 < 13)
                        {
                            NPC[A].Location.SpeedY = -2;
                            NPC[A].Special3 += 1;
                        }
                        else if(NPC[A].Special3 < 16)
                        {
                            NPC[A].Location.SpeedY = 2;
                            NPC[A].Special3 += 1;
                        }
                        else if(NPC[A].Special3 < 19)
                        {
                            NPC[A].Location.SpeedY = -2;
                            NPC[A].Special3 += 1;
                        }
                        else if(NPC[A].Special3 < 21)
                        {
                            NPC[A].Location.SpeedY = 2;
                            NPC[A].Special3 += 1;
                        }
                        else if(fiEqual(NPC[A].Special3, 21))
                        {
                            if(NPC[A].Location.SpeedY != 0.0)
                                NPC[A].Location.SpeedY = 10;
                            else
                            {
                                bool legacy = /*NPC[A].Legacy &&*/ (NPC[A].Variant == 1);
                                PlaySoundSpatial(SFX_Stone, NPC[A].Location);
                                NPC[A].Special3 = 30;
                                NPC[A].Frame = 11;
                                NPC[A].Projectile = false;
                                Location_t tempLocation = NPC[A].Location;

                                // Useless self-assignment code [PVS Studio]
                                //tempLocation.X = tempLocation.X; // + 16
                                //tempLocation.Width = tempLocation.Width; // - 32

                                tempLocation.Y += tempLocation.Height - 8;
                                tempLocation.Height = 16;
                                // fBlock = FirstBlock[long(NPC[A].Location.X / 32) - 1];
                                // lBlock = LastBlock[long((NPC[A].Location.X + NPC[A].Location.Width) / 32.0) + 1];
                                // blockTileGet(NPC[A].Location, fBlock, lBlock);

                                for(BlockRef_t block : treeFLBlockQuery(tempLocation, false))
                                {
                                    int B = block;
                                    if(Block[B].Type == 186 && CheckCollision(tempLocation, Block[B].Location) && !Block[B].Hidden)
                                        KillBlock(B);
                                }

                                if(!legacy && g_config.GameplayShakeScreenBowserIIIrd)
                                    doShakeScreen(0, 4, SHAKE_SEQUENTIAL, 7, 0.15);

                                if(legacy) // Classic SMBX 1.0's behavior when Bowser stomps a floor
                                {
                                    // fBlock = FirstBlock[long(level[NPC[A].Section].X / 32) - 1];
                                    // lBlock = LastBlock[long((level[NPC[A].Section].Width) / 32.0) + 2];
                                    // {
                                    //     auto &sec = level[NPC[A].Section];
                                    //     Location_t toShake;
                                    //     toShake.X = sec.X;
                                    //     toShake.Width = (sec.Width - sec.X);
                                    //     blockTileGet(toShake, fBlock, lBlock);
                                    // }

                                    // Shake all blocks up
                                    // for(int B = (int)fBlock; B <= lBlock; B++)
                                    //     BlockShakeUp(B);
                                    {
                                        const auto &sec = level[NPC[A].Section];
                                        Location_t toShake;
                                        toShake.X = sec.X;
                                        toShake.Width = (sec.Width - sec.X);
                                        toShake.Y = sec.Y;
                                        toShake.Height = (sec.Height - sec.Y);
                                        for(BlockRef_t block : treeFLBlockQuery(toShake, false))
                                        {
                                            int B = block;
                                            BlockShakeUp(B);
                                        }
                                    }

                                    // expand down a section at the bottom of destroyed blocks
                                    for(int B = 0; B <= numSections; B++)
                                    {
                                        auto &n = NPC[A];
                                        auto &s = level[B];

                                        if(n.Location.X >= s.X &&
                                           n.Location.X + n.Location.Width <= s.Width &&
                                           n.Location.Y + n.Location.Height + 48 >= s.Y &&
                                           n.Location.Y + n.Location.Height + 48 <= s.Height &&
                                           B != n.Section)
                                        {
                                            n.Special2 = 0;
                                            n.Special3 = 0;
                                            n.Special4 = 2;

                                            auto &ns = level[n.Section];
                                            if(s.X < ns.X)
                                                ns.X = s.X;
                                            if(s.Y < ns.Y)
                                                ns.Y = s.Y;
                                            if(s.Width > ns.Width)
                                                ns.Width = s.Width;
                                            if(s.Height > ns.Height)
                                                ns.Height = s.Height;

                                            s.X = 0;
                                            s.Y = 0;
                                            s.Width = 0;
                                            s.Height = 0;

                                            UpdateSectionOverlaps(B);
                                            UpdateSectionOverlaps(n.Section);

                                            for(int C = 1; C <= numNPCs; C++)
                                            {
                                                auto &nc = NPC[C];
                                                if(nc.Section == B)
                                                    nc.Section = n.Section;
                                            }
                                        }
                                    } // for
                                    SoundPause[2] = 12;
                                }
                            }
                        }
                        else if(NPC[A].Special3 < 35)
                        {
                            NPC[A].Frame = 11;
                            NPC[A].Special3 += 1;
                        }
                        else if(NPC[A].Special3 < 40)
                        {
                            NPC[A].Frame = 12;
                            NPC[A].Special3 += 1;
                            NPC[A].Special5 = 0;
                        }
                        else
                        {
                            if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
                            {
                                NPC[A].Special3 = 0;
                                if(Player[NPC[A].Special5].Location.X + Player[NPC[A].Special5].Location.Width / 2.0 < NPC[A].Location.X + NPC[A].Location.Width / 2.0)
                                    NPC[A].Special4 = -10;
                                else
                                    NPC[A].Special4 = 10;
                            }
                        }
                    }
                    else if(fiEqual(NPC[A].Special4, 4)) // shoot a fireball
                    {
                        NPC[A].Special3 += 1;
                        if(NPC[A].Special3 < 15)
                        {
                            if(NPC[A].Direction < 0)
                                NPC[A].Frame = 13;
                            else
                                NPC[A].Frame = 14;
                        }
                        else if(NPC[A].Special3 < 30)
                        {
                            if(NPC[A].Direction < 0)
                                NPC[A].Frame = 2;
                            else
                                NPC[A].Frame = 7;
                            if(fiEqual(NPC[A].Special3, 29))
                            {
                                numNPCs++;
                                NPC[numNPCs] = NPC_t();
                                NPC[numNPCs].Active = true;
                                NPC[numNPCs].TimeLeft = 100;
                                NPC[numNPCs].Direction = NPC[A].Direction;
                                NPC[numNPCs].Section = NPC[A].Section;
                                NPC[numNPCs].Type = NPCID_VILLAIN_FIRE;
                                if(NPC[numNPCs].Direction > 0)
                                    NPC[numNPCs].Frame = 4;
                                NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;
                                NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                                if(NPC[numNPCs].Direction < 0)
                                    NPC[numNPCs].Location.X = NPC[A].Location.X - 40;
                                else
                                    NPC[numNPCs].Location.X = NPC[A].Location.X + 54;
                                NPC[numNPCs].Location.Y = NPC[A].Location.Y + 19;
                                NPC[numNPCs].Location.SpeedX = 4 * double(NPC[numNPCs].Direction);
                                double C = (NPC[numNPCs].Location.X + NPC[numNPCs].Location.Width / 2.0) - (Player[int(NPC[A].Special5)].Location.X + Player[int(NPC[A].Special5)].Location.Width / 2.0);
                                double D = (NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0) - (Player[int(NPC[A].Special5)].Location.Y + Player[int(NPC[A].Special5)].Location.Height / 2.0);
                                NPC[numNPCs].Location.SpeedY = D / C * NPC[numNPCs].Location.SpeedX;
                                if(NPC[numNPCs].Location.SpeedY > 1)
                                    NPC[numNPCs].Location.SpeedY = 1;
                                else if(NPC[numNPCs].Location.SpeedY < -1)
                                    NPC[numNPCs].Location.SpeedY = -1;
                                syncLayers_NPC(numNPCs);
                                PlaySoundSpatial(SFX_BigFireball, NPC[A].Location);
                            }
                        }
                        else if(NPC[A].Special3 < 45)
                        {
                            if(NPC[A].Direction == -1)
                                NPC[A].Frame = 0;
                            else
                                NPC[A].Frame = 5;
                        }
                        else
                            NPC[A].Special4 = 0;
                    }

                }
                else if(NPC[A].Type == NPCID_HEAVY_THROWER && NPC[A].HoldingPlayer > 0)
                {
                    if(Player[NPC[A].HoldingPlayer].Effect == 0)
                        NPC[A].Special3 += 1;
                    if(NPC[A].Special3 >= 20)
                    {
                        PlaySoundSpatial(SFX_HeavyToss, NPC[A].Location);
                        NPC[A].Special3 = 0; // -15
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        NPC[numNPCs].Location.Height = 32;
                        NPC[numNPCs].Location.Width = 32;
                        NPC[numNPCs].Location.X = NPC[A].Location.X;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                        NPC[numNPCs].Direction = NPC[A].Direction;
                        NPC[numNPCs].Type = NPCID_HEAVY_THROWN;
                        NPC[numNPCs].Shadow = NPC[A].Shadow;
                        NPC[numNPCs].CantHurt = 200;
                        NPC[numNPCs].CantHurtPlayer = NPC[A].HoldingPlayer;
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].TimeLeft = 50;
                        NPC[numNPCs].Location.SpeedY = -8;
                        NPC[numNPCs].Location.SpeedX = 3 * Player[NPC[A].HoldingPlayer].Direction + Player[NPC[A].HoldingPlayer].Location.SpeedX * 0.8;
                        syncLayers_NPC(numNPCs);
                    }
                }
                else if(NPC[A].Type == NPCID_CANNONENEMY || NPC[A].Type == NPCID_CANNONITEM) // Bullet Bill Shooter
                {
                    if(NPC[A].Type == NPCID_CANNONENEMY)
                    {
                        NPC[A].Special += 1;
                        if(NPC[A].HoldingPlayer > 0)
                        {
                            if(Player[NPC[A].HoldingPlayer].Effect == 0)
                                NPC[A].Special += 6;
                        }
                    }
                    else
                    {
                        int shootStep = 10;
                        int shootStepSpin = 20;
                        int shootStepCar = 5;
                        bool keepProjectile = false;

                        int shootBehavior = NPC[A].Variant;

                        switch(shootBehavior)
                        {
                        default:
                        case 0:
                            // SMBX 1.2.1 and newer (shoot fast, don't shoot while projectile)
                            break;
                        case 1:
                            // SMBX 1.2 (shoot fast, keep shoot while projectile)
                            keepProjectile = true;
                            break;
                        case 2:
                            // SMBX older than 1.2 (shoot slow, keep shoot while projectile)
                            keepProjectile = true;
                            shootStep = 5;
                            shootStepSpin = 10;
                            break;
                        }

                        if(NPC[A].HoldingPlayer > 0)
                        {
                            if(Player[NPC[A].HoldingPlayer].SpinJump)
                            {
                                if(NPC[A].Direction != Player[NPC[A].HoldingPlayer].SpinFireDir)
                                {
                                    if(Player[NPC[A].HoldingPlayer].Effect == 0)
                                        NPC[A].Special += shootStepSpin;
                                }
                            }
                            else
                            {
                                if(Player[NPC[A].HoldingPlayer].Effect == 0)
                                    NPC[A].Special += shootStep;
                            }
                        }
                        else if(NPC[A].vehiclePlr > 0)
                            NPC[A].Special += shootStepCar;
                        else if(NPC[A].Projectile && keepProjectile)
                            NPC[A].Special += shootStep;
                    }

                    if(NPC[A].Special >= 200)
                    {
                        double C = 0;

                        if(NPC[A].HoldingPlayer > 0)
                        {
                            if(Player[NPC[A].HoldingPlayer].SpinJump)
                                Player[NPC[A].HoldingPlayer].SpinFireDir = int(NPC[A].Direction);
                        }

                        if(NPC[A].HoldingPlayer == 0 && NPC[A].vehiclePlr == 0 && NPC[A].Type == NPCID_CANNONENEMY)
                        {
                            C = 0;
                            for(int B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                                {
                                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                        if(NPC[A].Location.X + NPC[A].Location.Width / 2.0 > Player[B].Location.X + Player[B].Location.Width / 2.0)
                                            NPC[A].Direction = -1;
                                        else
                                            NPC[A].Direction = 1;
                                        if(!CanComeOut(NPC[A].Location, Player[B].Location))
                                            C = -1;
                                    }
                                }
                            }
                        }

                        if(numNPCs < maxNPCs)
                        {
                            if(fEqual(C, -1) && NPC[A].HoldingPlayer == 0 && NPC[A].vehiclePlr == 0)
                                NPC[A].Special = 0;
                            else if(Player[NPC[A].vehiclePlr].Controls.Run || NPC[A].vehiclePlr == 0)
                            {
                                NPC[A].Special = 0;
                                numNPCs++;
                                NPC[numNPCs] = NPC_t();
                                NPC[numNPCs].Inert = NPC[A].Inert;
                                bool tempBool = false;
                                NPC[numNPCs].Direction = NPC[A].Direction;
                                NPC[numNPCs].DefaultDirection = NPC[A].Direction;
                                if(NPC[A].HoldingPlayer > 0 || NPC[A].vehiclePlr > 0 || (NPC[A].Type == NPCID_CANNONITEM && NPC[A].Projectile))
                                {
                                    NPC[numNPCs].Projectile = true;
                                    NPC[numNPCs].CantHurt = 10000;
                                    NPC[numNPCs].CantHurtPlayer = NPC[A].HoldingPlayer;
                                    NPC[numNPCs].Location.SpeedX = 8 * NPC[numNPCs].Direction;
                                }
                                else if(NPC[A].CantHurtPlayer > 0)
                                {
                                    NPC[numNPCs].Projectile = true;
                                    NPC[numNPCs].CantHurt = 1000;
                                    NPC[numNPCs].CantHurtPlayer = NPC[A].CantHurtPlayer;
                                }
                                else if(NPC[A].Type == NPCID_CANNONITEM)
                                {
                                    tempBool = true;
                                    numNPCs--;
                                }
                                if(!tempBool)
                                {
                                    NPC[numNPCs].Shadow = NPC[A].Shadow;
                                    NPC[numNPCs].Active = true;
                                    NPC[numNPCs].TimeLeft = 100;
                                    NPC[numNPCs].JustActivated = 0;
                                    NPC[numNPCs].Section = NPC[A].Section;
                                    NPC[numNPCs].Type = NPCID_BULLET;
                                    NPC[numNPCs].Location.Width = NPC[numNPCs]->TWidth;
                                    NPC[numNPCs].Location.Height = NPC[numNPCs]->THeight;

                                    if(NPC[numNPCs].Direction > 0)
                                        NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0;
                                    else
                                        NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - NPC[numNPCs].Location.Width;

                                    if(NPC[numNPCs].Direction > 0)
                                        NPC[numNPCs].Frame = 1;
                                    else
                                        NPC[numNPCs].Frame = 0;
                                    NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - NPC[numNPCs].Location.Height / 2.0;
                                    syncLayers_NPC(numNPCs);

                                    Location_t tempLocation = NPC[numNPCs].Location;
                                    tempLocation.X = NPC[numNPCs].Location.X + (NPC[numNPCs].Location.Width / 2.0) * NPC[numNPCs].Direction;
                                    tempLocation.Y = NPC[numNPCs].Location.Y + NPC[numNPCs].Location.Height / 2.0 - EffectHeight[10] / 2.0;
                                    NewEffect(EFFID_SMOKE_S3, tempLocation);

                                    PlaySoundSpatial(SFX_Bullet, NPC[A].Location);
                                }
                            }
                        }
                    }
                }
                else if(NPC[A].Type == NPCID_TOOTHY)
                {
                    int B = 0;
                    if(NPC[A].Special > 0)
                    {
                        if(Player[NPC[A].Special].HoldingNPC > 0)
                        {
                            if(NPC[Player[NPC[A].Special].HoldingNPC].Type == 49)
                                B = 1;
                        }
                    }
                    else if(NPC[(int)NPC[A].Special2].Projectile && NPC[(int)NPC[A].Special2].Active)
                    {
                        B = 1;
                        NPC[A].Projectile = true;
                        NPC[A].Direction = NPC[(int)NPC[A].Special2].Direction;
                        if(NPC[A].Direction > 0)
                            NPC[A].Location.X = NPC[(int)NPC[A].Special2].Location.X + 32;
                        else
                            NPC[A].Location.X = NPC[(int)NPC[A].Special2].Location.X - NPC[A].Location.Width;
                        NPC[A].Location.Y = NPC[(int)NPC[A].Special2].Location.Y;
                    }

                    if(Player[NPC[A].vehiclePlr].Controls.Run)
                        B = 1;

                    if(NPC[A].Special2 > 0 && NPC[(int)NPC[A].Special2].Special2 != A)
                        B = 0;

                    if(NPC[A].Special > 0)
                    {
                        if(Player[NPC[A].Special].Effect != 0)
                            B = 0;
                    }

                    if(B == 0)
                    {
                        NPC[A].Killed = 9;
                        NPCQueues::Killed.push_back(A);
                    }
                }
                else if(NPC[A].Type == NPCID_TOOTHYPIPE)
                {
                    if(NPC[A].HoldingPlayer == 0 && NPC[A].vehiclePlr == 0)
                        NPC[A].Special = 0;
                    if(NPC[A].HoldingPlayer > 0 && NPC[A].Special2 > 0)
                        NPC[(int)NPC[A].Special2].Direction = NPC[A].Direction;
                    if(Player[NPC[A].HoldingPlayer].Effect != 0)
                        NPC[A].Special = 0;
#if 0
                    // Important: this also makes a thrown handheld plant harm NPCs, so it is a major balance change.
                    // Since it was disabled in SMBX code, better not to change it. -- ds-sloth
                    // In original game, this is a dead code because of "And 0" condition at end.
                    // In this sample, the "& false" was been commented
                    // This code makes Toothy shown off the pipe when the pipe is a projectile, shooted by generator
                    if(NPC[A].Projectile && NPC[A].Special2 == 0.0 && NPC[A].Special == 0.0 /*&& false*/)
                    {
                        numNPCs++;
                        NPC[numNPCs] = NPC_t();
                        NPC[A].Special2 = numNPCs;
                        NPC[numNPCs].Active = true;
                        NPC[numNPCs].Section = NPC[A].Section;
                        NPC[numNPCs].TimeLeft = 100;
                        NPC[numNPCs].Type = NPCID_TOOTHY;
                        NPC[numNPCs].Location.Height = 32;
                        NPC[numNPCs].Location.Width = 48;
                        NPC[numNPCs].Special = 0;
                        NPC[numNPCs].Special2 = A;
                        NPC[numNPCs].Projectile = true;
                        NPC[numNPCs].Direction = NPC[A].Direction;
                        if(NPC[numNPCs].Direction > 0)
                        {
                            NPC[numNPCs].Location.X = NPC[A].Location.X + 32;
                            NPC[numNPCs].Frame = 2;
                        }
                        else
                            NPC[numNPCs].Location.X = NPC[A].Location.X - NPC[numNPCs].Location.Width;
                        NPC[numNPCs].Location.Y = NPC[A].Location.Y;
                        syncLayers_NPC(numNPCs);
                    }
#endif

                    if(NPC[(int)NPC[A].Special2].Type == NPCID_TOOTHY && fEqual(NPC[(int)NPC[A].Special2].Special2, A))
                    {
                        NPC[(int)NPC[A].Special2].Projectile = true;
                        NPC[(int)NPC[A].Special2].Direction = NPC[A].Direction;
                        if(NPC[A].Direction > 0)
                            NPC[(int)NPC[A].Special2].Location.X = NPC[A].Location.X + 32;
                        else
                            NPC[(int)NPC[A].Special2].Location.X = NPC[A].Location.X - NPC[(int)NPC[A].Special2].Location.Width;
                        NPC[(int)NPC[A].Special2].Location.Y = NPC[A].Location.Y;

                        treeNPCUpdate((int)NPC[A].Special2);
                        if(NPC[(int)NPC[A].Special2].tempBlock != 0)
                            treeNPCSplitTempBlock((int)NPC[A].Special2);
                    }

                    if(NPC[A].vehiclePlr > 0 && !Player[NPC[A].vehiclePlr].Controls.Run)
                        NPC[A].Special = 0;
                }
                else if(NPC[A].Type == NPCID_KEY)
                {
                    if(NPC[A].HoldingPlayer > 0)
                    {
                        for(int B : treeBackgroundQuery(NPC[A].Location, SORTMODE_NONE))
                        {
                            if(B > numBackground)
                                continue;

                            if(Background[B].Type == 35)
                            {
                                Location_t tempLocation = Background[B].Location;
                                tempLocation.Width = 16;
                                tempLocation.X += 8;
                                tempLocation.Height = 26;
                                tempLocation.Y += 2;

                                if(CheckCollision(NPC[A].Location, tempLocation))
                                {
                                    PlaySound(SFX_Key);
                                    StopMusic();
                                    LevelMacro = LEVELMACRO_KEYHOLE_EXIT;
                                    LevelMacroWhich = B;
                                    break;
                                }
                            }
                        }
                    }
                }
                else if(NPCIsABot(NPC[A]))
                {
                    if(NPC[A].Projectile || NPC[A].HoldingPlayer > 0)
                    {
                        NPC[A].Special = -1;
                        NPC[A].Special2 = 0;
                        NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * 0.98;
                    }
                    else
                    {
                        if(NPC[A].Special == 0.0)
                        {
                            double C = 0;
                            int D = 1;
                            for(int B = 1; B <= numPlayers; B++)
                            {
                                if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                                {
                                    if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                                    {
                                        C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                        D = B;
                                    }
                                }
                            }
                            if(Player[D].Location.X + Player[D].Location.Width / 2.0 > NPC[A].Location.X + 16)
                                NPC[A].Direction = 1;
                            else
                                NPC[A].Direction = -1;
                        }

                        if(NPC[A].Location.SpeedY == 0.0 || NPC[A].Slope > 0)
                        {
                            NPC[A].Location.SpeedX = 0;
                            if(fiEqual(NPC[A].Special, 0))
                                NPC[A].Special = iRand(3) + 1;
                            if(fiEqual(NPC[A].Special, 1))
                            {
                                NPC[A].FrameCount += 1;
                                NPC[A].Special2 += 1;
                                NPC[A].Location.SpeedX = 0;
                                if(NPC[A].Special2 >= 90)
                                {
                                    NPC[A].Special2 = 0;
                                    NPC[A].Special = -1;
                                    NPC[A].Location.SpeedY = -7;
                                    NPC[A].Location.SpeedX = 2 * NPC[A].Direction;
                                }
                            }
                            else if(NPC[A].Special == 3)
                            {
                                NPC[A].FrameCount += 1;
                                NPC[A].Special2 += 30;
                                NPC[A].Location.SpeedX = 0;
                                if(NPC[A].Special2 >= 30)
                                {
                                    NPC[A].Special2 = 0;
                                    NPC[A].Special = -1;
                                    NPC[A].Location.SpeedY = -3;
                                    NPC[A].Location.SpeedX = 2.5 * NPC[A].Direction;
                                }
                            }
                            else if(NPC[A].Special == 2)
                            {
                                NPC[A].Location.SpeedX = 0.5 * NPC[A].Direction;
                                NPC[A].Special2 += 1;
                                if(NPC[A].Special2 == 120)
                                {
                                    NPC[A].Special2 = 0;
                                    NPC[A].Special = -2;
                                }
                            }
                            else
                            {
                                NPC[A].Special2 += 1;
                                if(NPC[A].Special2 == 30)
                                {
                                    NPC[A].Special2 = 0;
                                    NPC[A].Special = 0;
                                }
                            }
                        }
                    }
                }

                // If FreezeNPCs = True Then
                // .Direction = .DefaultDirection
                // .Special = .DefaultSpecial
                // .Special2 = 0
                // .Special3 = 0
                // .Special4 = 0
                // .Special5 = 0
                // End If


                NPCFrames(A);




                // Effects
            }
            else if(NPC[A].Effect == NPCEFF_EMERGE_UP) // Bonus coming out of a block effect
            {
                if(NPC[A].Direction == 0.0f) // Move toward the closest player
                {
                    double C = 0;
                    for(int B = 1; B <= numPlayers; B++)
                    {
                        if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                        {
                            if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                            {
                                C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                NPC[A].Direction = -Player[B].Direction;
                            }
                        }
                    }
                }
                NPC[A].Frame = EditorNPCFrame(NPC[A].Type, NPC[A].Direction, A);
                NPC[A].Effect2 += 1;
                NPC[A].Location.Y -= 1; // .01
                NPC[A].Location.Height += 1;
                if(NPC[A]->HeightGFX > 0)
                {
                    if(NPC[A].Effect2 >= NPC[A]->HeightGFX)
                    {
                        NPC[A].Effect = NPCEFF_NORMAL;
                        NPC[A].Effect2 = 0;
                        NPC[A].Location.Y += NPC[A].Location.Height;
                        NPC[A].Location.Height = NPC[A]->THeight;
                        NPC[A].Location.Y += -NPC[A].Location.Height;
                    }
                }
                else
                {
                    if(NPC[A].Effect2 >= NPC[A]->THeight)
                    {
                        NPC[A].Effect = NPCEFF_NORMAL;
                        NPC[A].Effect2 = 0;
                        NPC[A].Location.Height = NPC[A]->THeight;
                    }
                }
            }
            else if(NPC[A].Effect == NPCEFF_ENCASED)
            {
                bool tempBool = false;

                // Note: since SMBX64, this logic doesn't check for Hidden or Active, so an encased NPC will not escape encased mode properly in Battle Mode
                // Note 2: NPCID_BOSS_FRAGILE does not use the encased logic, it has its own specific logic to check for nearby NPCID_BOSS_CASE
                for(int B : treeNPCQuery(NPC[A].Location, SORTMODE_NONE))
                {
                    if(NPC[B].Type == NPCID_BOSS_CASE)
                    {
                        if(CheckCollision(NPC[A].Location, NPC[B].Location))
                        {
                            tempBool = true;
                            break;
                        }
                    }
                }

                if(!tempBool)
                    NPC[A].Effect = NPCEFF_NORMAL;
            }
            else if(NPC[A].Effect == NPCEFF_DROP_ITEM) // Bonus item is falling from the players container effect
            {
                NPC[A].Location.Y += 2.2;
                NPC[A].Effect2 += 1;
                if(NPC[A].Effect2 == 5)
                    NPC[A].Effect2 = 1;
            }
            else if(NPC[A].Effect == NPCEFF_EMERGE_DOWN) // Bonus falling out of a block
            {
                if(NPC[A].Type == NPCID_LEAF_POWER)
                    NPC[A].Direction = 1;
                else if(NPC[A].Direction == 0.f) // Move toward the closest player
                {
                    double C = 0;
                    for(int B = 1; B <= numPlayers; B++)
                    {
                        if(!Player[B].Dead && Player[B].Section == NPC[A].Section)
                        {
                            if(C == 0.0 || std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0)) < C)
                            {
                                C = std::abs(NPC[A].Location.X + NPC[A].Location.Width / 2.0 - (Player[B].Location.X + Player[B].Location.Width / 2.0));
                                NPC[A].Direction = -Player[B].Direction;
                            }
                        }
                    }
                }

                NPC[A].Effect2 += 1;
                NPC[A].Location.Y += 1;

                if(fEqual(NPC[A].Effect2, 32.0))
                {
                    NPC[A].Effect = NPCEFF_NORMAL;
                    NPC[A].Effect2 = 0;

                    NPC[A].Location.Height = (g_compatibility.fix_npc_emerge_size) ? NPC[A]->THeight : 32;

                    for(int bCheck = 1; bCheck <= 2; bCheck++)
                    {
                        // if(bCheck == 1)
                        // {
                        //     // fBlock = FirstBlock[(NPC[A].Location.X / 32) - 1];
                        //     // lBlock = LastBlock[((NPC[A].Location.X + NPC[A].Location.Width) / 32.0) + 1];
                        //     blockTileGet(NPC[A].Location, fBlock, lBlock);
                        // }
                        // else
                        // {
                               // buggy, mentioned above, should be numBlock - numTempBlock + 1 -- ds-sloth
                               // it's not a problem here because the NPC is moved out of the way of the block
                               // during the first loop, so can't collide during the second loop.
                        //     fBlock = numBlock - numTempBlock;
                        //     lBlock = numBlock;
                        // }

                        auto collBlockSentinel = (bCheck == 1)
                            ? treeFLBlockQuery(NPC[A].Location, SORTMODE_COMPAT)
                            : treeTempBlockQuery(NPC[A].Location, SORTMODE_LOC);

                        for(BlockRef_t block : collBlockSentinel)
                        {
                            int B = block;

                            if(!Block[B].Invis && !(BlockIsSizable[Block[B].Type] && NPC[A].Location.Y > Block[B].Location.Y) && !Block[B].Hidden)
                            {
                                if(CheckCollision(NPC[A].Location, Block[B].Location))
                                {
                                    NPC[A].Location.Y = Block[B].Location.Y - NPC[A].Location.Height - 0.1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else if(NPC[A].Effect == NPCEFF_WARP) // Warp Generator
            {
                if(NPC[A].Effect3 == 1)
                {
                    NPC[A].Location.Y -= 1;
                    if(NPC[A].Type == NPCID_PLATFORM_S1)
                        NPC[A].Location.Y -= 1;

                    if(NPC[A].Location.Y + NPC[A].Location.Height <= NPC[A].Effect2)
                    {
                        NPC[A].Effect = NPCEFF_NORMAL;
                        NPC[A].Effect2 = 0;
                        NPC[A].Effect3 = 0;
                    }
                }
                else if(NPC[A].Effect3 == 3)
                {
                    NPC[A].Location.Y += 1;

                    if(NPC[A].Type == NPCID_PLATFORM_S1)
                        NPC[A].Location.Y += 1;

                    if(NPC[A].Location.Y >= NPC[A].Effect2)
                    {
                        NPC[A].Effect = NPCEFF_NORMAL;
                        NPC[A].Effect2 = 0;
                        NPC[A].Effect3 = 0;
                    }
                }
                else if(NPC[A].Effect3 == 2)
                {
                    if(NPC[A].Type == NPCID_POWER_S3 || NPC[A].Type == NPCID_LIFE_S3 || NPC[A].Type == NPCID_POISON || NPC[A].Type == NPCID_POWER_S1 || NPC[A].Type == NPCID_POWER_S4 || NPC[A].Type == NPCID_LIFE_S1 || NPC[A].Type == NPCID_LIFE_S4 || NPC[A].Type == NPCID_BRUTE_SQUISHED || NPC[A].Type == NPCID_BIG_MOLE)
                        NPC[A].Location.X -= double(Physics.NPCMushroomSpeed);
                    else if(NPC[A]->CanWalkOn)
                        NPC[A].Location.X -= 1;
                    else
                        NPC[A].Location.X -= double(Physics.NPCWalkingSpeed);

                    if(NPC[A].Location.X + NPC[A].Location.Width <= NPC[A].Effect2)
                    {
                        NPC[A].Effect = NPCEFF_NORMAL;
                        NPC[A].Effect2 = 0;
                        NPC[A].Effect3 = 0;
                    }
                }
                else if(NPC[A].Effect3 == 4)
                {
                    if(NPC[A].Type == NPCID_POWER_S3 || NPC[A].Type == NPCID_LIFE_S3 || NPC[A].Type == NPCID_POISON || NPC[A].Type == NPCID_POWER_S1 || NPC[A].Type == NPCID_POWER_S4 || NPC[A].Type == NPCID_LIFE_S1 || NPC[A].Type == NPCID_LIFE_S4 || NPC[A].Type == NPCID_BRUTE_SQUISHED || NPC[A].Type == NPCID_BIG_MOLE)
                        NPC[A].Location.X += double(Physics.NPCMushroomSpeed);
                    else if(NPC[A]->CanWalkOn)
                        NPC[A].Location.X += 1;
                    else
                        NPC[A].Location.X += double(Physics.NPCWalkingSpeed);

                    if(NPC[A].Location.X >= NPC[A].Effect2)
                    {
                        NPC[A].Effect = NPCEFF_NORMAL;
                        NPC[A].Effect2 = 0;
                        NPC[A].Effect3 = 0;
                    }
                }

                NPCFrames(A);

                if(NPC[A].Effect == 0 && NPC[A].Type != NPCID_ITEM_BURIED)
                {
                    NPC[A].Layer = LAYER_SPAWNED_NPCS;
                    syncLayers_NPC(A);
                }
            }
            else if(NPC[A].Effect == NPCEFF_PET_TONGUE) // Grabbed by Yoshi
            {
                NPC[A].TimeLeft = 100;
                NPC[A].Effect3 -= 1;
                if(NPC[A].Effect3 <= 0)
                {
                    NPC[A].Effect = NPCEFF_NORMAL;
                    NPC[A].Effect2 = 0;
                    NPC[A].Effect3 = 0;
                }
            }
            else if(NPC[A].Effect == NPCEFF_PET_INSIDE) // Held by Yoshi
            {
                NPC[A].TimeLeft = 100;
                if(Player[NPC[A].Effect2].YoshiNPC != A)
                {
                    NPC[A].Effect = NPCEFF_NORMAL;
                    NPC[A].Effect2 = 0;
                    NPC[A].Effect3 = 0;
                }
            }
            else if(NPC[A].Effect == NPCEFF_WAITING) // Holding Pattern
            {
                NPC[A].Effect2 -= 1;
                if(NPC[A].Effect2 <= 0)
                {
                    NPC[A].Effect = NPCEFF_NORMAL;
                    NPC[A].Effect2 = 0;
                    NPC[A].Effect3 = 0;
                }
            }

            // Originally applied for all NPCs, even if inactive.
            // Moved here because speedVar is only validly set here.
            if(!fEqual(speedVar, 1) && !fEqual(speedVar, 0))
            {
                NPC[A].RealSpeedX = float(NPC[A].Location.SpeedX);
                NPC[A].Location.SpeedX = NPC[A].Location.SpeedX * double(speedVar);
            }

            // finally update NPC's tree status if needed, and split the tempBlock (since it has not been updated)
            if(NPC[A].Location.X != prevX
                || NPC[A].Location.Y != prevY
                || NPC[A].Location.Width != prevW
                || NPC[A].Location.Height != prevH)
            {
                treeNPCUpdate(A);

                if(NPC[A].tempBlock > 0)
                    treeNPCSplitTempBlock(A);
            }
        }

        if(NPC[A].AttLayer != LAYER_NONE && NPC[A].AttLayer != LAYER_DEFAULT && NPC[A].HoldingPlayer == 0)
        {
            int B = NPC[A].AttLayer;
            // for(int B = 1; B <= maxLayers; B++)
            {
                // if(Layer[B].Name != "")
                {
                    // if(Layer[B].Name == NPC[A].AttLayer)
                    {
                        if(NPC[A].Location.X - lyrX == 0.0 && NPC[A].Location.Y - lyrY == 0.0)
                        {
                            if(Layer[B].SpeedX != 0.0f || Layer[B].SpeedY != 0.0f)
                            {
                                Layer[B].EffectStop = true;
                                Layer[B].SpeedX = 0;
                                Layer[B].SpeedY = 0;

                                for(int C : Layer[B].blocks)
                                // for(int C = 1; C <= numBlock; C++)
                                {
                                    // if(Block[C].Layer == Layer[B].Name)
                                    {
                                        Block[C].Location.SpeedX = double(Layer[B].SpeedX);
                                        Block[C].Location.SpeedY = double(Layer[B].SpeedY);
                                    }
                                }

                                for(int C : Layer[B].NPCs)
                                {
                                    // if(NPC[C].Layer == Layer[B].Name)
                                    {
                                        if(NPC[C]->IsAVine || NPC[C].Type == NPCID_ITEM_BURIED)
                                        {
                                            NPC[C].Location.SpeedX = 0;
                                            NPC[C].Location.SpeedY = 0;
                                        }
                                    }
                                }

                                // @Wohlstand, should we add something here for the BGO fence fix?
                            }
                        }
                        else
                        {
                            Layer[B].EffectStop = false;
                            Layer[B].SpeedX = float(NPC[A].Location.X - lyrX);
                            Layer[B].SpeedY = float(NPC[A].Location.Y - lyrY);
                        }
                    }
                }
                // else
                //     break;
            }
        }

    }

    numBlock -= numTempBlock; // clean up the temp npc blocks

    treeTempBlockClear();


    // kill the NPCs, from last to first
    std::sort(NPCQueues::Killed.begin(), NPCQueues::Killed.end(),
    [](NPCRef_t a, NPCRef_t b)
    {
        return a > b;
    });

    int last_NPC = maxNPCs + 1;
    size_t KilledQueue_check = NPCQueues::Killed.size();
    size_t KilledQueue_known = NPCQueues::Killed.size();

    for(size_t i = 0; i < KilledQueue_check; i++) // KILL THE NPCS <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><
    {
        int A = NPCQueues::Killed[i];

        // duplicated entry, no problem
        if(A == last_NPC)
            continue;

        SDL_assert(A < last_NPC); // something's wrong in the sort order

        if(NPC[A].Killed > 0)
        {
            if(NPC[A].Location.SpeedX == 0.0)
            {
                NPC[A].Location.SpeedX = dRand() * 2 - 1;
                if(NPC[A].Location.SpeedX < 0)
                    NPC[A].Location.SpeedX -= 0.5;
                else
                    NPC[A].Location.SpeedX += 0.5;
            }

            KillNPC(A, NPC[A].Killed);

            // KillNPC sometimes adds duplicate / unnecessary members to NPCQueues::Killed
            bool real_new_killed = false;
            if(NPCQueues::Killed.size() > KilledQueue_known)
            {
                for(size_t j = NPCQueues::Killed.size() - 1; j >= KilledQueue_known; j--)
                {
                    int K = NPCQueues::Killed[j];
                    if(K != A && K >= 1 && K <= numNPCs)
                    {
                        real_new_killed = true;
                        break;
                    }
                }

                // ignore if no real new NPCs added
                if(!real_new_killed)
                    NPCQueues::Killed.resize(KilledQueue_known);
                else
                {
                    pLogDebug("During KillNPC(%d), %d actual new killed NPC indexes added.", A, (int)(NPCQueues::Killed.size() - KilledQueue_known));
                    KilledQueue_known = NPCQueues::Killed.size();
                }
            }

            // rare cases exist where a real new NPC is killed (mostly events that hide layers)
            // sort and check the ones smaller than A
            if(real_new_killed)
            {
                size_t old_check = KilledQueue_check;

                // partition to check all the ones < A this frame
                auto first_bigger_it = std::partition(NPCQueues::Killed.begin() + KilledQueue_check, NPCQueues::Killed.end(),
                [A](NPCRef_t a)
                {
                    return (int)a < A;
                });

                KilledQueue_check = first_bigger_it - NPCQueues::Killed.begin();

                if(old_check != KilledQueue_check)
                {
                    pLogDebug("Found %d indexes lower than %d. Sorting to check this frame.", (int)(first_bigger_it - NPCQueues::Killed.begin()) - (int)old_check, A);

                    // re-sort the range to check this frame
                    std::sort(NPCQueues::Killed.begin() + i + 1, NPCQueues::Killed.begin() + KilledQueue_check,
                    [](NPCRef_t a, NPCRef_t b)
                    {
                        return a > b;
                    });
                }
            }
        }
    }

    if(NPCQueues::Killed.size() > KilledQueue_check)
    {
        NPCQueues::Killed.erase(NPCQueues::Killed.begin(), NPCQueues::Killed.begin() + KilledQueue_check);
        pLogDebug("Checking %d newly killed NPC indexes next frame.", (int)NPCQueues::Killed.size());
    }
    else
        NPCQueues::Killed.clear();

    //    if(nPlay.Online == true)
    //    {
    //        if(nPlay.Mode == 1)
    //        {
    //            nPlay.NPCWaitCount += 10;
    //            if(nPlay.NPCWaitCount >= 5)
    //            {
    //                tempStr = "L" + LB;
    //                for(int A = 1; A <= numNPCs; A++)
    //                {
    //                    if(NPC[A].Active == true && NPC[A].TimeLeft > 1)
    //                    {
    //                        if(NPC[A].HoldingPlayer <= 1)
    //                        {
    //                            tempStr += "K" + std::to_string(A) + "|" + NPC[A].Type + "|" + NPC[A].Location.X + "|" + NPC[A].Location.Y + "|" + std::to_string(NPC[A].Location.Width) + "|" + std::to_string(NPC[A].Location.Height) + "|" + NPC[A].Location.SpeedX + "|" + NPC[A].Location.SpeedY + "|" + NPC[A].Section + "|" + NPC[A].TimeLeft + "|" + NPC[A].Direction + "|" + std::to_string(static_cast<int>(floor(static_cast<double>(NPC[A].Projectile)))) + "|" + NPC[A].Special + "|" + NPC[A].Special2 + "|" + NPC[A].Special3 + "|" + NPC[A].Special4 + "|" + NPC[A].Special5 + "|" + NPC[A].Effect + LB;
    //                            if(NPC[A].Effect != 0)
    //                                tempStr += "2c" + std::to_string(A) + "|" + NPC[A].Effect2 + "|" + NPC[A].Effect3 + LB;
    //                        }
    //                    }
    //                }
    //                Netplay::sendData tempStr + "O" + std::to_string(numPlayers) + LB;
    //                nPlay.NPCWaitCount = 0;
    //            }
    //        }
    //    }
    CharStuff();
}
