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

#include <SDL2/SDL_timer.h>

#include <Logger/logger.h>
#include <pge_delay.h>
#include <InterProcess/intproc.h>

#include "../globals.h"
#include "../frame_timer.h"
#include "../game_main.h"
#include "../sound.h"
#include "../controls.h"
#include "../effect.h"
#include "../graphics.h"
#include "../blocks.h"
#include "../npc.h"
#include "../layers.h"
#include "../player.h"
#include "../editor.h"
#include "../compat.h"
#include "speedrunner.h"
#include "menu_main.h"
#include "menu_connectscreen.h"
#include "screen_textentry.h"
#include "../pseudo_vb.h"

void CheckActive();//in game_main.cpp

void GameLoop()
{
    if(!Controls::Update())
        PauseGame(PauseCode::Reconnect, 0);
    if(LevelMacro > LEVELMACRO_OFF)
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

        speedRun_triggerLeave();
        NextLevel();
        // Controls::Update();
    }
    else if(qScreen)
    {
        UpdateEffects();
        speedRun_tick();
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
            PlaySound(SFX_Checkpoint);
    }
    else
    {
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

        bool pausePress = Player[1].Controls.Start || SharedControls.Pause;

        if(pausePress)
        {
            if(LevelMacro == LEVELMACRO_OFF && CheckLiving() > 0)
            {
                if(Player[1].UnStart)
                {
                    if((CaptainN || FreezeNPCs) && PSwitchStop == 0)
                    {
                        // not sure I understand why this distinction is here
                        if(SharedControls.Pause) // if(escPressed)
                        {
                            FreezeNPCs = false;
                            PauseGame(PauseCode::PauseGame, 1);
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
                            PlaySound(SFX_Pause);
                        }
                    }
                    else
                    {
                        PauseGame(PauseCode::PauseGame, 1);
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
                    // not sure I understand why the PSwitchStop is missing here
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

void MessageScreen_Init()
{
    SoundPause[SFX_Message] = 0;
    PlaySound(SFX_Message);
    MenuCursorCanMove = false;
}

void PauseScreen_Init()
{
    PlaySound(SFX_Pause);
    MenuCursor = 0;
    MenuCursorCanMove = false;
}

bool PauseScreen_Logic(int plr)
{
    bool upPressed = SharedControls.MenuUp;
    bool downPressed = SharedControls.MenuDown;

    bool menuDoPress = SharedControls.MenuDo;
    bool menuBackPress = SharedControls.MenuBack;

    if(SingleCoop > 0 || numPlayers > 2)
    {
        for(int A = 1; A <= numPlayers; A++)
            Player[A].Controls = Player[1].Controls;
    }

    auto &c = Player[plr].Controls;

    menuDoPress |= (c.Start || c.Jump);
    menuBackPress |= c.Run;

    upPressed |= c.Up;
    downPressed |= c.Down;

    if(menuBackPress && menuDoPress)
        menuDoPress = false;

    if(!MenuCursorCanMove)
    {
        if(!c.Down && !c.Up && !c.Run && !c.Jump && !c.Start &&
           !menuDoPress && !menuBackPress && !upPressed && !downPressed)
        {
            MenuCursorCanMove = true;
        }
        return false;
    }

    bool stopPause = false;
    bool playSound = true;
    bool CanSave = (LevelSelect || (/*StartLevel == FileName*/IsEpisodeIntro && NoMap)) && !Cheater;
    int max_item;
    if(TestLevel) // Level test pause menu (5 items)
        max_item = 3;
    else if(!CanSave) // Level play menu (2 items)
        max_item = 1;
    else
        max_item = 2;
    if(menuBackPress)
    {
        if(MenuCursor != max_item)
            PlaySound(SFX_Slide);
        MenuCursor = max_item;
        MenuCursorCanMove = false;
    }
    else if(menuDoPress)
        stopPause = true;

    if(upPressed)
    {
        PlaySound(SFX_Slide);
        MenuCursor = MenuCursor - 1;
        MenuCursorCanMove = false;
    }
    else if(downPressed)
    {
        PlaySound(SFX_Slide);
        MenuCursor = MenuCursor + 1;
        MenuCursorCanMove = false;
    }

    if(LevelSelect)
    {
        // unclear meaning given that A was numPlayers+1 after the above for loop
        // if(Player[A].Character == 1 || Player[A].Character == 2)
        //     Player[A].Hearts = 0;
        for(int A = 1; A <= numPlayers; A++)
        {
            if(!Player[A].RunRelease)
            {
                if(!Player[A].Controls.Left && !Player[A].Controls.Right)
                    Player[A].RunRelease = true;
            }
            else if(Player[A].Controls.Left || Player[A].Controls.Right)
            {
                AllCharBlock = 0;
                for(int B = 1; B <= numCharacters; B++)
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
                if(AllCharBlock == 0 && numPlayers <= 2)
                {
                    PlaySound(SFX_Slide);
                    Player[A].RunRelease = false;
                    int B;
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
            // spaghetti code because I wanted to preserve the switch
            //   this means that MenuCursor will be 3 whenever it is Quit (really 3/4)
            //   and 4 whenever it is Drop/Add (really 3)
            if(g_compatibility.allow_DropAdd && MenuCursor >= 3)
            {
                MenuCursor = 7 - MenuCursor;
            }
            switch(MenuCursor)
            {
            case 0: // Continue
                stopPause = true;
                break;
            case 1: // Restart level
                stopPause = true;
                playSound = false;
                MenuMode = MENU_MAIN;
                MenuCursor = 0;
                frmMain.setTargetTexture();
                frmMain.clearBuffer();
                frmMain.repaint();
                EndLevel = true;
                StopMusic();
                DoEvents();
                break;
            case 2: // Reset checkpoints
                stopPause = true;
                playSound = false;
                pLogDebug("Clear check-points from a menu");
                Checkpoint.clear();
                CheckpointsList.clear();
                numStars = 0;
                IntProc::sendStarsNumber(numStars);
                numSavedEvents = 0;
                BlockSwitch.fill(false);
                PlaySound(SFX_Bullet);
                break;
            case 4: // Drop/Add (if exists)
                PauseGame(PauseCode::DropAdd, 0);
                playSound = false;
                break;
            case 3: // Quit testing
                stopPause = true;
                playSound = false;
                MenuMode = MENU_MAIN;
                MenuCursor = 0;
                frmMain.setTargetTexture();
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
            if(g_compatibility.allow_DropAdd && MenuCursor >= 3)
            {
                MenuCursor = 7 - MenuCursor;
            }
        }
        else if(MenuCursor == 0) // Contunue
        {
            stopPause = true;
        }
        else if(MenuCursor == 1 && g_compatibility.allow_DropAdd) // Drop/Add
        {
            PauseGame(PauseCode::DropAdd, 0);
            playSound = false;
        }
        else if(CanSave
            && ((MenuCursor == 1 && !g_compatibility.allow_DropAdd)
                || (MenuCursor == 2 && g_compatibility.allow_DropAdd))) // "Save and continue"
        {
            SaveGame();
            PlaySound(SFX_Checkpoint);
            playSound = false;
            stopPause = true;
        }
        else // "Quit" or "Save & Quit"
        {
            if(CanSave)
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

            frmMain.setTargetTexture();
            frmMain.clearBuffer();
            frmMain.repaint();
            StopMusic();
            DoEvents();
        }
    }

    if(g_compatibility.allow_DropAdd)
        max_item += 1;

    if(MenuCursor < 0)
        MenuCursor = max_item;
    else if(MenuCursor > max_item)
        MenuCursor = 0;

    if(stopPause && playSound)
    {
        PlaySound(SFX_Pause);
    }

    return stopPause;
}

bool MessageScreen_Logic(int plr)
{
    bool menuDoPress = SharedControls.MenuDo;
    bool menuBackPress = SharedControls.MenuBack;

    if(SingleCoop > 0 || numPlayers > 2)
    {
        for(int A = 1; A <= numPlayers; A++)
            Player[A].Controls = Player[1].Controls;
    }

    auto &c = Player[plr].Controls;

    menuDoPress |= (c.Start || c.Jump);
    menuBackPress |= c.Run;

    if(!MenuCursorCanMove)
    {
        if(!c.Run && !c.Jump && !c.Start &&
           !menuDoPress && !menuBackPress)
        {
            MenuCursorCanMove = true;
        }
        return false;
    }

    if(MenuCursorCanMove && (menuDoPress || menuBackPress))
    {
        MessageText.clear();
        return true;
    }

    return false;
}

void PauseGame(PauseCode code, int plr)
{
//    double fpsTime = 0;
//    int fpsCount = 0;

    if(!GameMenu)
    {
        for(int A = numPlayers; A >= 1; A--)
            SavedChar[Player[A].Character] = Player[A];
    }

    if(code == PauseCode::Message)
        MessageScreen_Init();
    else if(code == PauseCode::PauseGame)
        PauseScreen_Init();
    else if(code == PauseCode::Reconnect)
        ConnectScreen::Reconnect_Start();
    else if(code == PauseCode::DropAdd)
        ConnectScreen::DropAdd_Start();
    else if(code == PauseCode::TextEntry)
    {
        // assume TextEntryScreen::Init has already been called.
    }

    PauseCode old_code = GamePaused;
    GamePaused = code;

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

            DoEvents();
            CheckActive();

            speedRun_tick();
            if(LevelSelect && !GameMenu)
                UpdateGraphics2();
            else
                UpdateGraphics();
            if(!Controls::Update())
            {
                if(code != PauseCode::Reconnect)
                {
                    PauseGame(PauseCode::Reconnect, 0);
                }
            }
            UpdateSound();
            BlockFrames();
            UpdateEffects();

            if(qScreen)
            {
                // prevent any logic or unpause from taking place
            }
            if(GamePaused == PauseCode::PauseGame)
            {
                if(PauseScreen_Logic(plr))
                    break;
            }
            else if(GamePaused == PauseCode::Message)
            {
                if(MessageScreen_Logic(plr))
                    break;
            }
            else if(GamePaused == PauseCode::Reconnect || GamePaused == PauseCode::DropAdd)
            {
                if(ConnectScreen::Logic())
                    break;
            }
            else if(GamePaused == PauseCode::TextEntry)
            {
                if(TextEntryScreen::Logic())
                    break;
            }
        }

        PGE_Delay(1);
        if(!GameIsActive)
            break;
    } while(true);

    GamePaused = old_code;
    for(int i = 1; i <= numPlayers; i++)
    {
        Player[i].UnStart = false;
        Player[i].CanJump = false;
    }

    if(PSwitchTime > 0)
    {
        // If noSound = False Then mciSendString "resume smusic", 0, 0, 0
        if(!noSound)
            SoundResumeAll();
    }

    resetFrameTimer();
}
