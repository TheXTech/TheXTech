/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#if defined(THEXTECH_ASSERTS_INGAME_MESSAGE) && !defined(THEXTECH_NO_SDL_BUILD)
#   ifdef __WIIU__
#       include <sysapp/launch.h>
#   endif
#   include "frm_main.h"
#endif

#if defined(__16M__) && !defined(__CALICO__)
#include "core/16m/sound_stream_16m.h"
#endif

#include "../globals.h"
#include "../config.h"
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
#include "change_res.h"
#include "message.h"
#include "screen_pause.h"
#include "screen_connect.h"
#include "screen_options.h"
#include "screen_quickreconnect.h"
#include "screen_textentry.h"
#include "screen_prompt.h"
#include "main/level_medals.h"
#include "main/game_loop_interrupt.h"
#include "script/luna/luna.h"
#include "game_strings.h"

#include "../pseudo_vb.h"

PauseCode GamePaused = PauseCode::None;

//! Holds the screen overlay for the level
ScreenFader g_levelScreenFader;
RangeArr<ScreenFader, 0, c_vScreenCount> g_levelVScreenFader;

void clearScreenFaders()
{
    g_levelScreenFader.clearFader();
    for(int s = 0; s <= c_vScreenCount; ++s)
        g_levelVScreenFader[s].clearFader();
}

void updateScreenFaders()
{
    g_levelScreenFader.update();

    for(int s = 0; s <= c_vScreenCount; ++s)
        g_levelVScreenFader[s].update();
}

#if 0
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

        if(!g_config.unlimited_framerate)
            PGE_Delay(1);
    }
}
#endif

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

        if(!g_config.unlimited_framerate)
            PGE_Delay(1);
    }
}


void CheckActive();//in game_main.cpp

GameLoopInterrupt g_gameLoopInterrupt;

