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

#include <Logger/logger.h>
#include <pge_delay.h>

#include "../globals.h"
#include "../game_main.h"
#include "../sound.h"
#include "../joystick.h"
#include "../effect.h"
#include "../graphics.h"
#include "../blocks.h"
#include "../npc.h"
#include "../layers.h"
#include "../player.h"
#include "../editor.h"

#include "../pseudo_vb.h"

void CheckActive();//in game_main.cpp

void GameLoop()
{
    UpdateControls();
    if(LevelMacro > 0)
        UpdateMacro();

    if(BattleMode)
    {
        if(BattleOutro > 0)
        {
            BattleOutro++;
            if(BattleOutro == 260)
                EndLevel = true;
        }
    }

    if(ErrorQuit)
    {
        EndLevel = true;
        ErrorQuit = false;
        pLogWarning("Quit level because of an error");
        frmMain.clearBuffer();
    }

    if(EndLevel)
    {
        if(LevelBeatCode > 0)
        {
            if(Checkpoint == FullFileName)
            {
                pLogDebug("Clear check-points at GameLoop()");
                Checkpoint.clear();
                CheckpointsList.clear();
            }
        }
        NextLevel();
        UpdateControls();
    }
    else if(qScreen)
    {
        UpdateEffects();
        UpdateGraphics();
    }
    else if(BattleIntro > 0)
    {
        UpdateGraphics();
        BlockFrames();
        UpdateSound();
        For(A, 1, numNPCs)
        {
            NPCFrames(A);
        }
        BattleIntro--;
        if(BattleIntro == 1)
            PlaySound(58);
    }
    else
    {
        UpdateLayers(); // layers before/after npcs
        UpdateNPCs();

        if(LevelMacro == 3)
            return; // stop on key exit

        UpdateBlocks();
        UpdateEffects();
        UpdatePlayer();
        if(LivingPlayers() || BattleMode)
            UpdateGraphics();
        UpdateSound();
        UpdateEvents();
//        If MagicHand = True Then UpdateEditor
        if(MagicHand)
            UpdateEditor();

        bool altPressed = getKeyState(SDL_SCANCODE_LALT) == KEY_PRESSED ||
                          getKeyState(SDL_SCANCODE_RALT) == KEY_PRESSED;

        bool escPressed = getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED;

        bool pausePress = (Player[1].Controls.Start || escPressed) && !altPressed;

        if(pausePress)
        {
            if(LevelMacro == 0 && CheckLiving() > 0)
            {
                if(Player[1].UnStart)
                {
                    if((CaptainN || FreezeNPCs) && PSwitchStop == 0)
                    {
                        if(escPressed)
                        {
                            FreezeNPCs = false;
                            PauseGame(1);
                        }
                        else
                        {
                            Player[1].UnStart = false;
                            if(FreezeNPCs)
                            {
                                FreezeNPCs = false;
                                if(PSwitchTime > 0)
                                {
                                    if(!noSound)
                                        SoundResumeAll();
                                }
                            }
                            else
                            {
                                FreezeNPCs = true;
                                if(PSwitchTime > 0)
                                {
                                    if(!noSound)
                                        SoundPauseAll();
                                }
                            }
                            PlaySound(30);
                        }
                    }
                    else
                    {
                        PauseGame(1);
                    }
                }
            }
        }
        else if(numPlayers == 2 && Player[2].Controls.Start)
        {
            if(LevelMacro == 0 && CheckLiving() > 0)
            {
                if(Player[2].UnStart)
                {
                    if(CaptainN || FreezeNPCs)
                    {
                        Player[2].UnStart = false;
                        if(FreezeNPCs)
                        {
                            FreezeNPCs = false;
                        }
                        else
                        {
                            FreezeNPCs = true;
                        }
                        PlaySound(30);
                    }
                }
            }
        }
    }
}

