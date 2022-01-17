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

#include "../globals.h"
#include "../config.h"
#include "../compat.h"
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
#include "../core/render.h"
#include "../core/events.h"
#include "game_globals.h"
#include "world_globals.h"
#include "speedrunner.h"
#include "menu_main.h"
#include "screen_pause.h"
#include "screen_connect.h"
#include "screen_quickreconnect.h"
#include "screen_textentry.h"
#include "../pseudo_vb.h"

PauseCode GamePaused = PauseCode::None;

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
    if(!Controls::Update())
    {
        if(g_config.NoPauseReconnect || !g_compatibility.pause_on_disconnect)
            QuickReconnectScreen::g_active = true;
        else
            PauseGame(PauseCode::Reconnect, 0);
    }

    if(QuickReconnectScreen::g_active)
        QuickReconnectScreen::Logic();

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
        // Controls::Update();
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

        // Pause game and CaptainN logic
        if(LevelMacro == LEVELMACRO_OFF && CheckLiving() > 0)
        {
            // this is always able to pause the game even when CaptainN is enabled.
            if(SharedControls.Pause)
                PauseGame(PauseCode::PauseScreen, 0);
            // don't let double-pause or double-toggle happen
            else
            {
                for(int p = 1; p <= numPlayers && p <= maxLocalPlayers; p++)
                {
                    // only consider new start presses
                    if(!Player[p].UnStart || !Player[p].Controls.Start)
                        continue;
                    // use limited, buggy code for non-player 1 in compat case
                    if(p != 1 && !g_compatibility.multiplayer_pause_controls)
                    {
                        if(CaptainN || FreezeNPCs)
                        {
                            Player[p].UnStart = false;
                            FreezeNPCs = !FreezeNPCs;
                            PlaySound(SFX_Pause);
                        }
                        // don't let double-pause or double-toggle happen
                        break;
                    }
                    // the special NPC freeze toggling functionality from CaptainN
                    if((CaptainN || FreezeNPCs) && PSwitchStop == 0)
                    {
                        Player[p].UnStart = false;
                        if(FreezeNPCs)
                        {
                            FreezeNPCs = false;
                            if(PSwitchTime > 0 && !noSound)
                                SoundResumeAll();
                        }
                        else
                        {
                            FreezeNPCs = true;
                            if(PSwitchTime > 0 && !noSound)
                                SoundPauseAll();
                        }
                        PlaySound(SFX_Pause);
                    }
                    // normally pause the game
                    else
                    {
                        PauseGame(PauseCode::PauseScreen, p);
                    }
                    // don't let double-pause or double-toggle happen
                    break;
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

bool MessageScreen_Logic(int plr)
{
    bool menuDoPress = SharedControls.MenuDo || SharedControls.Pause;
    bool menuBackPress = SharedControls.MenuBack;

    // this might no longer be necessary...
    if(SingleCoop > 0 || numPlayers > 2)
    {
        for(int A = 1; A <= numPlayers; A++)
            Player[A].Controls = Player[1].Controls;
    }

    if(!g_compatibility.multiplayer_pause_controls && plr == 0)
        plr = 1;

    if(plr == 0)
    {
        for(int i = 1; i <= numPlayers; i++)
        {
            const Controls_t& c = Player[i].Controls;

            menuDoPress |= (c.Start || c.Jump);
            menuBackPress |= c.Run;
        }
    }
    else
    {
        const Controls_t& c = Player[plr].Controls;

        menuDoPress |= (c.Start || c.Jump);
        menuBackPress |= c.Run;
    }

    if(!MenuCursorCanMove)
    {
        if(!menuDoPress && !menuBackPress)
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

int PauseGame(PauseCode code, int plr)
{
//    double fpsTime = 0;
//    int fpsCount = 0;

    // no reason to allow game to be paused during main menu or credits
    if(GameMenu || GameOutro)
        return 0;

    if(!GameMenu)
    {
        for(int A = numPlayers; A >= 1; A--)
            SavedChar[Player[A].Character] = Player[A];
    }

    if(code == PauseCode::Message)
        MessageScreen_Init();
    else if(code == PauseCode::PauseScreen)
        PauseScreen::Init(SharedControls.LegacyPause);
    else if(code == PauseCode::Reconnect)
        ConnectScreen::Reconnect_Start();
    else if(code == PauseCode::DropAdd)
        ConnectScreen::DropAdd_Start();
    else if(code == PauseCode::TextEntry)
    {
        // assume TextEntryScreen has already been inited through its Run function.
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

    // some pause games may return a status code
    int result = 0;

    do
    {
        if(canProceedFrame())
        {
            computeFrameTime1();
            computeFrameTime2();

            XEvents::doEvents();
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
                    if(g_config.NoPauseReconnect || !g_compatibility.pause_on_disconnect)
                        QuickReconnectScreen::g_active = true;
                    else
                        PauseGame(PauseCode::Reconnect, 0);
                }
            }
            UpdateSound();
            BlockFrames();
            UpdateEffects();

            if(LevelSelect)
                g_worldScreenFader.update();
            else
                updateScreenFaders();

            if(qScreen)
            {
                // prevent any logic or unpause from taking place
            }
            else if(GamePaused == PauseCode::PauseScreen)
            {
                if(PauseScreen::Logic(plr))
                    break;
            }
            else if(GamePaused == PauseCode::Message)
            {
                if(MessageScreen_Logic(plr))
                    break;
            }
            else if(GamePaused == PauseCode::Reconnect || GamePaused == PauseCode::DropAdd)
            {
                result = ConnectScreen::Logic();
                if(result)
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

    return result;
}
