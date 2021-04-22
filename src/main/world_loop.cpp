/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <SDL2/SDL_timer.h>

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

#include "../pseudo_vb.h"


void WorldLoop()
{
    // Keep them static to don't re-alloc them for every iteration
    static WorldPathPtrArr parr;
    static WorldLevelPtrArr larr;
    static WorldLevelPtrArr larr2;
    static WorldMusicPtrArr marr;
    // Reserve 20 elements per every array
    parr.reserve(20);
    larr.reserve(20);
    larr2.reserve(20);
    marr.reserve(20);

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
            treeWorldMusicQuery(WorldPlayer[1].Location, marr, false);
            //for(A = 1; A <= numWorldMusic; A++)
            for(auto *t : marr)
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
            treeWorldMusicQuery(WorldPlayer[1].Location, marr, false);
            //for(A = 1; A <= numWorldMusic; A++)
            for(auto *t : marr)
            {
                WorldMusic_t &mus = *t;
                if(CheckCollision(WorldPlayer[1].Location, mus.Location))
                {
                    if(curWorldMusic != mus.Type)
                        StartMusic(mus.Type);
                }
            }

            treeWorldLevelQuery(WorldPlayer[1].Location, larr, false);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
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

        bool altPressed = getKeyState(SDL_SCANCODE_LALT) == KEY_PRESSED ||
                          getKeyState(SDL_SCANCODE_RALT) == KEY_PRESSED;

        bool escPressed = getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED;
#ifdef __ANDROID__
        escPressed |= getKeyState(SDL_SCANCODE_AC_BACK) == KEY_PRESSED;
#endif

        bool pausePress = (Player[1].Controls.Start || escPressed) && !altPressed;

        treeWorldLevelQuery(tempLocation, larr, true);
        //for(A = 1; A <= numWorldLevels; A++)
        for(auto *t : larr)
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
            treeWorldPathQuery(tempLocation, parr, true);
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : parr)
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 1;
                    break;
                }
            }

            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
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
                PlaySound(3);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Left)
        {
            tempLocation.X = tempLocation.X - 32;
            treeWorldPathQuery(tempLocation, parr, true);
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : parr)
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 2;
                    break;
                }
            }

            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
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
                PlaySound(3);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Down)
        {
            tempLocation.Y = tempLocation.Y + 32;
            treeWorldPathQuery(tempLocation, parr, true);
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : parr)
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 3;
                    break;
                }
            }

            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
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
                PlaySound(3);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Right)
        {
            tempLocation.X = tempLocation.X + 32;
            treeWorldPathQuery(tempLocation, parr, true);
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : parr)
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 4;
                    break;
                }
            }

            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
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
                PlaySound(3);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Jump && Player[1].UnStart)
        {
            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
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
                        treeWorldLevelQuery(WorldPlayer[1].Location, larr2, true);
                        //for(B = 1; B <= numWorldLevels; B++)
                        for(auto *t2 : larr2)
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
                            PlaySound(28);
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
                        StopMusic();
                        PlaySound(28);
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
        else
        {
            if(WorldPlayer[1].Frame == 5)
                WorldPlayer[1].Frame = 4;
            if(WorldPlayer[1].Frame == 3)
                WorldPlayer[1].Frame = 2;
        }
        if(WorldPlayer[1].Move3 && WorldPlayer[1].Move == 0)
        {
            WorldPlayer[1].Move3 = false;
            PlaySound(26);
        }

        treeWorldMusicQuery(tempLocation, marr, true);
        //for(A = 1; A <= numWorldMusic; A++)
        for(auto *t : marr)
        {
            WorldMusic_t &mus = *t;
            if(CheckCollision(WorldPlayer[1].Location, mus.Location))
            {
                if((curWorldMusic != mus.Type) ||
                   (mus.Type == CustomWorldMusicId() && curWorldMusicFile != mus.MusicFile))
                {
                    curWorldMusicFile = mus.MusicFile;
                    StartMusic(mus.Type);
                }
            }
        }
    }
    else if(WorldPlayer[1].Move == 1)
    {
        WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
        WorldPlayer[1].Location.Y = WorldPlayer[1].Location.Y - 2;
        if(WalkAnywhere)
        {
            WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
            WorldPlayer[1].Location.Y = WorldPlayer[1].Location.Y - 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 2)
    {
        WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
        WorldPlayer[1].Location.X = WorldPlayer[1].Location.X - 2;
        if(WalkAnywhere)
        {
            WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
            WorldPlayer[1].Location.X = WorldPlayer[1].Location.X - 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 3)
    {
        WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
        WorldPlayer[1].Location.Y = WorldPlayer[1].Location.Y + 2;
        if(WalkAnywhere)
        {
            WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
            WorldPlayer[1].Location.Y = WorldPlayer[1].Location.Y + 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 4)
    {
        WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
        WorldPlayer[1].Location.X = WorldPlayer[1].Location.X + 2;
        if(WalkAnywhere)
        {
            WorldPlayer[1].Move2 = WorldPlayer[1].Move2 + 2;
            WorldPlayer[1].Location.X = WorldPlayer[1].Location.X + 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }

}

void LevelPath(const WorldLevel_t &Lvl, int Direction, bool Skp)
{
    WorldPathPtrArr parr;
    parr.reserve(20);
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

        treeWorldPathQuery(tempLocation, parr, false);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
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

        treeWorldPathQuery(tempLocation, parr, false);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
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

        treeWorldPathQuery(tempLocation, parr, false);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
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

        treeWorldPathQuery(tempLocation, parr, false);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
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

void PathPath(WorldPath_t &Pth, bool Skp)
{
    WorldPathPtrArr parr;
    WorldLevelPtrArr larr;
    ScenePtrArr sarr;
    parr.reserve(20);
    larr.reserve(20);
    sarr.reserve(20);
    //int A = 0;
    int B = 0;

    Location_t tempLocation;
    tempLocation = Pth.Location;
    tempLocation.X = tempLocation.X + 4;
    tempLocation.Y = tempLocation.Y + 4;
    tempLocation.Width = tempLocation.Width - 8;
    tempLocation.Height = tempLocation.Height - 8;

    treeWorldSceneQuery(tempLocation, sarr, true);
    //for(A = 1; A <= numScenes; A++)
    for(auto *t : sarr)
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
        PlaySound(27);
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

        treeWorldPathQuery(tempLocation, parr, true);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
        {
            WorldPath_t &path = *t;
            D_pLogDebug("Found path activity: %d", (int)path.Active);
            if(!path.Active)
            {
                D_pLogDebugNA("Collision with path...");
                if(CheckCollision(tempLocation, path.Location))
                {
                    D_pLogDebugNA("Collision with path FOUND...");
                    PathPath(path, Skp);
                }
            }
        }

        treeWorldLevelQuery(tempLocation, larr, true);
        //for(A = 1; A <= numWorldLevels; A++)
        for(auto *t : larr)
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
                        PlaySound(27);
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