void PauseGame(int plr)
{
    bool stopPause = false;
    int A = 0;
    int B = 0;
    bool noButtons = false;
    double fpsTime = 0;
    int fpsCount = 0;

    for(A = numPlayers; A >= 1; A--)
        SavedChar[Player[A].Character] = Player[A];

//    if(TestLevel && MessageText.empty())
//        return;
    if(MessageText.empty())
        PlaySound(30);
    else
    {
        SoundPause[47] = 0;
        PlaySound(47);
    }

    GamePaused = true;
    MenuCursor = 0;
    MenuCursorCanMove = false;

    if(PSwitchTime > 0)
    {
        // If noSound = False Then mciSendString "pause smusic", 0, 0, 0
        if(noSound == false)
            SoundPauseAll();
    }

    overTime = 0;
    GoalTime = SDL_GetTicks() + 1000;
    fpsCount = 0;
    fpsTime = 0;
    cycleCount = 0;
    gameTime = 0;

    do
    {
        tempTime = SDL_GetTicks();
        if(tempTime >= gameTime + frameRate || tempTime < gameTime || MaxFPS)
        {
            if(fpsCount >= 32000) // Fixes Overflow bug
                fpsCount = 0;
            if(cycleCount >= 32000) // Fixes Overflow bug
                cycleCount = 0;
            overTime = overTime + (tempTime - (gameTime + frameRate));
            if(gameTime == 0.0)
                overTime = 0;
            if(overTime <= 1)
                overTime = 0;
            else if(overTime > 1000)
                overTime = 1000;
            gameTime = tempTime - overTime;
            overTime = (overTime - (tempTime - gameTime));
            if(SDL_GetTicks() > fpsTime)
            {
                if(cycleCount >= 65)
                {
                    overTime = 0;
                    gameTime = tempTime;
                }
                cycleCount = 0;
                fpsTime = SDL_GetTicks() + 1000;
                GoalTime = fpsTime;
//                if(Debugger == true)
//                    frmLevelDebugger.lblFPS = fpsCount;
                if(ShowFPS)
                    PrintFPS = fpsCount;
                fpsCount = 0;
            }


            DoEvents();
            CheckActive();

            if(LevelSelect == true)
                UpdateGraphics2();
            else
                UpdateGraphics();
            UpdateControls();
            UpdateSound();
            BlockFrames();
            UpdateEffects();

            bool altPressed = getKeyState(SDL_SCANCODE_LALT) == KEY_PRESSED ||
                              getKeyState(SDL_SCANCODE_RALT) == KEY_PRESSED;
            bool escPressed = getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED;
            bool spacePressed = getKeyState(SDL_SCANCODE_SPACE) == KEY_PRESSED;
            bool returnPressed = getKeyState(SDL_SCANCODE_RETURN) == KEY_PRESSED;
            bool upPressed = getKeyState(SDL_SCANCODE_UP) == KEY_PRESSED;
            bool downPressed = getKeyState(SDL_SCANCODE_DOWN) == KEY_PRESSED;

            bool menuDoPress = (returnPressed && !altPressed) || spacePressed;
            bool menuBackPress = (escPressed && !altPressed);

            if(SingleCoop > 0 || numPlayers > 2)
            {
                for(A = 1; A <= numPlayers; A++)
                    Player[A].Controls = Player[1].Controls;
            }

            auto &c = Player[plr].Controls;

            menuDoPress |= (c.Start || c.Jump) && !altPressed;
            menuBackPress |= c.Run && !altPressed;

            upPressed |= (c.Up && !altPressed);
            downPressed |= (c.Down && !altPressed);

            if(MessageText.empty())
            {
                // Pause menu
                if(!noButtons)
                {
                    if(!c.Down && !c.Up && !c.Run && !c.Jump && !c.Start &&
                       !menuDoPress && !menuBackPress && !upPressed && !downPressed)
                    {
                        noButtons = true;
                    }
                }
                else
                {
                    if(menuBackPress)
                    {
                        if(LevelSelect && !Cheater)
                        {
                            if(MenuCursor != 2)
                                PlaySound(26);
                            MenuCursor = 2;
                        }
                        else
                        {
                            if(MenuCursor != 1)
                                PlaySound(26);
                            MenuCursor = 1;
                        }
                        noButtons = false;
                    }
                    else if(menuDoPress)
                        stopPause = true;

                    if(upPressed)
                    {
                        PlaySound(26);
                        MenuCursor = MenuCursor - 1;
                        noButtons = false;
                    }
                    else if(downPressed)
                    {
                        PlaySound(26);
                        MenuCursor = MenuCursor + 1;
                        noButtons = false;
                    }

                    if(LevelSelect)
                    {
                        if(Player[A].Character == 1 || Player[A].Character == 2)
                            Player[A].Hearts = 0;
                        for(A = 1; A <= numPlayers; A++)
                        {
                            if(!Player[A].RunRelease)
                            {
                                if(!Player[A].Controls.Left && !Player[A].Controls.Right)
                                    Player[A].RunRelease = true;
                            }
                            else if(Player[A].Controls.Left || Player[A].Controls.Right)
                            {
                                AllCharBlock = 0;
                                for(B = 1; B <= numCharacters; B++)
                                {
                                    if(!blockCharacter[B])
                                    {
                                        if(AllCharBlock == 0)
                                            AllCharBlock = B;
                                        else
                                        {
                                            AllCharBlock = 0;
                                            break;
                                        }
                                    }
                                }
                                if(AllCharBlock == 0)
                                {
                                    PlaySound(26);
                                    Player[A].RunRelease = false;
                                    if(A == 1)
                                        B = 2;
                                    else
                                        B = 1;
                                    if(numPlayers == 1)
                                        B = 0;
                                    Player[0].Character = 0;
                                    if(Player[A].Controls.Left)
                                    {
                                        do
                                        {
                                            Player[A].Character = Player[A].Character - 1;
                                            if(Player[A].Character <= 0)
                                                Player[A].Character = 5;
                                        } while(Player[A].Character == Player[B].Character || blockCharacter[Player[A].Character]);
                                    }
                                    else
                                    {
                                        do
                                        {
                                            Player[A].Character = Player[A].Character + 1;
                                            if(Player[A].Character >= 6)
                                                Player[A].Character = 1;
                                        } while(Player[A].Character == Player[B].Character || blockCharacter[Player[A].Character]);
                                    }
                                    Player[A] = SavedChar[Player[A].Character];
                                    SetupPlayers();
                                }
                            }
                        }
                    }

                    if(menuDoPress)
                    {
                        if(TestLevel) // Pause menu of a level testing
                        {
                            switch(MenuCursor)
                            {
                            case 0: // Continue
                                stopPause = true;
                                break;
                            case 1: // Restart level
                                stopPause = true;
                                MenuMode = 0;
                                MenuCursor = 0;
                                frmMain.clearBuffer();
                                frmMain.repaint();
                                EndLevel = true;
                                StopMusic();
                                DoEvents();
                                break;
                            case 2: // Reset checkpoints
                                stopPause = true;
                                pLogDebug("Clear check-points from a menu");
                                Checkpoint.clear();
                                CheckpointsList.clear();
                                numStars = 0;
                                numSavedEvents = 0;
                                BlockSwitch.fill(false);
                                PlaySound(22);
                                break;
                            case 3: // Quit testing
                                stopPause = true;
                                MenuMode = 0;
                                MenuCursor = 0;
                                frmMain.clearBuffer();
                                frmMain.repaint();
                                EndLevel = true;
                                StopMusic();
                                DoEvents();
                                KillIt(); // Quit the game entirely
                                break;
                            default:
                                break;
                            }
                        }
                        else if(MenuCursor == 0) // Contunue
                        {
                            stopPause = true;
                        }
                        else if(MenuCursor == 1 && (LevelSelect || (/*StartLevel == FileName*/IsEpisodeIntro && NoMap)) && !Cheater) // "Save and continue"
                        {
                            SaveGame();
                            stopPause = true;
                        }
                        else // "Quit" or "Save & Quit"
                        {
                            if(!Cheater && (LevelSelect || (/*StartLevel == FileName*/IsEpisodeIntro && NoMap)))
                                SaveGame(); // "Save & Quit"
                            stopPause = true;
                            GameMenu = true;

                            MenuMode = 0;
                            MenuCursor = 0;
                            if(!LevelSelect)
                            {
                                LevelSelect = true;
                                EndLevel = true;
                            }
                            else
                                LevelSelect = false;
                            frmMain.clearBuffer();
                            frmMain.repaint();
                            StopMusic();
                            DoEvents();
                        }
                    }

                    if(TestLevel) // Level test pause menu (4 items)
                    {
                        if(MenuCursor > 3)
                            MenuCursor = 0;
                        if(MenuCursor < 0)
                            MenuCursor = 3;
                    }
                    else if(Cheater || !(LevelSelect || (/*StartLevel == FileName*/IsEpisodeIntro && NoMap))) // Level play menu (2 items)
                    {
                        if(MenuCursor > 1)
                            MenuCursor = 0;
                        if(MenuCursor < 0)
                            MenuCursor = 1;
                    }
                    else // World map or HUB (3 items)
                    {
                        if(MenuCursor > 2)
                            MenuCursor = 0;
                        if(MenuCursor < 0)
                            MenuCursor = 2;
                    }
                }
            }
            else // Message box
            {
                if(!noButtons)
                {
                    if(!c.Down && !c.Up && !c.Run && !c.Jump && !c.Start &&
                       !menuDoPress && !menuBackPress && !upPressed && !downPressed)
                    {
                        noButtons = true;
                    }
                }
                else
                {
                    if(menuBackPress || menuDoPress)
                    {
                        stopPause = true;
                    }
                }
            }
        }

        if(qScreen)
            stopPause = false;
        PGE_Delay(1);
        if(!GameIsActive)
            break;
    } while(!(stopPause == true));

    GamePaused = false;
    Player[plr].UnStart = false;
    Player[plr].CanJump = false;
    if(!TestLevel && MessageText.empty())
        PlaySound(30);
    if(PSwitchTime > 0)
    {
        // If noSound = False Then mciSendString "resume smusic", 0, 0, 0
        if(noSound == false)
            SoundResumeAll();
    }
    MessageText.clear();

    overTime = 0;
    GoalTime = SDL_GetTicks() + 1000;
    fpsCount = 0;
    cycleCount = 0;
    gameTime = 0;
    fpsTime = 0;
}

