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

#include "sdl_proxy/sdl_stdinc.h"

#include "../globals.h"
#include "../npc.h"
#include "player.h"
#include "../sound.h"
#include "../graphics.h"
#include "../collision.h"
#include "../effect.h"
#include "../layers.h"
#include "../editor.h"
#include "../blocks.h"
#include "../sorting.h"
#include "../config.h"
#include "../main/trees.h"
#include "../npc_id.h"
#include "../eff_id.h"
#include "../layers.h"

#include "game_main.h"
#include "npc_traits.h"

#include "npc/npc_queues.h"
#include "npc/section_overlap.h"
#include "npc/npc_update/npc_update_priv.h"

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
                if(g_config.fix_FreezeNPCs_no_reset)
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
        if(!g_config.modern_lives_system && Lives >= 99 && Coins >= 99)
            CoinMode = false;
        else
        {
            PlaySound(SFX_Coin);
            Coins += 1;
            if(Coins >= 100)
            {
                if(g_config.modern_lives_system)
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
                                // NOTE: this code previously used Effect2 to store the destination position, and now it uses SpecialX/Y
                                NPC[numNPCs].Layer = NPC[A].Layer;
                                NPC[numNPCs].Effect3 = NPC[A].GeneratorDirection;
                                NPC[numNPCs].Effect2 = 0;
                                NPC[numNPCs].Effect = NPCEFF_WARP;
                                NPC[numNPCs].Location.SpeedX = 0;
                                NPC[numNPCs].TimeLeft = 100;
                                if(NPC[A].GeneratorDirection == 1)
                                {
                                    if(NPC[A]->HeightGFX > NPC[A].Location.Height)
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A]->HeightGFX;
                                        NPC[numNPCs].SpecialY = NPC[numNPCs].Location.Y - (NPC[A]->HeightGFX - NPC[A].Location.Height);
                                    }
                                    else
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y + NPC[A].Location.Height;
                                        NPC[numNPCs].SpecialY = NPC[numNPCs].Location.Y;
                                    }
                                }
                                else if(NPC[A].GeneratorDirection == 3)
                                {
                                    if(NPC[A]->HeightGFX > NPC[A].Location.Height)
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                                        NPC[numNPCs].SpecialY = NPC[numNPCs].Location.Y + NPC[A].Location.Height + (NPC[A]->HeightGFX - NPC[A].Location.Height);
                                    }
                                    else
                                    {
                                        NPC[numNPCs].Location.Y = NPC[A].Location.Y - NPC[A].Location.Height;
                                        NPC[numNPCs].SpecialY = NPC[numNPCs].Location.Y + NPC[A].Location.Height;
                                    }
                                }
                                else if(NPC[A].GeneratorDirection == 2)
                                {
                                    NPC[numNPCs].Location.Y -= 4;
                                    NPC[numNPCs].Location.X = NPC[A].Location.X + NPC[A].Location.Width;
                                    NPC[numNPCs].SpecialX = NPC[numNPCs].Location.X;
                                }
                                else if(NPC[A].GeneratorDirection == 4)
                                {
                                    NPC[numNPCs].Location.Y -= 4;
                                    NPC[numNPCs].Location.X = NPC[A].Location.X - NPC[A].Location.Width;
                                    NPC[numNPCs].SpecialX = NPC[numNPCs].Location.X + NPC[A].Location.Width;
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
                                    SoundPause[SFX_BlockHit] = 1;
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
                NPC[A].TimeLeft = 100;
                NPC[A].Active = true;
                NPC[A].JustActivated = 0;
            }
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
                    // In SMBX 1.3, Deactivate was called every frame for every Hidden NPC (in this loop over A). That set Reset to false. Now we need to emulate it.
                    bool reset_should_be_false = (B < A && NPC[B].Hidden);

                    if(!NPC[B].Active && B != A && !reset_should_be_false && NPC[B].Reset[1] && NPC[B].Reset[2])
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location))
                        {
                            SDL_assert_release(numAct < maxNPCs);
                            newAct[numAct] = B;
                            numAct++;

                            NPC[B].Active = true;
                            NPC[B].TimeLeft = NPC[A].TimeLeft;
                            NPC[B].Section = NPC[A].Section;

                            if(g_config.fix_npc_camera_logic)
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
                            // In SMBX 1.3, Deactivate was called every frame for every Hidden NPC (in this loop over A). That set Reset to false. Now we need to emulate it.
                            bool reset_should_be_false = (B < A && NPC[B].Hidden);

                            if(!NPC[B].Active &&
                                !reset_should_be_false &&
                                (!NPC[B].Hidden || !g_config.fix_npc_activation_event_loop_bug) &&
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

                                    if(g_config.fix_npc_camera_logic)
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
                    if(NPC[B].Type != NPCID_BOSS_CASE && NPC[B].Effect == NPCEFF_NORMAL && NPC[B].Active)
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
                        NPC[A].Effect == NPCEFF_NORMAL && !(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 1) &&
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
            if(!g_config.emulate_classic_block_order)
                treeTempBlockAdd(numBlock);

            numTempBlock++;
        }
    }

    // need to sort the temp blocks in strict compatibility mode, to fully emulate the specific way that switched block clipping works in X64
    if(g_config.emulate_classic_block_order)
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
        if(NPC[A].JustActivated == 0 && !(NPC[A]->IsFish && NPC[A].Special == 2) && NPC[A].Type != NPCID_LAVABUBBLE)
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
                        // save location of ground below goal tape (was previously Special2)
                        NPC[A].SpecialY = Block[C].Location.Y + 4;
                        NPC[A].Location.Y = Block[C].Location.Y - NPC[A].Location.Height;
                        NPC[A].Special = 1;

                        treeNPCUpdate(A);
                        if(NPC[A].tempBlock > 0)
                            treeNPCSplitTempBlock(A);
                    }
                }
                else if(NPC[A].Type == NPCID_LAVA_MONSTER) // blaarg
                {
                    NPC[A].Location.Y = NPC[A].DefaultLocationY + NPC[A].Location.Height + 36;
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
                    else if(NPC[A]->IsFish && NPC[A].Special == 2)
                    {
                        NPC[A].Location.Y = level[Player[NPC[A].JustActivated].Section].Height - 0.1;
                        NPC[A].Location.SpeedX = (1 + (NPC[A].Location.Y - NPC[A].DefaultLocationY) * 0.005) * NPC[A].Direction;
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
            if(!GameMenu && !(NPC[A]->IsFish && NPC[A].Special == 2) && NPC[A].Type != NPCID_LAVABUBBLE)
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
                                if(NPC[A].Location.SpeedY >= 1 && (!g_config.fix_submerged_splash_effect || !CheckCollisionIntersect(NPC[A].Location, static_cast<Location_t>(Water[B].Location))))
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

                                // assigned to Special in SMBX 1.3
                                if(NPC[A].Type == NPCID_PLATFORM_S3)
                                    NPC[A].SpecialY = NPC[A].Location.SpeedY;
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

            if(!(NPC[A].Type == NPCID_PLR_FIREBALL || (NPC[A]->IsFish && NPC[A].Special == 2) ||
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
                            if(NPC[A].Special == 0)
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

                        // dead code, impossible condition in SMBX 1.3 source (IsFish did not apply for NPC Type 1)
                        // if(NPC[A].Type == (NPC[A]->IsFish && NPC[A].Special == 2))
                        //     NPC[A].Special5 = 0;

                        NPC[A].Direction = Player[NPC[A].HoldingPlayer].Direction; // Face the player
                        NPC[A].Location.SpeedY = Player[NPC[A].HoldingPlayer].Location.SpeedY;
                        NPC[A].Location.SpeedX = 0;

                        NPCCollideHeld(A);

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

                        // was Special2
                        if(NPC[A].Type == NPCID_SAW)
                            NPC[A].SpecialX = -NPC[A].SpecialX;

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

                    NPCMovementLogic(A, speedVar);

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

                    NPC[A].onWall = false;
                    if(NPC[A].Location.X < -(FLBlocks - 1) * 32)
                        NPC[A].Location.X = -(FLBlocks - 1) * 32;
                    if(NPC[A].Location.X + NPC[A].Location.Width > (FLBlocks + 1) * 32)
                        NPC[A].Location.X = (FLBlocks + 1) * 32 - NPC[A].Location.Width;

                    if(!(NPC[A]->IsACoin && NPC[A].Special == 0) && !(NPC[A].Type == NPCID_SLIDE_BLOCK && NPC[A].Special == 0) &&
                       NPC[A].Type != NPCID_CONVEYOR && NPC[A].Type != NPCID_STATUE_FIRE && NPC[A].Type != NPCID_ITEM_BURIED && NPC[A].Type != NPCID_STAR_EXIT &&
                       NPC[A].Type != NPCID_STAR_COLLECT && !(NPC[A].Type >= NPCID_PLATFORM_S3 && NPC[A].Type <= NPCID_PLATFORM_S1) &&
                       !(NPCIsAnExit(NPC[A]) && ((NPC[A].DefaultLocationX == NPC[A].Location.X &&
                       NPC[A].DefaultLocationY == NPC[A].Location.Y) || NPC[A].Inert)) &&
                       NPC[A].Type != NPCID_LIFT_SAND && NPC[A].Type != NPCID_CHECKPOINT && NPC[A].Type != NPCID_SICK_BOSS_BALL &&
                       !(NPC[A].Type == NPCID_PLANT_FIRE || NPC[A].Type == NPCID_LOCK_DOOR || NPC[A].Type == NPCID_FIRE_DISK || NPC[A].Type == NPCID_FIRE_CHAIN))
                    {
                        double tempHit = 0; // height of block NPC is walking on
                        int tempHitBlock = 0; // index of block NPC is walking on
                        float tempSpeedA = 0; // speed of ground the NPC is possibly standing on

                        NPCBlockLogic(A, tempHit, tempHitBlock, tempSpeedA, numTempBlock, speedVar);


                        // End Block Collision
                        // possible usually-nulled function pointer for logic between block and NPC collisions

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
                        NPCCollide(A);

                        if(NPC[A].WallDeath > 0)
                        {
                            if(NPC[A]->IsFish)
                                NPC[A].WallDeath -= 1;
                            else
                                NPC[A].WallDeath = 0;
                        }

                        if(tempHit != 0) // Walking   // VERIFY ME
                            NPCWalkingLogic(A, tempHit, tempHitBlock, tempSpeedA);
                    }
                    else
                    {
                        NPC[A].BeltSpeed = 0;
                        NPC[A].Slope = 0;
                    }
                }

                if(NPC[A].tempBlock > 0 && (NPC[A].Type < NPCID_YEL_PLATFORM || NPC[A].Type > NPCID_RED_PLATFORM) && NPC[A].Type != NPCID_CONVEYOR)
                {
                    if((NPC[A].Type < NPCID_TANK_TREADS || NPC[A].Type > NPCID_SLANT_WOOD_M) && NPC[A].Type != NPCID_SPRING)
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
                NPCSpecialMaybeHeld(A);


                // If FreezeNPCs = True Then
                // .Direction = .DefaultDirection
                // .Special = .DefaultSpecial
                // .Special2 = 0
                // .Special3 = 0
                // .Special4 = 0
                // .Special5 = 0
                // End If


                NPCFrames(A);
            }
            // Effects
            else
                NPCEffects(A);

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
            SetLayerSpeed(NPC[A].AttLayer, NPC[A].Location.X - lyrX, NPC[A].Location.Y - lyrY, false);
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

        // in rare cases, the game may accidentally kill an NPC outside of the NPC array (storage glitch example)
        if(A > numNPCs)
            continue;

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
    //                            if(NPC[A].Effect != NPCEFF_NORMAL)
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
