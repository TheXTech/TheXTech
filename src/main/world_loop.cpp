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

#include <Utils/files.h>
#include <pge_delay.h>
#include <fmt_format_ne.h>

#include "../globals.h"
#include "../frame_timer.h"
#include "../game_main.h"
#include "../sound.h"
#include "../control/joystick.h"
#include "../effect.h"
#include "../graphics.h"
#include "../collision.h"
#include "../main/trees.h"
#include "level_file.h"
#include "speedrunner.h"

#include "../config.h"

#include "../pseudo_vb.h"


void WorldLoop()
{
    bool musicReset = false;
    Location_t tempLocation;
    int A = 0;
    int B = 0;
    if(SingleCoop > 0)
        SingleCoop = 1;
    vScreenX[1] = -(WorldPlayer[1].Location.X + WorldPlayer[1].Location.Width / 2.0) + vScreen[1].Width / 2.0;
    vScreenY[1] = -(WorldPlayer[1].Location.Y + WorldPlayer[1].Location.Height / 2.0) + vScreen[1].Height / 2.0 + 32;
    if(numPlayers > 2)
        numPlayers = 1;
    for(B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 2)
            Player[B].Mount = 0;
    }

    speedRun_tick();
    UpdateGraphics2();
    UpdateControls();
    UpdateSound();

    if(curWorldLevel > 0)
    {
        if(LevelBeatCode > 0)
        {
            //for(A = 1; A <= numWorldMusic; A++)
            for(auto *t : treeWorldMusicQuery(WorldPlayer[1].Location, false))
            {
                WorldMusic_t &mus = *t;
                if(CheckCollision(WorldPlayer[1].Location, mus.Location))
                {
                    if(curWorldMusic != mus.Type)
                        StartMusic(mus.Type);
                }
            }

            for(A = 1; A <= 4; A++)
            {
                if(WorldLevel[curWorldLevel].LevelExit[A] == LevelBeatCode || WorldLevel[curWorldLevel].LevelExit[A] == -1)
                {
                    WorldPlayer[1].LevelName = WorldLevel[curWorldLevel].LevelName;
                    LevelPath(WorldLevel[curWorldLevel], A);
                }
            }

            SaveGame();
            LevelBeatCode = 0;
        }
        else if(LevelBeatCode == -1)
        {
            //for(A = 1; A <= numWorldMusic; A++)
            for(auto *t : treeWorldMusicQuery(WorldPlayer[1].Location, false))
            {
                WorldMusic_t &mus = *t;
                if(CheckCollision(WorldPlayer[1].Location, mus.Location))
                {
                    if(curWorldMusic != mus.Type)
                        StartMusic(mus.Type);
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(WorldPlayer[1].Location, false))
            {
                WorldLevel_t &level = *t;
                if(CheckCollision(WorldPlayer[1].Location, level.Location))
                {
                    curWorldLevel = level.index;
                    WorldPlayer[1].LevelName = level.LevelName;
                    break;
                }
            }

            if(curWorldLevel > 0)
                LevelPath(WorldLevel[curWorldLevel], 5);

            SaveGame();
            LevelBeatCode = 0;
        }
    }
    else
        LevelBeatCode = 0;

    for(A = 1; A <= numPlayers; A++)
    {
        Player[A].Bumped = false;
        Player[A].Bumped2 = 0;
        Player[A].CanFly = false;
        Player[A].CanFly2 = false;
        Player[A].Effect = 0;
        Player[A].Effect2 = 0;
        Player[A].FlyCount = 0;
        Player[A].TailCount = 0;
        Player[A].Stoned = false;
    }

    if(WorldPlayer[1].Move > 0)
    {
        WorldPlayer[1].Frame2 = WorldPlayer[1].Frame2 + 1;
        if(WorldPlayer[1].Frame2 >= 8)
        {
            WorldPlayer[1].Frame2 = 0;
            WorldPlayer[1].Frame = WorldPlayer[1].Frame + 1;
        }
        if(WorldPlayer[1].Move == 1)
        {
            if(WorldPlayer[1].Frame < 6)
                WorldPlayer[1].Frame = 7;
            if(WorldPlayer[1].Frame > 7)
                WorldPlayer[1].Frame = 6;
        }
        if(WorldPlayer[1].Move == 4)
        {
            if(WorldPlayer[1].Frame < 2)
                WorldPlayer[1].Frame = 3;
            if(WorldPlayer[1].Frame > 3)
                WorldPlayer[1].Frame = 2;
        }
        if(WorldPlayer[1].Move == 3)
        {
            if(WorldPlayer[1].Frame < 0)
                WorldPlayer[1].Frame = 1;
            if(WorldPlayer[1].Frame > 1)
                WorldPlayer[1].Frame = 0;
        }
        if(WorldPlayer[1].Move == 2)
        {
            if(WorldPlayer[1].Frame < 4)
                WorldPlayer[1].Frame = 5;
            if(WorldPlayer[1].Frame > 5)
                WorldPlayer[1].Frame = 4;
        }
    }


    if(WorldPlayer[1].Move == 0)
    {
        tempLocation = WorldPlayer[1].Location;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        WorldPlayer[1].LevelName.clear();

#ifndef NO_SDL
        bool altPressed = getKeyState(SDL_SCANCODE_LALT) == KEY_PRESSED ||
                          getKeyState(SDL_SCANCODE_RALT) == KEY_PRESSED;

        bool escPressed = getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED;
#else
        bool altPressed = false;
        bool escPressed = false;
#endif

#ifdef __ANDROID__
        escPressed |= getKeyState(SDL_SCANCODE_AC_BACK) == KEY_PRESSED;
#endif

        bool pausePress = (Player[1].Controls.Start || escPressed) && !altPressed;

        //for(A = 1; A <= numWorldLevels; A++)
        for(auto *t : treeWorldLevelQuery(tempLocation, true))
        {
            WorldLevel_t &level = *t;
            if(CheckCollision(tempLocation, level.Location))
            {
                WorldPlayer[1].LevelName = level.LevelName;
                break;
            }
        }

        if(pausePress)
        {
            if(Player[1].UnStart)
                PauseGame(1);
        }

        if(Player[1].Controls.Up)
        {
            tempLocation.Y = tempLocation.Y - 32;
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : treeWorldPathQuery(tempLocation, true))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 1;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, true))
            {
                WorldLevel_t &level = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, level.Location) && level.Active)
                    {
                        WorldPlayer[1].Move = 1;
                        break;
                    }
                }
            }
            if(WalkAnywhere)
                WorldPlayer[1].Move = 1;
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_BlockHit);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Left)
        {
            tempLocation.X = tempLocation.X - 32;
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : treeWorldPathQuery(tempLocation, true))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 2;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, true))
            {
                WorldLevel_t &level = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, level.Location) && level.Active)
                    {
                        WorldPlayer[1].Move = 2;
                        break;
                    }
                }
            }

            if(WalkAnywhere)
                WorldPlayer[1].Move = 2;
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_BlockHit);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Down)
        {
            tempLocation.Y = tempLocation.Y + 32;
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : treeWorldPathQuery(tempLocation, true))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 3;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, true))
            {
                WorldLevel_t &level = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, level.Location) && level.Active)
                    {
                        WorldPlayer[1].Move = 3;
                        break;
                    }
                }
            }

            if(WalkAnywhere)
                WorldPlayer[1].Move = 3;
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_BlockHit);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Right)
        {
            tempLocation.X = tempLocation.X + 32;
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : treeWorldPathQuery(tempLocation, true))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 4;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, true))
            {
                WorldLevel_t &level = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, level.Location) && level.Active)
                    {
                        WorldPlayer[1].Move = 4;
                        break;
                    }
                }
            }

            if(WalkAnywhere)
                WorldPlayer[1].Move = 4;
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_BlockHit);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Jump && Player[1].UnStart)
        {
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, true))
            {
                WorldLevel_t &level = *t;
                if(CheckCollision(tempLocation, level.Location))
                {
                    if(int(level.WarpX) != -1)
                        WorldPlayer[1].Location.X = level.WarpX;

                    if(int(level.WarpY) != -1)
                        WorldPlayer[1].Location.Y = level.WarpY;

                    if(int(level.WarpY) != -1 || int(level.WarpX) != -1)
                    {
                        LevelBeatCode = 6;
                        //for(B = 1; B <= numWorldLevels; B++)
                        for(auto *t2 : treeWorldLevelQuery(WorldPlayer[1].Location, true))
                        {
                            WorldLevel_t &level2 = *t2;
                            if(CheckCollision(WorldPlayer[1].Location, level2.Location))
                            {
                                level2.Active = true;
                                curWorldLevel = level2.index;
                            }
                        }
                    }

                    if(!level.FileName.empty() && level.FileName != ".lvl" && level.FileName != ".lvlx")
                    {
                        addMissingLvlSuffix(level.FileName);
                        if(Files::fileExists(SelectWorld[selWorld].WorldPath + level.FileName))
                        {
                            StartWarp = level.StartWarp;
                            StopMusic();
                            PlaySound(SFX_LevelSelect);
                            SoundPause[26] = 200;
                            curWorldLevel = level.index;
                            LevelSelect = false;
                            GameThing();
                            ClearLevel();
                            PGE_Delay(1000);
                            std::string levelPath = SelectWorld[selWorld].WorldPath + level.FileName;
                            if(!OpenLevel(levelPath))
                            {
                                MessageText = fmt::format_ne("ERROR: Can't open \"{0}\": file doesn't exist or corrupted.", level.FileName);
                                PauseGame(1);
                                ErrorQuit = true;
                            }
                            break;
                        }
                    }
                    else if(int(level.WarpX) != -1 || int(level.WarpY) != -1)
                    {
                        musicReset = true;
                        StopMusic();
                        PlaySound(SFX_LevelSelect);
                        frmMain.setTargetTexture();
                        frmMain.clearBuffer();
                        frmMain.repaint();
                        DoEvents();
                        PGE_Delay(1000);
                        resetFrameTimer();
                    }
                }
            }
        }
        if(WorldPlayer[1].Move3 && WorldPlayer[1].Move == 0)
        {
            if(g_config.FastMove)
                PlayerPath(1);
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_Slide);
            }
        }

        if(WorldPlayer[1].Move == 0)
        {
            if(WorldPlayer[1].Frame == 5)
                WorldPlayer[1].Frame = 4;
            if(WorldPlayer[1].Frame == 3)
                WorldPlayer[1].Frame = 2;
        }

        WorldPlayer[1].LastMove = 0;

        //for(A = 1; A <= numWorldMusic; A++)
        for(auto *t : treeWorldMusicQuery(tempLocation, true))
        {
            WorldMusic_t &mus = *t;
            if(CheckCollision(WorldPlayer[1].Location, mus.Location))
            {
                if((curWorldMusic != mus.Type) ||
                   (mus.Type == CustomWorldMusicId() && curWorldMusicFile != mus.MusicFile))
                {
                    curWorldMusicFile = mus.MusicFile;
                    StartMusic(mus.Type);
                    musicReset = false;
                }
            }
        }

        if(musicReset) // Resume the last playing music after teleportation
        {
            StartMusic(curWorldMusic);
            // musicReset = false;
        }
    }
    else if(WorldPlayer[1].Move == 1)
    {
        WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
        WorldPlayer[1].Location.Y = WorldPlayer[1].Location.Y - 2;
        if(WalkAnywhere || g_config.FastMove)
        {
            WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
            WorldPlayer[1].Location.Y = WorldPlayer[1].Location.Y - 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].LastMove = WorldPlayer[1].Move;
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 2)
    {
        WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
        WorldPlayer[1].Location.X = WorldPlayer[1].Location.X - 2;
        if(WalkAnywhere || g_config.FastMove)
        {
            WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
            WorldPlayer[1].Location.X = WorldPlayer[1].Location.X - 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].LastMove = WorldPlayer[1].Move;
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 3)
    {
        WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
        WorldPlayer[1].Location.Y = WorldPlayer[1].Location.Y + 2;
        if(WalkAnywhere || g_config.FastMove)
        {
            WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
            WorldPlayer[1].Location.Y = WorldPlayer[1].Location.Y + 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].LastMove = WorldPlayer[1].Move;
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 4)
    {
        WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
        WorldPlayer[1].Location.X = WorldPlayer[1].Location.X + 2;
        if(WalkAnywhere || g_config.FastMove)
        {
            WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
            WorldPlayer[1].Location.X = WorldPlayer[1].Location.X + 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].LastMove = WorldPlayer[1].Move;
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }

}

