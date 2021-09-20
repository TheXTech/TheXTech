/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#include <PGE_File_Formats/file_formats.h>
#include <pge_delay.h>
#include <fmt_format_ne.h>

#include "../globals.h"
#include "../game_main.h"
#include "../sound.h"
#include "../control/joystick.h"
#include "../effect.h"
#include "../graphics.h"
#include "../blocks.h"
#include "../npc.h"
#include "../layers.h"
#include "../player.h"
#include "../collision.h"
#include "level_file.h"
#include "menu_main.h"
#include "game_info.h"
#include "speedrunner.h"
#include "trees.h"

#include "../pseudo_vb.h"

static void updateIntroLevelActivity()
{
    Location_t tempLocation;
    bool tempBool;

    SingleCoop = 0;

    if(CheckLiving() == 0)
    {
        ShowLayer(LAYER_DESTROYED_BLOCKS);

        For(A, 1, numNPCs)
        {
            if(NPC[A].DefaultType == 0)
            {
                if(NPC[A].TimeLeft > 10) NPC[A].TimeLeft = 10;
            }
        }
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
            if((p.State ==3 || p.State == 6 || p.State == 7) && (dRand() * 100.0) > 90.0)
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

                For(B, 1, numNPCs)
                {
                    if(NPC[B].Active && !NPCIsABonus[NPC[B].Type] &&
                       !NPCWontHurt[NPC[B].Type] && NPC[B].HoldingPlayer == 0)
                    {
                        if(CheckCollision(tempLocation, NPC[B].Location))
                            p.Controls.Run = false;
                    }
                }
            }

            if(p.StandingOnNPC > 0)
            {
                if(NPCGrabFromTop[NPC[p.StandingOnNPC].Type])
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

                For(B, 1, numNPCs)
                {
                    if(NPC[B].Active && !NPCIsABonus[NPC[B].Type] &&
                      !NPCWontHurt[NPC[B].Type] && NPC[B].HoldingPlayer == 0)
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

                    For(B, 1, numNPCs)
                    {
                        if(NPC[B].Active && !NPCIsABonus[NPC[B].Type] &&
                           !NPCWontHurt[NPC[B].Type] && NPC[B].HoldingPlayer == 0)
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

                    For(B, 1, numNPCs)
                    {
                        if(NPC[B].Active && !NPCIsABonus[NPC[B].Type] &&
                           !NPCWontHurt[NPC[B].Type] && NPC[B].HoldingPlayer == 0)
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

        if(p.Location.X < -vScreenX[1] - p.Location.Width && !(-vScreenX[1] <= level[0].X))
            p.Dead = true;

        if(p.Location.X > -vScreenX[1] + vScreen[1].Width + 200)
            p.Dead = true;

        if(p.Location.X > -vScreenX[1] + vScreen[1].Width * 0.75 && -vScreenX[1] + vScreen[1].Width + 50 < level[0].Width)
            p.Controls.Run = false;

        if(-vScreenX[1] <= level[0].X && (p.Dead || p.TimeToLive > 0) && g_gameInfo.introMaxPlayersCount > 0)
        {
            p.ForceHold = 65;
            p.State = iRand(6) + 2;
            p.CanFly = false;
            p.CanFly2 = false;
            p.TailCount = 0;
            p.Dead = false;
            p.TimeToLive = 0;
            p.Character = iRand(5) + 1;

            if(A >= 1 && A <= 5)
                p.Character = A;

            p.HeldBonus = 0;
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

            PlayerFrame(A);

            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            p.Location.X = level[p.Section].X - A * 48;
            p.Location.SpeedX = double(Physics.PlayerRunSpeed);
            p.Location.Y = level[p.Section].Height - p.Location.Height - 33;

            do
            {
                tempBool = true;
                for(Block_t* block : treeBlockQuery(p.Location, false))
                {
                    Block_t& b = *block;
                    if(CheckCollision(p.Location, b.Location))
                    {
                        p.Location.Y = b.Location.Y - p.Location.Height - 0.1;
                        tempBool = false;
                        break;
                    }
                }
            } while(!tempBool);

            if(!UnderWater[p.Section])
            {
                if(iRand(25) + 1 == 25)
                {
                    p.Mount = 1;
                    p.MountType = iRand(3) + 1;
                    if(p.State == 1)
                    {
                        p.Location.Height = Physics.PlayerHeight[1][2];
                        p.Location.Y = p.Location.Y - Physics.PlayerHeight[1][2] + Physics.PlayerHeight[p.Character][1];
                    }
                }
            }

            if(p.Mount == 0 && p.Character <= 2)
            {
                if(iRand(15) + 1 == 15)
                {
                    p.Mount = 3;
                    p.MountType = iRand(7) + 1;
                    p.Location.Y = p.Location.Y + p.Location.Height;
                    p.Location.Height = Physics.PlayerHeight[2][2];
                    p.Location.Y = p.Location.Y - p.Location.Height - 0.01;
                }
            }

            p.CanFly = false;
            p.CanFly2 = false;
            p.RunCount = 0;

            if(p.Mount == 0 && p.Character != 5)
            {
                numNPCs += 1;
                p.HoldingNPC = numNPCs;
                p.ForceHold = 120;

                {
                    NPC_t &n = NPC[numNPCs];
                    do
                    {
                        do
                        {
                            n.Type = iRand(286) + 1;
                        } while(n.Type == 11 || n.Type == 16 || n.Type == 18 || n.Type == 15 ||
                                n.Type == 21 || n.Type == 12 || n.Type == 13 || n.Type == 30 ||
                                n.Type == 17 || n.Type == 31 || n.Type == 32 ||
                                (n.Type >= 37 && n.Type <= 44) || n.Type == 46 || n.Type == 47 ||
                                n.Type == 50 || (n.Type >= 56 && n.Type <= 70) || n.Type == 8 ||
                                n.Type == 74 || n.Type == 51 || n.Type == 52 || n.Type == 75 ||
                                n.Type == 34 || NPCIsToad[n.Type] || NPCIsAnExit[n.Type] ||
                                NPCIsYoshi[n.Type] || (n.Type >= 78 && n.Type <= 87) ||
                                n.Type == 91 || n.Type == 93 || (n.Type >= 104 && n.Type <= 108) ||
                                n.Type == 125 || n.Type == 133 || (n.Type >= 148 && n.Type <= 151) ||
                                n.Type == 159 || n.Type == 160 || n.Type == 164 || n.Type == 168 ||
                                (n.Type >= 154 && n.Type <= 157) || n.Type == 159 || n.Type == 160 ||
                                n.Type == 164 || n.Type == 165 || n.Type == 171 || n.Type == 178 ||
                                n.Type == 197 || n.Type == 180 || n.Type == 181 || n.Type == 190 ||
                                n.Type == 192 || n.Type == 196 || n.Type == 197 ||
                                (UnderWater[0] == true && NPCIsBoot[n.Type] == true) ||
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
                    n.Location.Height = NPCHeight[n.Type];
                    n.Location.Width = NPCWidth[n.Type];
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
            if((dRand() * 100) > 98.0 || p.Location.SpeedY == 0.0 || p.Slope > 0)
                p.Controls.Jump = true;
        }

        if((dRand() * 100) > 95.0 && Player[A].HoldingNPC == 0 && Player[A].Slide == false && Player[A].CanAltJump == true && Player[A].Mount == 0)
            Player[A].Controls.AltJump = true;
        if(dRand() * 1000 >= 999 && Player[A].CanFly2 == false)
            Player[A].Controls.Run = false;
        if(Player[A].Mount == 3 && dRand() * 100 >= 98 && Player[A].RunRelease == false)
            Player[A].Controls.Run = false;
        if(NPC[Player[A].HoldingNPC].Type == 22 || NPC[Player[A].HoldingNPC].Type == 49)
            Player[A].Controls.Run = true;
        if(Player[A].Slide == true && Player[A].CanJump == true)
        {
            if(Player[A].Location.SpeedX > -2 && Player[A].Location.SpeedX < 2)
                Player[A].Controls.Jump = true;
        }
        if(Player[A].CanFly == false && Player[A].CanFly2 == false && (Player[A].State == 4 || Player[A].State == 5) && Player[A].Slide == false)
            Player[A].Controls.Jump = true;
        if(Player[A].Quicksand > 0)
        {
            Player[A].CanJump = true;
            Player[A].Controls.Jump = true;
        }
        if(Player[A].FloatTime > 0 || (Player[A].CanFloat == true && Player[A].FloatRelease == true && Player[A].Jump == 0 && Player[A].Location.SpeedY > 0 && (dRand() * 100) > 95.0))
            Player[A].Controls.Jump = true;
        if(NPC[Player[A].HoldingNPC].Type == 13 && (dRand() * 100) > 95.0)
        {
            Player[A].Controls.Run = false;
            if(iRand(2) == 1)
                Player[A].Controls.Up = true;
            if(iRand(2) == 1)
                Player[A].Controls.Right = false;
        }

        if(Player[A].Slide == false && (Player[A].Slope > 0 || Player[A].StandingOnNPC > 0 || Player[A].Location.SpeedY == 0.0))
        {
            tempLocation = Player[A].Location;
            tempLocation.Width = 95;
            tempLocation.Height = tempLocation.Height - 1;
            for(Block_t* block : treeBlockQuery(tempLocation, false))
            {
                Block_t& b = *block;
                if(BlockSlope[b.Type] == 0 && !BlockIsSizable[b.Type] && !BlockOnlyHitspot1[b.Type] && !b.Hidden)
                {
                    if(CheckCollision(b.Location, tempLocation) == true)
                    {
                        Player[A].CanJump = true;
                        Player[A].SpinJump = false;
                        Player[A].Controls.Jump = true;
                        break;
                    }
                }
            }
        }
        if(Player[A].Slope == 0 && Player[A].Slide == false && Player[A].StandingOnNPC == 0 && (Player[A].Slope > 0 || Player[A].Location.SpeedY == 0.0))
        {
            tempBool = false;
            tempLocation = Player[A].Location;
            tempLocation.Width = 16;
            tempLocation.Height = 16;
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width;
            tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height;
            for(Block_t* block : treeBlockQuery(tempLocation, false))
            {
                Block_t& b = *block;
                if((!BlockIsSizable[b.Type] || b.Location.Y > Player[A].Location.Y + Player[A].Location.Height - 1) && !BlockOnlyHitspot1[b.Type] && !b.Hidden)
                {
                    if(CheckCollision(b.Location, tempLocation) == true)
                    {
                        tempBool = true;
                        break;
                    }
                }
            }
            if(tempBool == false)
            {
                Player[A].CanJump = true;
                Player[A].SpinJump = false;
                Player[A].Controls.Jump = true;
            }
        }
        if(Player[A].Character == 5 && Player[A].Controls.Jump == true)
        {
            Player[A].Controls.AltJump = true;
            // .Controls.Jump = False
        }
    }

    if(LevelMacro > LEVELMACRO_OFF)
        UpdateMacro();

    UpdateLayers();
    UpdateNPCs();
    UpdateBlocks();
    UpdateEffects();
    UpdatePlayer();
    UpdateGraphics();
    UpdateSound();
    UpdateEvents();
}

void MenuLoop()
{
    UpdateControls();

    if(mainMenuUpdate())
        return;

    if(g_gameInfo.introEnableActivity && g_gameInfo.introMaxPlayersCount > 0)
        updateIntroLevelActivity();
    else
    {
        UpdateLayers();
        UpdateNPCs();
        UpdateBlocks();
        UpdateEffects();
        //UpdatePlayer();
        UpdateGraphics();
        UpdateSound();
        UpdateEvents();
    }

    if(MenuMouseDown)
    {
        if(dRand() * 100 > 40.0)
        {
            NewEffect(80, newLoc(MenuMouseX - vScreenX[1], MenuMouseY - vScreenY[1]));
            Effect[numEffects].Location.SpeedX = dRand() * 4 - 2;
            Effect[numEffects].Location.SpeedY = dRand() * 4 - 2;
        }

        For(A, 1, numNPCs)
        {
            if(NPC[A].Active)
            {
                if(CheckCollision(newLoc(MenuMouseX - vScreenX[1], MenuMouseY - vScreenY[1]), NPC[A].Location))
                {
                    if(!NPCIsACoin[NPC[A].Type])
                    {
                        NPC[0] = NPC[A];
                        NPC[0].Location.X = MenuMouseX - vScreenX[1];
                        NPC[0].Location.Y = MenuMouseY - vScreenY[1];
                        NPCHit(A, 3, 0);
                    }
                    else
                    {
                        NewEffect(78, NPC[A].Location);
                        NPC[A].Killed = 9;
                    }
                }
            }
        }

        Location_t tempLocation = newLoc(MenuMouseX - vScreenX[1], MenuMouseY - vScreenY[1]);
        for(Block_t* block : treeBlockQuery(tempLocation, false))
        {
            int A = block - &Block[1] + 1;
            if(!Block[A].Hidden)
            {
                if(CheckCollision(tempLocation, Block[A].Location))
                {
                    BlockHit(A);
                    BlockHitHard(A);
                }
            }
        }
    }

    MenuMouseMove = false;
    MenuMouseClick = false;

    if(MenuMouseDown)
        MenuMouseRelease = false;
    else
        MenuMouseRelease = true;

    MenuMouseBack = false;
}

void FindSaves()
{
    std::string newInput;
    std::string episode = SelectWorld[selWorld].WorldPath;
    GamesaveData f;

    for(auto A = 1; A <= maxSaveSlots; A++)
    {
        SaveSlot[A] = -1;
        SaveStars[A] = 0;

        std::string saveFile = makeGameSavePath(episode,
                                                SelectWorld[selWorld].WorldFile,
                                                fmt::format_ne("save{0}.savx", A));
        std::string saveFileOld = episode + fmt::format_ne("save{0}.savx", A);
        std::string saveFileAncient = episode + fmt::format_ne("save{0}.sav", A);

        if((Files::fileExists(saveFile) && FileFormats::ReadExtendedSaveFileF(saveFile, f)) ||
           (Files::fileExists(saveFileOld) && FileFormats::ReadExtendedSaveFileF(saveFileOld, f)) ||
           (Files::fileExists(saveFileAncient) && FileFormats::ReadSMBX64SavFileF(saveFileAncient, f)))
        {
            int curActive = 0;
            int maxActive = 0;

            // "game beat flag"
            maxActive++;
            if(f.gameCompleted)
                curActive++;

            // How much paths open
            maxActive += f.visiblePaths.size();
            for(auto &p : f.visiblePaths)
            {
                if(p.second)
                    curActive++;
            }

            // How much levels opened
            maxActive += f.visibleLevels.size();
            for(auto &p : f.visibleLevels)
            {
                if(p.second)
                    curActive++;
            }

            // How many stars collected
            maxActive += (int(f.totalStars) * 4);
            SaveStars[A] = int(f.gottenStars.size());

            curActive += (SaveStars[A] * 4);

            if(maxActive > 0)
                SaveSlot[A] = int((float(curActive) / float(maxActive)) * 100);
            else
                SaveSlot[A] = 100;
        }
    }
}
