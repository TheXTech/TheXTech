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

#include <AppPath/app_path.h>
#include <Utils/files.h>
#include <Integrator/integrator.h>
#include <PGE_File_Formats/file_formats.h>
#include <fmt_format_ne.h>

#include "../globals.h"
#include "../game_main.h"
#include "../sound.h"
#include "../effect.h"
#include "../graphics.h"
#include "../blocks.h"
#include "../npc.h"
#include "../eff_id.h"
#include "../layers.h"
#include "../player.h"
#include "../collision.h"
#include "../controls.h"
#include "menu_main.h"
#include "game_info.h"
#include "game_globals.h"
#include "menu_controls.h"

#include "npc_traits.h"

#include "main/trees.h"
#include "script/luna/luna.h"

#include "npc/npc_queues.h"


static void updateIntroLevelActivity()
{
    Location_t tempLocation;
    bool tempBool;

    SingleCoop = 0;

    // only restore the level on the first frame that all players are dead
    static bool restore_done = false;

    bool any_living = CheckLiving();
    if(!any_living && !restore_done)
    {
        ShowLayer(LAYER_DESTROYED_BLOCKS);

        For(A, 1, numNPCs)
        {
            if(NPC[A].DefaultType == 0)
            {
                if(NPC[A].TimeLeft > 10) NPC[A].TimeLeft = 10;
            }
        }

        restore_done = true;
    }
    else if(any_living)
    {
        restore_done = false;
    }

    For(A, 1, numPlayers)
    {
        Player_t &p = Player[A];
        if(p.TimeToLive > 0)
        {
            p.TimeToLive = 0;
            p.Dead = true;
        }

        p.Controls.Down = false;
        p.Controls.Drop = false;
        p.Controls.Right = true;
        p.Controls.Left = false;
        p.Controls.Run = true;
        p.Controls.Up = false;
        p.Controls.AltRun = false;
        p.Controls.AltJump = false;

        if(p.Jump == 0 || p.Location.Y < level[0].Y + 200)
            p.Controls.Jump = false;

        if(p.Location.SpeedX < 0.5)
        {
            p.Controls.Jump = true;
            if(p.Slope > 0 || p.StandingOnNPC > 0 || p.Location.SpeedY == 0.0)
                p.CanJump = true;
        }

        if(p.HoldingNPC ==0)
        {
            if((p.State == 3 || p.State == 6 || p.State == 7) && iRand(100) >= 90)
            {
                if(p.FireBallCD == 0 && !p.RunRelease)
                    p.Controls.Run = false;
            }

            if((p.State == 4 || p.State == 5) && p.TailCount == 0 && !p.RunRelease)
            {
                tempLocation.Width = 24;
                tempLocation.Height = 20;
                tempLocation.Y = p.Location.Y + p.Location.Height - 22;
                tempLocation.X = p.Location.X + p.Location.Width;

                for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
                {
                    if(NPC[B].Active && !NPC[B]->IsABonus &&
                       !NPC[B]->WontHurt && NPC[B].HoldingPlayer == 0)
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location))
                        {
                            p.Controls.Run = false;
                            break;
                        }
                    }
                }
            }

            if(p.StandingOnNPC > 0)
            {
                if(NPC[p.StandingOnNPC]->GrabFromTop)
                {
                    p.Controls.Down = true;
                    p.Controls.Run = true;
                    p.RunRelease = true;
                }
            }
        }

        if(p.Character == 5)
        {
            if(p.FireBallCD == 0 && !p.RunRelease)
            {
                tempLocation.Width = 38 + p.Location.SpeedX * 0.5;
                tempLocation.Height = p.Location.Height - 8;
                tempLocation.Y = p.Location.Y + 4;
                tempLocation.X = p.Location.X + p.Location.Width;

                for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
                {
                    if(NPC[B].Active && !NPC[B]->IsABonus &&
                      !NPC[B]->WontHurt && NPC[B].HoldingPlayer == 0)
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location))
                        {
                            p.RunRelease = true;
                            if(NPC[B].Location.Y > p.Location.Y + p.Location.Height / 2)
                                p.Controls.Down = true;
                            break;
                        }
                    }
                }
            }

            if(p.Slope == 0 && p.StandingOnNPC == 0)
            {
                if(p.Location.SpeedY < 0)
                {
                    tempLocation.Width = 200;
                    tempLocation.Height = p.Location.Y - level[0].Y + p.Location.Height;
                    tempLocation.Y = level[0].Y;
                    tempLocation.X = p.Location.X;

                    for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
                    {
                        if(NPC[B].Active && !NPC[B]->IsABonus &&
                           !NPC[B]->WontHurt && NPC[B].HoldingPlayer == 0)
                        {
                            if(CheckCollision(tempLocation, NPC[B].Location))
                            {
                                p.Controls.Up = true;
                                break;
                            }
                        }
                    }

                }
                else if(p.Location.SpeedY > 0)
                {
                    tempLocation.Width = 200;
                    tempLocation.Height = level[0].Height - p.Location.Y;
                    tempLocation.Y = p.Location.Y;
                    tempLocation.X = p.Location.X;

                    for(int B : treeNPCQuery(tempLocation, SORTMODE_NONE))
                    {
                        if(NPC[B].Active && !NPC[B]->IsABonus &&
                           !NPC[B]->WontHurt && NPC[B].HoldingPlayer == 0)
                        {
                            if(CheckCollision(tempLocation, NPC[B].Location))
                            {
                                p.Controls.Down = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if(p.Location.X < -vScreen[1].X - p.Location.Width && -vScreen[1].X > level[0].X)
            p.Dead = true;

        if(p.Location.X > -vScreen[1].X + vScreen[1].Width + 200)
            p.Dead = true;

        if(p.Location.X > -vScreen[1].X + vScreen[1].Width * 0.75 && -vScreen[1].X + vScreen[1].Width + 50 < level[0].Width)
            p.Controls.Run = false;

        if(-vScreen[1].X <= level[0].X && (p.Dead || p.TimeToLive > 0) && g_gameInfo.introMaxPlayersCount > 0)
        {
            p.ForceHold = 65;
            p.State = iRand(6) + 2;
            p.CanFly = false;
            p.CanFly2 = false;
            p.TailCount = 0;
            p.Dead = false;
            p.TimeToLive = 0;
            iRand(1); // advance the random state by one
//            p.Character = (iRand() % 5) + 1;

//            if(A >= 1 && A <= 5)
            p.Character = g_gameInfo.introCharacterNext();

            p.HeldBonus = NPCID(0);
            p.Section = 0;
            p.Mount = 0;
            p.MountType = 0;
            p.YoshiBlue = false;
            p.YoshiRed = false;
            p.YoshiYellow = false;
            p.YoshiNPC = 0;
            p.Wet = 0;
            p.WetFrame = false;
            p.YoshiPlayer = 0;
            p.Bumped = false;
            p.Bumped2 = 0;
            p.Direction = 1;
            p.Dismount = 0;
            p.Effect = 0;
            p.Effect2 = 0;
            p.FireBallCD = 0;
            p.ForceHold = 0;
            p.Warp = 0;
            p.WarpBackward = false;
            p.WarpCD = 0;
            p.GroundPound = false;
            p.Immune = 0;
            p.Frame = 0;
            p.Slope = 0;
            p.Slide = false;
            p.SpinJump = false;
            p.FrameCount = 0;
            p.TailCount = 0;
            p.Duck = false;
            p.GroundPound = false;
            p.Hearts = 3;

            PlayerFrame(p);

            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            p.Location.X = level[p.Section].X - A * 48;
            p.Location.SpeedX = double(Physics.PlayerRunSpeed);
            p.Location.Y = level[p.Section].Height - p.Location.Height - 33;

            do
            {
                tempBool = true;
                for(int B : treeBlockQuery(p.Location, SORTMODE_NONE))
                {
                    if(CheckCollision(p.Location, Block[B].Location))
                    {
                        p.Location.Y = Block[B].Location.Y - p.Location.Height - 0.1;
                        tempBool = false;
                        break;
                    }
                }
            } while(!tempBool);

            if(!UnderWater[p.Section])
            {
                if(iRand(25) == 0)
                {
                    p.Mount = 1;
                    p.MountType = iRand(3) + 1;
                    if(p.State == 1)
                    {
                        p.Location.Height = Physics.PlayerHeight[1][2];
                        p.Location.Y += -Physics.PlayerHeight[1][2] + Physics.PlayerHeight[p.Character][1];
                    }
                }
            }

            if(p.Mount == 0 && p.Character <= 2)
            {
                if(iRand(15) == 0)
                {
                    p.Mount = 3;
                    p.MountType = iRand(7) + 1;
                    p.Location.Y += p.Location.Height;
                    p.Location.Height = Physics.PlayerHeight[2][2];
                    p.Location.Y += -p.Location.Height - 0.01;
                }
            }

            p.CanFly = false;
            p.CanFly2 = false;
            p.RunCount = 0;

            if(p.Mount == 0 && p.Character != 5)
            {
                numNPCs++;
                p.HoldingNPC = numNPCs;
                p.ForceHold = 120;

                {
                    NPC_t &n = NPC[numNPCs];
                    do
                    {
                        do
                        {
                            n.Type = NPCID(iRand(286) + 1);
                        } while(n.Type == 11 || n.Type == 16 || n.Type == 18 || n.Type == 15 ||
                                n.Type == 21 || n.Type == 12 || n.Type == 13 || n.Type == 30 ||
                                n.Type == 17 || n.Type == 31 || n.Type == 32 ||
                                (n.Type >= 37 && n.Type <= 44) || n.Type == 46 || n.Type == 47 ||
                                n.Type == 50 || (n.Type >= 56 && n.Type <= 70) || n.Type == 8 ||
                                n.Type == 74 || n.Type == 51 || n.Type == 52 || n.Type == 75 ||
                                n.Type == 34 || NPCIsToad(n) || NPCIsAnExit(n) ||
                                NPCIsYoshi(n) || (n.Type >= 78 && n.Type <= 87) ||
                                n.Type == 91 || n.Type == 93 || (n.Type >= 104 && n.Type <= 108) ||
                                n.Type == 125 || n.Type == 133 || (n.Type >= 148 && n.Type <= 151) ||
                                n.Type == 159 || n.Type == 160 || n.Type == 164 || n.Type == 168 ||
                                // n.Type == 159 || n.Type == 160 || n.Type == 164 || // Duplicated segment [PVS Studio]
                                (n.Type >= 154 && n.Type <= 157) ||
                                n.Type == 165 || n.Type == 171 || n.Type == 178 ||
                                n.Type == 197 || n.Type == 180 || n.Type == 181 || n.Type == 190 ||
                                n.Type == 192 || n.Type == 196 ||
                                // Duplicated segment, .....! [PVS Studio]
                                // n.Type == 197 ||
                                (UnderWater[0] && NPCIsBoot(n)) ||
                                (n.Type >= 198 && n.Type <= 228) || n.Type == 234);

                    } while(n.Type == 235 || n.Type == 231 || n.Type == 179 || n.Type == 49 ||
                            n.Type == 237 || n.Type == 238 || n.Type == 239 || n.Type == 240 ||
                            n.Type == 245 || n.Type == 246 || n.Type == 248 || n.Type == 254 ||
                            n.Type == 255 || n.Type == 256 || n.Type == 257 || n.Type == 259 ||
                            n.Type == 260 || n.Type == 262 || n.Type == 263 || n.Type == 265 ||
                            n.Type == 266 || (n.Type >= 267 && n.Type <= 272) ||
                            n.Type == 275 || n.Type == 276 ||
                            (n.Type >= 280 && n.Type <= 284) || n.Type == 241);

                    n.Active = true;
                    n.HoldingPlayer = A;
                    n.Location.Height = n->THeight;
                    n.Location.Width = n->TWidth;
                    n.Location.Y = Player[A].Location.Y;  // level[n.Section].Height + 1000
                    n.Location.X = Player[A].Location.X; // level[n.Section].X + 1000
                    n.TimeLeft = 100;
                    n.Section = Player[A].Section;
                    syncLayers_NPC(numNPCs);
                }
            }

        }
        else if(p.Location.X > level[p.Section].Width + 64)
        {
            p.Dead = true;
        }

        if(p.WetFrame)
        {
            if(p.Location.SpeedY == 0.0 || p.Slope > 0)
                p.CanJump = true;
            if(iRand(100) >= 98 || p.Location.SpeedY == 0.0 || p.Slope > 0)
                p.Controls.Jump = true;
        }

        if(iRand(20) == 0 && Player[A].HoldingNPC == 0 && !Player[A].Slide && Player[A].CanAltJump && Player[A].Mount == 0)
            Player[A].Controls.AltJump = true;

        if(iRand(1000) == 0 && !Player[A].CanFly2)
            Player[A].Controls.Run = false;

        if(Player[A].Mount == 3 && iRand(50) == 0 && !Player[A].RunRelease)
            Player[A].Controls.Run = false;

        if(NPC[Player[A].HoldingNPC].Type == 22 || NPC[Player[A].HoldingNPC].Type == 49)
            Player[A].Controls.Run = true;

        if(Player[A].Slide && Player[A].CanJump)
        {
            if(Player[A].Location.SpeedX > -2 && Player[A].Location.SpeedX < 2)
                Player[A].Controls.Jump = true;
        }

        if(!Player[A].CanFly && !Player[A].CanFly2 && (Player[A].State == 4 || Player[A].State == 5) && !Player[A].Slide)
            Player[A].Controls.Jump = true;

        if(Player[A].Quicksand > 0)
        {
            Player[A].CanJump = true;
            Player[A].Controls.Jump = true;
        }

        if(Player[A].FloatTime > 0 || (Player[A].CanFloat && Player[A].FloatRelease && Player[A].Jump == 0 && Player[A].Location.SpeedY > 0 && iRand(20) == 0))
            Player[A].Controls.Jump = true;

        if(NPC[Player[A].HoldingNPC].Type == 13 && iRand(20) == 0)
        {
            Player[A].Controls.Run = false;
            if(iRand(2) == 0)
                Player[A].Controls.Up = true;
            if(iRand(2) == 0)
                Player[A].Controls.Right = false;
        }

        if(!Player[A].Slide && (Player[A].Slope > 0 || Player[A].StandingOnNPC > 0 || Player[A].Location.SpeedY == 0.0))
        {
            tempLocation = Player[A].Location;
            tempLocation.Width = 95;
            tempLocation.Height -= 1;

            for(int B : treeBlockQuery(tempLocation, SORTMODE_NONE))
            {
                if(BlockSlope[Block[B].Type] == 0 && !BlockIsSizable[Block[B].Type] &&
                   !BlockOnlyHitspot1[Block[B].Type] && !Block[B].Hidden)
                {
                    if(CheckCollision(Block[B].Location, tempLocation))
                    {
                        Player[A].CanJump = true;
                        Player[A].SpinJump = false;
                        Player[A].Controls.Jump = true;
                        break;
                    }
                }
            }
        }

        if(Player[A].Slope == 0 && !Player[A].Slide && Player[A].StandingOnNPC == 0 && (Player[A].Slope > 0 || Player[A].Location.SpeedY == 0.0))
        {
            tempBool = false;
            tempLocation = Player[A].Location;
            tempLocation.Width = 16;
            tempLocation.Height = 16;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height;

            for(int B : treeBlockQuery(tempLocation, SORTMODE_NONE))
            {
                if((!BlockIsSizable[Block[B].Type] || Block[B].Location.Y > Player[A].Location.Y + Player[A].Location.Height - 1) &&
                   !BlockOnlyHitspot1[Block[B].Type] && !Block[B].Hidden)
                {
                    if(CheckCollision(Block[B].Location, tempLocation))
                    {
                        tempBool = true;
                        break;
                    }
                }
            }

            if(!tempBool)
            {
                Player[A].CanJump = true;
                Player[A].SpinJump = false;
                Player[A].Controls.Jump = true;
            }
        }

        if(Player[A].Character == 5 && Player[A].Controls.Jump)
        {
            Player[A].Controls.AltJump = true;
            // .Controls.Jump = False
        }
    }

    if(LevelMacro > LEVELMACRO_OFF)
        UpdateMacro();

    ClearTriggeredEvents();
    UpdateLayers();
    UpdateNPCs();
    UpdateBlocks();
    UpdateEffects();
    UpdatePlayer();
    UpdateGraphics();
    UpdateSound();
    UpdateEvents();
    updateScreenFaders();
}

void MenuLoop()
{
    lunaLoop();

    // ConnectScreen and ControlsSettings screens handles its own input method polling
    if(MenuMode != MENU_CHARACTER_SELECT_NEW && MenuMode != MENU_INPUT_SETTINGS && !g_pollingInput)
        Controls::PollInputMethod();
    Controls::Update();
    if(!SharedCursor.Primary && !SharedCursor.Secondary)
        MenuMouseRelease = true;
    // replicates legacy behavior allowing clicks to be detected
    if(SharedCursor.Primary || SharedCursor.Secondary || SharedCursor.Tertiary)
        SharedCursor.Move = true;

    if(mainMenuUpdate())
        return;

    Integrator::sync();

    if(!g_gameInfo.introDeadMode)
        updateIntroLevelActivity();
    else
    {
        ClearTriggeredEvents();
        UpdateLayers();
        UpdateNPCs();
        UpdateBlocks();
        UpdateEffects();
//        UpdatePlayer();
        UpdateGraphics();
        UpdateSound();
        UpdateEvents();
        updateScreenFaders();
    }

    if(SharedCursor.Primary)
    {
        const Location_t cursorLoc = newLoc(SharedCursor.X - vScreen[1].X - vScreen[1].TargetX(), SharedCursor.Y - vScreen[1].Y - vScreen[1].TargetY());
        if(iRand(5) >= 2)
        {
            NewEffect(EFFID_SPARKLE, cursorLoc);
            Effect[numEffects].Location.SpeedX = dRand() * 4 - 2;
            Effect[numEffects].Location.SpeedY = dRand() * 4 - 2;
        }

        for(int A : treeNPCQuery(cursorLoc, SORTMODE_NONE))
        {
            if(NPC[A].Active)
            {
                if(CheckCollision(cursorLoc, NPC[A].Location))
                {
                    if(!NPC[A]->IsACoin)
                    {
                        NPC[0] = NPC[A];
                        NPC[0].Location.X = SharedCursor.X - vScreen[1].X - vScreen[1].ScreenLeft;
                        NPC[0].Location.Y = SharedCursor.Y - vScreen[1].Y - vScreen[1].ScreenTop;
                        NPCHit(A, 3, 0);
                    }
                    else
                    {
                        NewEffect(EFFID_COIN_COLLECT, NPC[A].Location);
                        NPC[A].Killed = 9;
                        NPCQueues::Killed.push_back(A);
                    }
                }
            }
        }

        for(int A : treeBlockQuery(cursorLoc, SORTMODE_COMPAT))
        {
            if(!Block[A].Hidden)
            {
                if(CheckCollision(cursorLoc, Block[A].Location))
                {
                    BlockHit(A);
                    BlockHitHard(A);
                }
            }
        }
    }

    if(SharedCursor.Primary || SharedCursor.Secondary)
        MenuMouseRelease = false;
    MenuMouseClick = false;
}
