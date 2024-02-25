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


#include <Logger/logger.h>
#include <Integrator/integrator.h>
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
#include "../core/window.h"
#include "graphics/gfx_update.h"
#include "game_globals.h"
#include "world_globals.h"
#include "speedrunner.h"
#include "main/record.h"
#include "menu_main.h"
#include "screen_pause.h"
#include "screen_connect.h"
#include "screen_quickreconnect.h"
#include "screen_textentry.h"
#include "screen_prompt.h"
#include "script/luna/luna.h"

#include "../pseudo_vb.h"

PauseCode GamePaused = PauseCode::None;

//! Holds the screen overlay for the level
ScreenFader g_levelScreenFader;
RangeArr<ScreenFader, 0, c_vScreenCount> g_levelVScreenFader;

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
            UpdateGraphicsDraw();
            UpdateSound();
            XEvents::doEvents();
            computeFrameTime2();
            updateScreenFaders();
        }

        if(!MaxFPS)
            PGE_Delay(1);
    }
}

void editorWaitForFade()
{
    while(!g_levelScreenFader.isComplete() && GameIsActive)
    {
        XEvents::doEvents();

        if(canProceedFrame())
        {
            computeFrameTime1();
            if(GamePaused == PauseCode::Prompt)
                PromptScreen::Render();
            else if(WorldEditor)
                UpdateGraphics2();
            else
                UpdateGraphics();
            UpdateSound();
            XEvents::doEvents();
            computeFrameTime2();
            updateScreenFaders();
        }

        if(!MaxFPS)
            PGE_Delay(1);
    }
}


void CheckActive();//in game_main.cpp

void GameLoop()
{
    g_microStats.start_task(MicroStats::Script);
    lunaLoop();

    g_microStats.start_task(MicroStats::Controls);

    if(!Controls::Update())
    {
        QuickReconnectScreen::g_active = true;

        if(!g_config.NoPauseReconnect && g_compatibility.pause_on_disconnect && !TestLevel)
            PauseGame(PauseCode::PauseScreen, 0);
    }

    if(QuickReconnectScreen::g_active)
        QuickReconnectScreen::Logic();

    Integrator::sync();

    g_microStats.start_task(MicroStats::Layers);

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
        LevelBeatCode = -1;
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

            // Quit to world map when finishing the sub-hub
            if(!NoMap && IsHubLevel && !FileRecentSubHubLevel.empty() && GoToLevel.empty())
            {
                FileRecentSubHubLevel.clear();
                ReturnWarp = 0;
                ReturnWarpSaved = 0;
            }
        }

        if(SwapCharAllowed())
        {
            pLogDebug("Save drop/add characters configuration at EndLevel");
            ConnectScreen::SaveChars();
        }

        speedRun_triggerLeave();
        NextLevel();

        Controls::Update(false);
    }
    else if(qScreen || (g_compatibility.allow_multires && qScreen_canonical))
    {
        g_microStats.start_task(MicroStats::Effects);
        UpdateEffects();
        if(!g_compatibility.modern_section_change)
            speedRun_tick();
        g_microStats.start_task(MicroStats::Graphics);
        UpdateGraphics();
        updateScreenFaders();
    }
    else if(BattleIntro > 0)
    {
        g_microStats.start_task(MicroStats::Graphics);
        UpdateGraphics();
        BlockFrames();
        g_microStats.start_task(MicroStats::Sound);
        UpdateSound();
        g_microStats.start_task(MicroStats::NPCs);
        For(A, 1, numNPCs)
            NPCFrames(A);
        BattleIntro--;
        if(BattleIntro == 1)
            PlaySound(SFX_Checkpoint);
        updateScreenFaders();
    }
    else
    {
        if(MagicHand)
            UpdateEditor();

        ClearTriggeredEvents();
        UpdateLayers(); // layers before/after npcs

        g_microStats.start_task(MicroStats::NPCs);
        UpdateNPCs();

        if(LevelMacro == LEVELMACRO_KEYHOLE_EXIT)
            return; // stop on key exit

        g_microStats.start_task(MicroStats::Blocks);
        UpdateBlocks();
        g_microStats.start_task(MicroStats::Effects);
        UpdateEffects();
        g_microStats.start_task(MicroStats::Player);
        UpdatePlayer();
        speedRun_tick();
        g_microStats.start_task(MicroStats::Graphics);
        if(LivingPlayers() || BattleMode)
            UpdateGraphics();
        g_microStats.start_task(MicroStats::Sound);
        UpdateSound();
        g_microStats.start_task(MicroStats::Events);
        UpdateEvents();
//        If MagicHand = True Then UpdateEditor

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
                            if(PSwitchTime > 0)
                                ResumeMusic();
                        }
                        else
                        {
                            FreezeNPCs = true;
                            if(PSwitchTime > 0)
                                PauseMusic();
                        }
                        PlaySound(SFX_Pause);
                    }
                    // normally pause the game
                    else
                    {
                        PauseGame(PauseCode::PauseScreen, 0);
                    }

                    // don't let double-pause or double-toggle happen
                    break;
                }
            }
        }
    }

    g_microStats.end_frame();
}

