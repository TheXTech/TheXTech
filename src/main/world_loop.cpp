/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "level_file.h"

#include "../pseudo_vb.h"


void WorldLoop()
{
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

    UpdateGraphics2();
    UpdateControls();
    UpdateSound();

    if(curWorldLevel > 0)
    {
        if(LevelBeatCode > 0)
        {
            for(A = 1; A <= numWorldMusic; A++)
            {
                if(CheckCollision(WorldPlayer[1].Location, WorldMusic[A].Location))
                {
                    if(curWorldMusic != WorldMusic[A].Type)
                    {
                        StartMusic(WorldMusic[A].Type);
                    }
                }
            }
            for(A = 1; A <= 4; A++)
            {
                if(WorldLevel[curWorldLevel].LevelExit[A] == LevelBeatCode || WorldLevel[curWorldLevel].LevelExit[A] == -1)
                {
                    WorldPlayer[1].LevelName = WorldLevel[curWorldLevel].LevelName;
                    LevelPath(curWorldLevel, A);
                }
            }
            SaveGame();
            LevelBeatCode = 0;
        }
        else if(LevelBeatCode == -1)
        {
            for(A = 1; A <= numWorldMusic; A++)
            {
                if(CheckCollision(WorldPlayer[1].Location, WorldMusic[A].Location))
                {
                    if(curWorldMusic != WorldMusic[A].Type)
                    {
                        StartMusic(WorldMusic[A].Type);
                    }
                }
            }
            for(A = 1; A <= numWorldLevels; A++)
            {
                if(CheckCollision(WorldPlayer[1].Location, WorldLevel[A].Location))
                {
                    curWorldLevel = A;
                    WorldPlayer[1].LevelName = WorldLevel[A].LevelName;
                    break;
                }
            }
            if(curWorldLevel > 0)
            {
                LevelPath(curWorldLevel, 5);
            }
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
        WorldPlayer[1].LevelName = "";

        bool altPressed = getKeyState(SDL_SCANCODE_LALT) == KEY_PRESSED ||
                          getKeyState(SDL_SCANCODE_RALT) == KEY_PRESSED;

        bool escPressed = getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED;
#ifdef __ANDROID__
        escPressed |= getKeyState(SDL_SCANCODE_AC_BACK) == KEY_PRESSED;
#endif

        bool pausePress = (Player[1].Controls.Start || escPressed) && !altPressed;

        for(A = 1; A <= numWorldLevels; A++)
        {
            if(CheckCollision(tempLocation, WorldLevel[A].Location))
            {
                WorldPlayer[1].LevelName = WorldLevel[A].LevelName;
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
            for(A = 1; A <= numWorldPaths; A++)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location) && WorldPath[A].Active)
                {
                    WorldPlayer[1].Move = 1;
                    break;
                }
            }
            for(A = 1; A <= numWorldLevels; A++)
            {
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, WorldLevel[A].Location) && WorldLevel[A].Active)
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
            for(A = 1; A <= numWorldPaths; A++)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location) && WorldPath[A].Active)
                {
                    WorldPlayer[1].Move = 2;
                    break;
                }
            }
            for(A = 1; A <= numWorldLevels; A++)
            {
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, WorldLevel[A].Location) && WorldLevel[A].Active)
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
            for(A = 1; A <= numWorldPaths; A++)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location) && WorldPath[A].Active)
                {
                    WorldPlayer[1].Move = 3;
                    break;
                }
            }
            for(A = 1; A <= numWorldLevels; A++)
            {
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, WorldLevel[A].Location) && WorldLevel[A].Active)
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
            for(A = 1; A <= numWorldPaths; A++)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location) && WorldPath[A].Active)
                {
                    WorldPlayer[1].Move = 4;
                    break;
                }
            }
            for(A = 1; A <= numWorldLevels; A++)
            {
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, WorldLevel[A].Location) && WorldLevel[A].Active)
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
            for(A = 1; A <= numWorldLevels; A++)
            {
                if(CheckCollision(tempLocation, WorldLevel[A].Location))
                {
                    if(int(WorldLevel[A].WarpX) != -1)
                        WorldPlayer[1].Location.X = WorldLevel[A].WarpX;
                    if(int(WorldLevel[A].WarpY) != -1)
                        WorldPlayer[1].Location.Y = WorldLevel[A].WarpY;
                    if(int(WorldLevel[A].WarpY) != -1 || int(WorldLevel[A].WarpX) != -1)
                    {
                        LevelBeatCode = 6;
                        for(B = 1; B <= numWorldLevels; B++)
                        {
                            if(CheckCollision(WorldPlayer[1].Location, WorldLevel[B].Location))
                            {
                                WorldLevel[B].Active = true;
                                curWorldLevel = B;
                            }
                        }
                    }

                    if(!WorldLevel[A].FileName.empty() && WorldLevel[A].FileName != ".lvl" && WorldLevel[A].FileName != ".lvlx")
                    {
                        addMissingLvlSuffix(WorldLevel[A].FileName);
                        if(Files::fileExists(SelectWorld[selWorld].WorldPath + WorldLevel[A].FileName))
                        {
                            StartWarp = WorldLevel[A].StartWarp;
                            StopMusic();
                            PlaySound(28);
                            SoundPause[26] = 200;
                            curWorldLevel = A;
                            LevelSelect = false;
                            GameThing();
                            ClearLevel();
                            PGE_Delay(1000);
                            std::string levelPath = SelectWorld[selWorld].WorldPath + WorldLevel[A].FileName;
                            if(!OpenLevel(levelPath))
                            {
                                MessageText = fmt::format_ne("ERROR: Can't open \"{0}\": file doesn't exist or corrupted.", WorldLevel[A].FileName);
                                PauseGame(1);
                                ErrorQuit = true;
                            }
                            break;
                        }
                    }
                    else if(int(WorldLevel[A].WarpX) != -1 || int(WorldLevel[A].WarpY) != -1)
                    {
                        StopMusic();
                        PlaySound(28);
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
        for(A = 1; A <= numWorldMusic; A++)
        {
            if(CheckCollision(WorldPlayer[1].Location, WorldMusic[A].Location))
            {
                if((curWorldMusic != WorldMusic[A].Type) ||
                   (WorldMusic[A].Type == CustomWorldMusicId() && curWorldMusicFile != WorldMusic[A].MusicFile))
                {
                    curWorldMusicFile = WorldMusic[A].MusicFile;
                    StartMusic(WorldMusic[A].Type);
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

void LevelPath(int Lvl, int Direction, bool Skp)
{
    Location_t tempLocation;
    int A = 0;

    if(Direction == 1 || Direction == 5)
    {
        tempLocation = WorldLevel[Lvl].Location;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.Y = tempLocation.Y - 32;
        for(A = 1; A <= numWorldPaths; A++)
        {
            if(WorldPath[A].Active == false)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location))
                {
                    PathPath(A, Skp);
                }
            }
        }
    }
    if(Direction == 2 || Direction == 5)
    {
        tempLocation = WorldLevel[Lvl].Location;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.X = tempLocation.X - 32;
        for(A = 1; A <= numWorldPaths; A++)
        {
            if(WorldPath[A].Active == false)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location))
                {
                    PathPath(A, Skp);
                }
            }
        }
    }
    if(Direction == 3 || Direction == 5)
    {
        tempLocation = WorldLevel[Lvl].Location;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.Y = tempLocation.Y + 32;
        for(A = 1; A <= numWorldPaths; A++)
        {
            if(WorldPath[A].Active == false)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location))
                {
                    PathPath(A, Skp);
                }
            }
        }
    }
    if(Direction == 4 || Direction == 5)
    {
        tempLocation = WorldLevel[Lvl].Location;
        tempLocation.X = tempLocation.X + 4;
        tempLocation.Y = tempLocation.Y + 4;
        tempLocation.Width = tempLocation.Width - 8;
        tempLocation.Height = tempLocation.Height - 8;
        tempLocation.X = tempLocation.X + 32;
        for(A = 1; A <= numWorldPaths; A++)
        {
            if(WorldPath[A].Active == false)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location))
                {
                    PathPath(A, Skp);
                }
            }
        }
    }
}