void GameLoop()
{
    if(GamePaused != PauseCode::None)
    {
        PauseLoop();

        // resume code (if needed)
        if(GamePaused == PauseCode::None)
        {
            switch(g_gameLoopInterrupt.site)
            {
            case GameLoopInterrupt::UpdatePlayer_MessageNPC:
            case GameLoopInterrupt::UpdatePlayer_TriggerTalk:
            case GameLoopInterrupt::UpdatePlayer_SuperWarp:
                goto resume_UpdatePlayer;
            case GameLoopInterrupt::UpdateNPCs_Activation_Generator:
            case GameLoopInterrupt::UpdateNPCs_Activation_Self:
            case GameLoopInterrupt::UpdateNPCs_Activation_Chain:
            case GameLoopInterrupt::UpdateNPCs_KillNPC:
                goto resume_UpdateNPCs;
            case GameLoopInterrupt::UpdateEvents:
                goto resume_UpdateEvents;
            case GameLoopInterrupt::UpdateBlocks_KillBlock:
            case GameLoopInterrupt::UpdateBlocks_TriggerHit:
            case GameLoopInterrupt::UpdateBlocks_SwitchOn:
            case GameLoopInterrupt::UpdateBlocks_SwitchOff:
            case GameLoopInterrupt::UpdateBlocks_SwitchOff_KillBlock:
                goto resume_UpdateBlocks;
            case GameLoopInterrupt::IntroEvents:
                goto resume_IntroEvents;
            default:
                break;
            }
        }

        return;
    }

    if(g_gameLoopInterrupt.process_intro_events)
    {
        int A;
        for(A = 0; A <= maxEvents; ++A)
        {
            // excluded in SMBX 1.3
            if(A == 1)
                continue;

            if(A == EVENT_LEVEL_START || Events[A].AutoStart)
            {
                eventindex_t resume_index;
                resume_index = ProcEvent_Safe(false, A, 0, true);
                while(resume_index != EVENT_NONE)
                {
                    g_gameLoopInterrupt.A = A;
                    g_gameLoopInterrupt.C = resume_index;
                    g_gameLoopInterrupt.site = GameLoopInterrupt::IntroEvents;
                    return;

resume_IntroEvents:
                    A = g_gameLoopInterrupt.A;
                    resume_index = g_gameLoopInterrupt.C;
                    g_gameLoopInterrupt.site = GameLoopInterrupt::None;

                    resume_index = ProcEvent_Safe(true, resume_index, 0, true);
                }
            }
        }

        g_gameLoopInterrupt.process_intro_events = false;
    }

    g_microStats.start_task(MicroStats::Script);
    lunaLoop();

    g_microStats.start_task(MicroStats::Controls);

    if(!Controls::Update())
    {
        QuickReconnectScreen::g_active = true;

        if(g_config.allow_drop_add && !TestLevel && XMessage::GetStatus() == XMessage::Status::local)
            PauseGame(PauseCode::DropAdd, 0);
    }

    if(QuickReconnectScreen::g_active)
        QuickReconnectScreen::Logic();

    Integrator::sync();

    g_microStats.start_task(MicroStats::Layers);

    if(LevelMacro > LEVELMACRO_OFF)
    {
        UpdateMacro();

        // was previously a nested frameloop, now that logic is still done in UpdateMacro,
        // but per-frame -> we need to skip the rest of the frame if it didn't finish
        if(LevelMacro == LEVELMACRO_KEYHOLE_EXIT)
            return;
    }

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
                g_curLevelMedals.reset_checkpoint();
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
    else if(qScreen || (g_config.allow_multires && qScreen_canonical))
    {
        g_microStats.start_task(MicroStats::Effects);
        UpdateEffects();
        if(!g_config.modern_section_change)
            speedRun_tick();
        UpdateGraphics();
        updateScreenFaders();

#if defined(__16M__) && !defined(__CALICO__)
        // make sure that streamed audio is still updated on non-threading 16M build
        Sound_StreamUpdate();
#endif
    }
    else if(BattleIntro > 0)
    {
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

resume_UpdateNPCs:
        g_microStats.start_task(MicroStats::NPCs);
        if(UpdateNPCs())
            return;

        if(LevelMacro == LEVELMACRO_KEYHOLE_EXIT)
            return; // stop on key exit

resume_UpdateBlocks:
        g_microStats.start_task(MicroStats::Blocks);
        if(UpdateBlocks())
            return;

        g_microStats.start_task(MicroStats::Effects);
        UpdateEffects();

resume_UpdatePlayer:
        g_microStats.start_task(MicroStats::Player);
        if(UpdatePlayer())
            return;

        speedRun_tick();
        // UpdateGraphics() now calls start_task internally
        if(LivingPlayers() || BattleMode)
            UpdateGraphics();
        g_microStats.start_task(MicroStats::Sound);
        UpdateSound();

resume_UpdateEvents:
        g_microStats.start_task(MicroStats::Events);
        if(UpdateEvents())
            return;
//        If MagicHand = True Then UpdateEditor

        updateScreenFaders();

        // Pause game and CaptainN logic
        if((LevelMacro == LEVELMACRO_OFF && CheckLiving() > 0) || SharedPauseForce)
        {
            // this is always able to pause the game even when CaptainN is enabled.
            if(SharedPause)
                PauseInit(PauseCode::PauseScreen, 0);
            // don't let double-pause or double-toggle happen
            else
            {
                for(int p = 1; p <= numPlayers && p <= maxLocalPlayers; p++)
                {
                    // only consider new start presses
                    if(!Player[p].UnStart || !Player[p].Controls.Start)
                        continue;

                    // use limited, buggy code for non-player 1 in compat case
                    if(p != 1 && !g_config.multiplayer_pause_controls)
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
                        PauseInit(PauseCode::PauseScreen, 0);

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
    switch(g_MessageType)
    {
    case MESSAGE_TYPE_SYS_ERROR:
        SoundPause[SFX_SMGlass] = 0;
        PlaySoundMenu(SFX_SMGlass);
        break;
#ifdef THEXTECH_ASSERTS_INGAME_MESSAGE
    case MESSAGE_TYPE_SYS_FATAL_ASSERT:
        StopMusic();
        SoundPause[SFX_SMGlass] = 0;
        PlayErrorSound(SFX_SMGlass);
        break;
#endif
    default:
        SoundPause[SFX_Message] = 0;
        PlaySound(SFX_Message);
        break;
    }

    if(g_MessageType >= MESSAGE_TYPE_SYS_INFO && MessageTitle.empty())
    {
        switch(g_MessageType)
        {
        case MESSAGE_TYPE_SYS_INFO:
            MessageTitle = g_gameStrings.msgBoxTitleInfo;
            break;
        case MESSAGE_TYPE_SYS_WARNING:
            MessageTitle = g_gameStrings.msgBoxTitleWarning;
            break;
        case MESSAGE_TYPE_SYS_ERROR:
        case MESSAGE_TYPE_SYS_FATAL_ASSERT:
            MessageTitle = g_gameStrings.msgBoxTitleError;
            break;
        default:
            break;
        }
    }

    MenuCursorCanMove = false;
    MenuCursorCanMove_Back = false;
    PrepareMessageDims();
}

bool MessageScreen_Logic(int plr)
{
    // can't check other local shared controls because this is global logic
    bool menuDoPress = SharedPause;
    bool menuBackPress = false;

    if(GameMenu)
    {
        bool clicked = (SharedCursor.Primary || SharedCursor.Secondary);
        menuDoPress |= clicked;
        MenuMouseRelease = !clicked;
    }

    // there was previously code to copy all players' controls from the main player, but this is no longer necessary (and actively harmful in the SingleCoop case)

    if(!g_config.multiplayer_pause_controls && plr == 0)
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

    if(!MenuCursorCanMove_Back)
    {
        if(!menuBackPress && MenuCursorCanMove)
            MenuCursorCanMove_Back = true;

        menuBackPress = false;
    }
    else if(menuBackPress)
        MenuCursorCanMove_Back = false;

    if(!MenuCursorCanMove)
    {
        if(!menuDoPress && !menuBackPress)
            MenuCursorCanMove = true;

        return false;
    }

    if(MenuCursorCanMove && (menuDoPress || menuBackPress))
    {
#if defined(THEXTECH_ASSERTS_INGAME_MESSAGE) && !defined(THEXTECH_NO_SDL_BUILD)
        // When it's a fatal error, just quit everything immediately
        if(g_MessageType == MESSAGE_TYPE_SYS_FATAL_ASSERT)
        {
            GracefulQuit(false);

#ifdef __WIIU__
            if(g_isHBLauncher)
                SYSRelaunchTitle(0, NULL);
#endif

            Controls::Quit();
            QuitMixerX();
            g_frmMain.freeSystem();
#   if defined(__WII__) || defined(__WIIU__)
            exit(0);
#   else
            abort();
#   endif
        }
#endif

        MessageTitle.clear();
        MessageText.clear();
        g_MessageType = MESSAGE_TYPE_NORMAL;
        return true;
    }

    return false;
}

static constexpr int s_max_pause_stack_depth = 4;

struct PauseLoopState
{
    int pause_player = 0;
    PauseCode pause_stack[s_max_pause_stack_depth + 1];
    typedef void (*callback_t)();
    callback_t pause_stack_callback[s_max_pause_stack_depth + 1];
    int pause_stack_depth = 0;
};

static PauseLoopState s_pauseLoopState;

// initializes a certain pause screen (but does not reset the game loop)
void PauseInit(PauseCode code, int plr, void (*callback)())
{
    // initialize pause from main game
    if(GamePaused == PauseCode::None)
    {
        s_pauseLoopState = PauseLoopState();
        s_pauseLoopState.pause_player = plr;

        if(!GameMenu && !LevelEditor)
        {
            for(int A = numPlayers; A >= 1; A--)
                SavedChar[Player[A].Character] = Player[A];
        }

        if(PSwitchTime > 0)
            PauseMusic();
    }
    // push pause code stack
    else if(s_pauseLoopState.pause_stack_depth < s_max_pause_stack_depth)
        s_pauseLoopState.pause_stack_depth++;

    // set pause code
    GamePaused = code;

    // store pause code and pause callback for this pause
    s_pauseLoopState.pause_stack[s_pauseLoopState.pause_stack_depth] = GamePaused;
    s_pauseLoopState.pause_stack_callback[s_pauseLoopState.pause_stack_depth] = callback;

    // init correct pause screen type
    if(code == PauseCode::Message)
        MessageScreen_Init();
    else if(code == PauseCode::PauseScreen)
        PauseScreen::Init(s_pauseLoopState.pause_player, SharedPauseLegacy);
    else if(code == PauseCode::DropAdd)
        ConnectScreen::DropAdd_Start();
    else if(code == PauseCode::Prompt)
        PromptScreen::Init();
    else if(code == PauseCode::Options)
        OptionsScreen::Init();
    else if(code == PauseCode::TextEntry)
    {
        // assume TextEntryScreen has already been inited through its Run function.
    }

    // sync system cursor
    SyncSysCursorDisplay();

    // resetFrameTimer();
}

// finishes the current pause and pops it from the pause stack
static void s_PauseFinish(int stack_level);

// a main loop for cases where the game is paused
void PauseLoop()
{
    int cur_stack_level = s_pauseLoopState.pause_stack_depth;

    g_microStats.start_task(MicroStats::Graphics);

#if defined(THEXTECH_ASSERTS_INGAME_MESSAGE) && !defined(THEXTECH_NO_SDL_BUILD)
    const bool is_fatal_message = (g_MessageType == MESSAGE_TYPE_SYS_FATAL_ASSERT);
#else
    constexpr bool is_fatal_message = false;
#endif

    if(!is_fatal_message)
        speedRun_tick();

    if(is_fatal_message)
        UpdateGraphicsFatalAssert();
    else if(GamePaused == PauseCode::Prompt)
        PromptScreen::Render();
    else if((LevelSelect && !GameMenu) || WorldEditor)
        UpdateGraphics2();
    else
        UpdateGraphics();

    g_microStats.start_task(MicroStats::Controls);

    if(!Controls::Update())
        QuickReconnectScreen::g_active = true;

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
    if(s_pauseLoopState.pause_player > numPlayers)
        s_pauseLoopState.pause_player = 0;

    g_microStats.start_task(MicroStats::Script);

    bool pause_done = false;

    // run the appropriate pause logic
    if(qScreen)
    {
        // prevent any logic or unpause from taking place

        // qScreen takes place in WorldLoop, not world graphics
        if(LevelSelect)
            qScreen = Update_qScreen(1);
    }
    else if(GamePaused == PauseCode::Message)
    {
        if(MessageScreen_Logic(s_pauseLoopState.pause_player))
            pause_done = true;
    }
    else if(GamePaused == PauseCode::Prompt)
    {
        if(PromptScreen::Logic())
            pause_done = true;
    }
    else
    {
        // Check messages from the main pause screen first, then everything else. Important for catching delayed messages.
        if(PauseScreen::Logic())
        {
            s_pauseLoopState.pause_stack_depth = 0;
            pause_done = true;
        }
        else if(GamePaused == PauseCode::DropAdd)
        {
            if(ConnectScreen::Logic())
                pause_done = true;
        }
        else if(GamePaused == PauseCode::Options)
        {
            if(OptionsScreen::Logic())
                pause_done = true;
        }
        else if(GamePaused == PauseCode::TextEntry)
        {
            if(TextEntryScreen::Logic())
                pause_done = true;
        }
    }

    g_microStats.end_frame();

    if(pause_done)
        s_PauseFinish(cur_stack_level);
}

static void s_PauseFinish(int stack_level)
{
    // perform callback
    if(s_pauseLoopState.pause_stack_callback[stack_level])
        s_pauseLoopState.pause_stack_callback[stack_level]();

    // special case: remove dropped frame from middle of pause stack
    if(s_pauseLoopState.pause_stack_depth > stack_level)
    {
        for(int i = stack_level; i < s_pauseLoopState.pause_stack_depth; i++)
        {
            s_pauseLoopState.pause_stack[i] = s_pauseLoopState.pause_stack[i + 1];
            s_pauseLoopState.pause_stack_callback[i] = s_pauseLoopState.pause_stack_callback[i + 1];
        }

        s_pauseLoopState.pause_stack_depth--;

        return;
    }

    // pop pause stack
    if(s_pauseLoopState.pause_stack_depth > 0)
    {
        s_pauseLoopState.pause_stack_depth--;
        GamePaused = s_pauseLoopState.pause_stack[s_pauseLoopState.pause_stack_depth];
    }
    // resume main game
    else
    {
        GamePaused = PauseCode::None;

        if(PSwitchTime > 0)
            ResumeMusic();

        // prevent unexpected button presses
        for(int i = 1; i <= numPlayers; i++)
        {
            Player[i].UnStart = false;
            Player[i].CanJump = false;
        }
    }

    MenuCursorCanMove = false;

    // sync system cursor
    SyncSysCursorDisplay();

    // resetFrameTimer();
}

void PauseGame(PauseCode code, int plr)
{
//    double fpsTime = 0;
//    int fpsCount = 0;

    // allow waiting for the current pause frame to terminate by triggering PauseGame with PauseCode::None
    if(code != PauseCode::None)
        PauseInit(code, plr);

    int cur_pause_stack_depth = s_pauseLoopState.pause_stack_depth;

    do
    {
        if(canProceedFrame())
        {
            computeFrameTime1();
            computeFrameTime2();

            g_microStats.start_task(MicroStats::Controls);

            XEvents::doEvents();
            CheckActive();

            PauseLoop();
        }

        if(!g_config.unlimited_framerate)
            PGE_Delay(1);

        if(!GameIsActive)
            break;
    } while(GamePaused != PauseCode::None && s_pauseLoopState.pause_stack_depth >= cur_pause_stack_depth);
}
