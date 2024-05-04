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
#include <Logger/private/logger_sets.h>

#include "core/msgbox.h"
#include "core/events.h"

#include "config.h"
#include "config/config_hooks.h"

#include "main/translate.h"
#include "main/menu_main.h"
#include "main/screen_options.h"

#include "frm_main.h"
#include "change_res.h"
#include "globals.h"
#include "game_main.h"
#include "sound.h"
#include "graphics.h"
#include "player.h"

#include "Integrator/integrator.h"

#ifndef THEXTECH_NO_SDL_BUILD
#include <Graphics/graphics_funcs.h>
#endif


// config_main.cpp
extern bool g_configInTransaction;

void config_res_set()
{
    if(GameIsActive && (GameMenu || GamePaused == PauseCode::Options || g_config.internal_res.m_set == ConfigSetLevel::cheat))
        UpdateWindowRes();
    UpdateInternalRes();
}

void config_rendermode_set()
{
    if(!GameIsActive)
        return;

    bool res = g_frmMain.restartRenderer();
    PlaySoundMenu(SFX_PSwitch);

    if(!res)
    {
        MessageText = g_mainMenu.optionsRestartEngine;
        PauseGame(PauseCode::Message);
    }
}

void config_fullscreen_set()
{
    if(!GameIsActive)
        return;

    if(g_config.fullscreen)
        ChangeRes(0, 0, 0, 0);
    else
        SetOrigRes();

    XEvents::doEvents();
}

void config_mountdrums_set()
{
    if(!GameIsActive)
        return;

    UpdateYoshiMusic();
}

void config_screenmode_set()
{
    // TODO: only update the locally-controlled screens for NetPlay

    Screens[0].two_screen_pref = g_config.two_screen_mode;
    Screens[0].four_screen_pref = g_config.four_screen_mode;

    Screens[0].canonical_screen().two_screen_pref = g_config.two_screen_mode;
    Screens[0].canonical_screen().four_screen_pref = g_config.four_screen_mode;

    SetupScreens();
    PlayersEnsureNearby(Screens[0]);
}

void config_audiofx_set()
{
    if(!GameIsActive)
        return;

    if(!LevelSelect && !LevelEditor)
        UpdateSoundFX(Player[1].Section);
}

void config_audio_set()
{
    if(!GameIsActive)
        return;

    RestartMixerX();

    if(GameMenu || !LevelSelect)
        UpdateSoundFX(Player[1].Section);
}

void config_music_volume_set()
{
    if(!GameIsActive)
        return;

    UpdateMusicVolume();
}

void config_language_set()
{
    if(!GameIsActive)
        return;

    ReloadTranslations();

    OptionsScreen::ResetStrings();
}

void config_log_level_set()
{
    g_pLogGlobalSetup.level = (PGE_LogLevel::Level)(int)g_config.log_level;
    LogWriter::m_logLevel = g_pLogGlobalSetup.level;
    pLogInfo("Updated log level to %d", (int)g_config.log_level);
}

void config_integrations_set()
{
    Integrator::quitIntegrations();
    Integrator::initIntegrations();
}

void config_compat_changed()
{
    if(!g_configInTransaction)
        UpdateConfig();
}