void PathPath(int Pth, bool Skp)
{
    int A = 0;
    int B = 0;
    Location_t tempLocation;
    tempLocation = WorldPath[Pth].Location;
    tempLocation.X = tempLocation.X + 4;
    tempLocation.Y = tempLocation.Y + 4;
    tempLocation.Width = tempLocation.Width - 8;
    tempLocation.Height = tempLocation.Height - 8;
    for(A = 1; A <= numScenes; A++)
    {
        if(Scene[A].Active)
        {
            if(CheckCollision(tempLocation, Scene[A].Location))
                Scene[A].Active = false;
        }
    }
    if(WorldPath[Pth].Active == false && Skp == false)
    {
        WorldPath[Pth].Active = true;
        vScreenX[1] = -(WorldPath[Pth].Location.X + WorldPath[Pth].Location.Width / 2.0) + vScreen[1].Width / 2.0;
        vScreenY[1] = -(WorldPath[Pth].Location.Y + WorldPath[Pth].Location.Height / 2.0) + vScreen[1].Height / 2.0;
        PlaySound(27);
        PathWait();
    }
    WorldPath[Pth].Active = true;
    for(B = 1; B <= 4; B++)
    {
        if(B == 1)
            tempLocation.Y = tempLocation.Y - 32;
        else if(B == 2)
        {
            tempLocation.Y = tempLocation.Y + 32;
            tempLocation.X = tempLocation.X - 32;
        }
        else if(B == 3)
        {
            tempLocation.X = tempLocation.X + 32;
            tempLocation.Y = tempLocation.Y + 32;
        }
        else if(B == 4)
        {
            tempLocation.Y = tempLocation.Y - 32;
            tempLocation.X = tempLocation.X + 32;
        }
        for(A = 1; A <= numWorldPaths; A++)
        {
            if(WorldPath[A].Active == false)
            {
                if(CheckCollision(tempLocation, WorldPath[A].Location))
                {
                    PathPath(A, Skp);
                }
            }
        }
        for(A = 1; A <= numWorldLevels; A++)
        {
            if(WorldLevel[A].Active == false)
            {
                if(CheckCollision(tempLocation, WorldLevel[A].Location))
                {
                    WorldLevel[A].Active = true;
                    if(Skp == false)
                    {
                        vScreenX[1] = -(WorldLevel[A].Location.X + WorldLevel[A].Location.Width / 2.0) + vScreen[1].Width / 2.0;
                        vScreenY[1] = -(WorldLevel[A].Location.Y + WorldLevel[A].Location.Height / 2.0) + vScreen[1].Height / 2.0;
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
