/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL_timer.h>

#include <Logger/logger.h>
#include <pge_delay.h>
#include <InterProcess/intproc.h>

#include "../globals.h"
#include "../config.h"
#include "../frame_timer.h"
#include "../game_main.h"
#include "../sound.h"
#include "../control/joystick.h"
#include "../effect.h"
#include "../graphics.h"
#include "../blocks.h"
#include "../npc.h"
#include "../layers.h"
#include "../player.h"
#include "../editor.h"
#include "../core/render.h"
#include "../core/events.h"
#include "game_globals.h"
#include "world_globals.h"
#include "speedrunner.h"
#include "menu_main.h"
#include "script/luna/luna.h"

#include "../pseudo_vb.h"

//! Holds the screen overlay for the level
ScreenFader g_levelScreenFader;
RangeArr<ScreenFader, 0, 2> g_levelVScreenFader;

void clearScreenFaders()
{
    g_levelScreenFader.clearFader();
    for(int s = 0; s < 3; ++s)
        g_levelVScreenFader[s].clearFader();
}

void updateScreenFaders()
{
    g_levelScreenFader.update();

    for(int s = 0; s < 3; ++s)
        g_levelVScreenFader[s].update();
}

void levelWaitForFade()
{
    while(!g_levelScreenFader.isComplete() && GameIsActive)
    {
        XEvents::doEvents();

        if(canProceedFrame())
        {
            computeFrameTime1();
            UpdateGraphics();
            UpdateSound();
            XEvents::doEvents();
            computeFrameTime2();
            updateScreenFaders();
        }
        PGE_Delay(1);
    }
}


void CheckActive();//in game_main.cpp