void LevelPath(const WorldLevel_t &Lvl, int Direction, bool Skp)
{
    Location_t tempLocation;
//    int A = 0;

    // Up
    if(Direction == 1 || Direction == 5)
    {
        tempLocation = Lvl.Location;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.Y = tempLocation.Y - 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : treeWorldPathQuery(tempLocation, false))
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                }
            }
        }
    }

    // Left
    if(Direction == 2 || Direction == 5)
    {
        tempLocation = Lvl.Location;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.X = tempLocation.X - 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : treeWorldPathQuery(tempLocation, false))
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                }
            }
        }
    }

    // Down
    if(Direction == 3 || Direction == 5)
    {
        tempLocation = Lvl.Location;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.Y = tempLocation.Y + 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : treeWorldPathQuery(tempLocation, false))
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                }
            }
        }
    }

    // Right
    if(Direction == 4 || Direction == 5)
    {
        tempLocation = Lvl.Location;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.X = tempLocation.X + 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : treeWorldPathQuery(tempLocation, false))
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                }
            }
        }
    }
}

void PlayerPath(int A)
{
    if(!WorldPlayer[A].LevelName.empty())
        return;

    Location_t tempLocation = WorldPlayer[A].Location;

    tempLocation.X += 4;
    tempLocation.Y += 4;
    tempLocation.Width -= 8;
    tempLocation.Height -= 8;

    int n_moves = 0;
    for(int B = 1; B <= 4; B++)
    {
        if(B == 1)
            tempLocation.Y -= 32; // Up
        else if(B == 2)
        {
            tempLocation.Y += 32; // Down
            tempLocation.X -= 32; // Left
        }
        else if(B == 3)
        {
            tempLocation.X += 32; // Right
            tempLocation.Y += 32; // Down
        }
        else if(B == 4)
        {
            tempLocation.Y -= 32; // Up
            tempLocation.X += 32; // Right
        }
        if(B != WorldPlayer[A].LastMove && B % 2 == WorldPlayer[A].LastMove % 2)
            continue;
        for(auto *t : treeWorldPathQuery(tempLocation, false))
        {
            WorldPath_t& path = *t;
            if(CheckCollision(tempLocation, path.Location) && path.Active)
            {
                WorldPlayer[A].Move = B;
                n_moves ++;
                break;
            }
        }
        if(WorldPlayer[A].Move == B)
            continue;
        for(auto *t : treeWorldLevelQuery(tempLocation, false))
        {
            WorldLevel_t& level = *t;
            if(CheckCollision(tempLocation, level.Location) && level.Active)
            {
                WorldPlayer[A].Move = B;
                n_moves ++;
                break;
            }
        }
    }
    if(n_moves > 1)
        WorldPlayer[A].Move = 0;
}