void MessageScreen_Init()
{
    SoundPause[SFX_Message] = 0;
    PlaySound(SFX_Message);
    MenuCursorCanMove = false;
    BuildUTF8CharMap(MessageText, MessageTextMap);
}

bool MessageScreen_Logic(int plr)
{
    bool menuDoPress = SharedControls.MenuDo || SharedControls.Pause;
    bool menuBackPress = SharedControls.MenuBack;

    // there was previously code to copy all players' controls from the main player, but this is no longer necessary (and actively harmful in the SingleCoop case)

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
            MenuCursorCanMove = true;

        return false;
    }

    if(MenuCursorCanMove && (menuDoPress || menuBackPress))
    {
        MessageText.clear();
        MessageTextMap.clear();
        return true;
    }

    return false;
}

int PauseGame(PauseCode code, int plr)
{
//    double fpsTime = 0;
//    int fpsCount = 0;

    int prev_cursor = XWindow::showCursor(-1);

    if(!GameMenu && !LevelEditor)
    {
        for(int A = numPlayers; A >= 1; A--)
            SavedChar[Player[A].Character] = Player[A];
    }

    if(code == PauseCode::Message)
        MessageScreen_Init();
    else if(code == PauseCode::PauseScreen)
        PauseScreen::Init(plr, SharedControls.LegacyPause);
    else if(code == PauseCode::DropAdd)
    {
        ConnectScreen::DropAdd_Start();
        XWindow::showCursor(0);
    }
    else if(code == PauseCode::Prompt)
    {
        PromptScreen::Init();
    }
    else if(code == PauseCode::TextEntry)
    {
        // assume TextEntryScreen has already been inited through its Run function.
    }

    PauseCode old_code = GamePaused;
    GamePaused = code;

    if(PSwitchTime > 0)
        PauseMusic();

    // resetFrameTimer();

    // some pause games may return a status code
    int result = 0;

    do
    {
        if(canProceedFrame())
        {
            computeFrameTime1();
            computeFrameTime2();

            g_microStats.start_task(MicroStats::Controls);

            XEvents::doEvents();
            CheckActive();

            g_microStats.start_task(MicroStats::Graphics);

            speedRun_tick();

            if(code == PauseCode::Prompt)
                PromptScreen::Render();
            else if((LevelSelect && !GameMenu) || WorldEditor)
                UpdateGraphics2();
            else
                UpdateGraphics();

            g_microStats.start_task(MicroStats::Controls);

            if(!Controls::Update())
            {
                QuickReconnectScreen::g_active = true;
            }

            if(QuickReconnectScreen::g_active)
                QuickReconnectScreen::Logic();

            g_microStats.start_task(MicroStats::Sound);

            UpdateSound();
            BlockFrames();

            g_microStats.start_task(MicroStats::Effects);

            UpdateEffects();

            if(LevelSelect)
                g_worldScreenFader.update();
            else
                updateScreenFaders();

            // reset the active player if it is no longer present
            if(plr > numPlayers)
                plr = 0;

            g_microStats.start_task(MicroStats::Script);

            // run the appropriate pause logic
            if(qScreen)
            {
                // prevent any logic or unpause from taking place

                // qScreen takes place in WorldLoop, not world graphics
                if(LevelSelect)
                    qScreen = Update_qScreen(1);
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
            else if(GamePaused == PauseCode::Prompt)
            {
                if(PromptScreen::Logic())
                    break;
            }
            else if(GamePaused == PauseCode::DropAdd)
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

            g_microStats.end_frame();
        }

        if(!MaxFPS)
            PGE_Delay(1);

        if(!GameIsActive)
            break;
    } while(true);

    GamePaused = old_code;

    // prevent unexpected button presses
    for(int i = 1; i <= numPlayers; i++)
    {
        Player[i].UnStart = false;
        Player[i].CanJump = false;
    }

    MenuCursorCanMove = false;

    XWindow::showCursor(prev_cursor);

    if(PSwitchTime > 0)
        ResumeMusic();

    // resetFrameTimer();

    return result;
}