void GameLoop()
{
    lunaLoop();

    UpdateControls();
    if(LevelMacro > LEVELMACRO_OFF)
        UpdateMacro();

    if(BattleMode)
    {
        if(BattleOutro > 0)
        {
            BattleOutro++;

            if(g_config.EnableInterLevelFade && BattleOutro == 195)
                g_levelScreenFader.setupFader(1, 0, 65, ScreenFader::S_FADE);

            if(BattleOutro == 260)
                EndLevel = true;
        }
    }

    if(ErrorQuit)
    {
        EndLevel = true;
        ErrorQuit = false;
        pLogWarning("Quit level because of an error");
        XRender::clearBuffer();
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

        speedRun_triggerLeave();
        NextLevel();
        UpdateControls();
    }
    else if(qScreen)
    {
        UpdateEffects();
        speedRun_tick();
        UpdateGraphics();
        updateScreenFaders();
    }
    else if(BattleIntro > 0)
    {
        UpdateGraphics();
        BlockFrames();
        UpdateSound();
        For(A, 1, numNPCs)
            NPCFrames(A);
        BattleIntro--;
        if(BattleIntro == 1)
            PlaySound(SFX_Checkpoint);
        updateScreenFaders();
    }
    else
    {
        ClearTriggeredEvents();
        UpdateLayers(); // layers before/after npcs
        UpdateNPCs();

        if(LevelMacro == LEVELMACRO_KEYHOLE_EXIT)
            return; // stop on key exit

        UpdateBlocks();
        UpdateEffects();
        UpdatePlayer();
        speedRun_tick();
        if(LivingPlayers() || BattleMode)
            UpdateGraphics();
        UpdateSound();
        UpdateEvents();
//        If MagicHand = True Then UpdateEditor
        if(MagicHand)
            UpdateEditor();

        updateScreenFaders();

        bool altPressed = XEvents::getKeyState(SDL_SCANCODE_LALT) ||
                          XEvents::getKeyState(SDL_SCANCODE_RALT);

        bool escPressed = XEvents::getKeyState(SDL_SCANCODE_ESCAPE);
#ifdef __ANDROID__
        escPressed |= XEvents::getKeyState(SDL_SCANCODE_AC_BACK);
#endif

        bool pausePress = (Player[1].Controls.Start || escPressed) && !altPressed;

        if(pausePress)
        {
            if(LevelMacro == LEVELMACRO_OFF && CheckLiving() > 0)
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
                                    SoundResumeAll();
                            }
                            else
                            {
                                FreezeNPCs = true;
                                if(PSwitchTime > 0)
                                    SoundPauseAll();
                            }
                            PlaySound(SFX_Pause);
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
            if(LevelMacro == LEVELMACRO_OFF && CheckLiving() > 0)
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
                        PlaySound(SFX_Pause);
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
    bool noButtons = false, quitNoSound = false;
//    double fpsTime = 0;
//    int fpsCount = 0;

    for(A = numPlayers; A >= 1; A--)
        SavedChar[Player[A].Character] = Player[A];

//    if(TestLevel && MessageText.empty())
//        return;
    if(MessageText.empty())
        PlaySound(SFX_Pause);
    else
    {
        SoundPause[SFX_Message] = 0;
        PlaySound(SFX_Message);
    }

    GamePaused = true;
    MenuCursor = 0;
    MenuCursorCanMove = false;

    if(PSwitchTime > 0)
    {
        // If noSound = False Then mciSendString "pause smusic", 0, 0, 0
        if(!noSound)
            SoundPauseAll();
    }

    resetFrameTimer();

    do
    {
        if(canProceedFrame())
        {
            computeFrameTime1();
            computeFrameTime2();

            XEvents::doEvents();
            CheckActive();

            speedRun_tick();
            if(LevelSelect)
                UpdateGraphics2();
            else
                UpdateGraphics();
            UpdateControls();
            UpdateSound();
            BlockFrames();
            UpdateEffects();

            if(LevelSelect)
                g_worldScreenFader.update();
            else
                updateScreenFaders();

            bool altPressed = XEvents::getKeyState(SDL_SCANCODE_LALT) ||
                              XEvents::getKeyState(SDL_SCANCODE_RALT);
            bool escPressed = XEvents::getKeyState(SDL_SCANCODE_ESCAPE);
            bool spacePressed = XEvents::getKeyState(SDL_SCANCODE_SPACE);
            bool returnPressed = XEvents::getKeyState(SDL_SCANCODE_RETURN);
            bool upPressed = XEvents::getKeyState(SDL_SCANCODE_UP);
            bool downPressed = XEvents::getKeyState(SDL_SCANCODE_DOWN);

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
                                PlaySound(SFX_Slide);
                            MenuCursor = 2;
                        }
                        else
                        {
                            if(MenuCursor != 1)
                                PlaySound(SFX_Slide);
                            if(TestLevel)
                                MenuCursor = 3;
                            else
                                MenuCursor = 1;
                        }
                        noButtons = false;
                    }
                    else if(menuDoPress)
                        stopPause = true;

                    if(upPressed)
                    {
                        PlaySound(SFX_Slide);
                        MenuCursor -= 1;
                        noButtons = false;
                    }
                    else if(downPressed)
                    {
                        PlaySound(SFX_Slide);
                        MenuCursor += 1;
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
                                    PlaySound(SFX_Slide);
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
                                            Player[A].Character -= 1;
                                            if(Player[A].Character <= 0)
                                                Player[A].Character = 5;
                                        } while(Player[A].Character == Player[B].Character || blockCharacter[Player[A].Character]);
                                    }
                                    else
                                    {
                                        do
                                        {
                                            Player[A].Character += 1;
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
                                MenuMode = MENU_MAIN;
                                MenuCursor = 0;
                                XRender::setTargetTexture();
                                XRender::clearBuffer();
                                XRender::repaint();
                                EndLevel = true;
                                StopMusic();
                                XEvents::doEvents();
                                break;
                            case 2: // Reset checkpoints
                                stopPause = true;
                                pLogDebug("Clear check-points from a menu");
                                Checkpoint.clear();
                                CheckpointsList.clear();
                                numStars = 0;
#ifdef THEXTECH_INTERPROC_SUPPORTED
                                IntProc::sendStarsNumber(numStars);
#endif // THEXTECH_INTERPROC_SUPPORTED
                                numSavedEvents = 0;
                                BlockSwitch.fill(false);
                                PlaySound(SFX_Bullet);
                                break;
                            case 3: // Quit testing
                                stopPause = true;
                                MenuMode = MENU_MAIN;
                                MenuCursor = 0;
                                XRender::setTargetTexture();
                                XRender::clearBuffer();
                                XRender::repaint();
                                EndLevel = true;
                                StopMusic();
                                XEvents::doEvents();
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
                            PlaySound(SFX_Checkpoint);
                            quitNoSound = true;
                            stopPause = true;
                        }
                        else // "Quit" or "Save & Quit"
                        {
                            if(!Cheater && (LevelSelect || (/*StartLevel == FileName*/IsEpisodeIntro && NoMap)))
                                SaveGame(); // "Save & Quit"
                            else
                                speedRun_saveStats();
                            stopPause = true;
                            GameMenu = true;

                            MenuMode = MENU_MAIN;
                            MenuCursor = 0;

                            if(!LevelSelect)
                            {
                                LevelSelect = true;
                                EndLevel = true;
                            }
                            else
                                LevelSelect = false;

                            XRender::setTargetTexture();
                            XRender::clearBuffer();
                            XRender::repaint();
                            StopMusic();
                            XEvents::doEvents();
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
    } while(!stopPause);

    GamePaused = false;
    Player[plr].UnStart = false;
    Player[plr].CanJump = false;

    if(!TestLevel && MessageText.empty() && !quitNoSound)
        PlaySound(SFX_Pause);

    if(PSwitchTime > 0)
    {
        // If noSound = False Then mciSendString "resume smusic", 0, 0, 0
        if(!noSound)
            SoundResumeAll();
    }
    MessageText.clear();

    resetFrameTimer();
}