void PathPath(WorldPath_t &Pth, bool Skp)
{
    //int A = 0;
    int B = 0;

    Location_t tempLocation;
    tempLocation = Pth.Location;
    tempLocation.X = tempLocation.X + 4;
    tempLocation.Y = tempLocation.Y + 4;
    tempLocation.Width = tempLocation.Width - 8;
    tempLocation.Height = tempLocation.Height - 8;

    //for(A = 1; A <= numScenes; A++)
    for(auto *t : treeWorldSceneQuery(tempLocation, true))
    {
        Scene_t &scene = *t;
        if(scene.Active)
        {
            if(CheckCollision(tempLocation, scene.Location))
                scene.Active = false;
        }
    }

    if(!Pth.Active && !Skp)
    {
        Pth.Active = true;
        vScreenX[1] = -(Pth.Location.X + Pth.Location.Width / 2.0) + vScreen[1].Width / 2.0;
        vScreenY[1] = -(Pth.Location.Y + Pth.Location.Height / 2.0) + vScreen[1].Height / 2.0;
        PlaySound(SFX_NewPath);
        PathWait();
    }

    Pth.Active = true;

    for(B = 1; B <= 4; B++)
    {
        if(B == 1)
            tempLocation.Y -= 32; // Up
        else if(B == 2)
        {
            tempLocation.Y += 32; // Down
            tempLocation.X -= 32; // Left
        }
        else if(B == 3)
        {
            tempLocation.X += 32; // Right
            tempLocation.Y += 32; // Down
        }
        else if(B == 4)
        {
            tempLocation.Y -= 32; // Up
            tempLocation.X += 32; // Right
        }

        //for(A = 1; A <= numWorldPaths; A++)
        WorldPath_t* found = nullptr;
        for(WorldPath_t* path : treeWorldPathQuery(tempLocation, true))
        {
            D_pLogDebug("Found path activity: %d", (int)path->Active);
            if(!path->Active)
            {
                D_pLogDebugNA("Collision with path...");
                if(CheckCollision(tempLocation, path->Location))
                {
                    D_pLogDebugNA("Collision with path FOUND...");
                    found = path;
                    break;
                }
            }
        }
        if(found)
            PathPath(*found, Skp);

        //for(A = 1; A <= numWorldLevels; A++)
        for(auto *t : treeWorldLevelQuery(tempLocation, true))
        {
            WorldLevel_t &lev = *t;
            D_pLogDebug("Found level activity: %d", (int)lev.Active);
            if(!lev.Active)
            {
                D_pLogDebugNA("Collision with level...");
                if(CheckCollision(tempLocation, lev.Location))
                {
                    D_pLogDebugNA("Collision with level FOUND...");
                    lev.Active = true;
                    if(!Skp)
                    {
                        vScreenX[1] = -(lev.Location.X + lev.Location.Width / 2.0) + vScreen[1].Width / 2.0;
                        vScreenY[1] = -(lev.Location.Y + lev.Location.Height / 2.0) + vScreen[1].Height / 2.0;
                        PlaySound(SFX_NewPath);
                        PathWait();
                    }
                }
            }
        }
    }
}

void PathWait()
{
    int C = 0;

    resetFrameTimer();

    do
    {
        DoEvents();
        if(canProceedFrame())
        {
            speedRun_tick();
            UpdateGraphics2();
            UpdateSound();

            C++;
            computeFrameTime1();
            DoEvents();
            computeFrameTime2();
        }
        PGE_Delay(1);
    } while(!(C >= 24));

    resetFrameTimer();
}
