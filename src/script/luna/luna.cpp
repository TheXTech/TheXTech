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

#include "sdl_proxy/sdl_stdinc.h"

#include "luna.h"
#include "autocode.h"
#include "autocode_manager.h"
#include "lunainput.h"
#include "lunarender.h"
#include "lunaspriteman.h"
#include "lunacell.h"
#include "lunaplayer.h"
#include "lunacounter.h"
#include "lunalevels.h"
#include "lunavarbank.h"
#include "config.h"

#include "globals.h"


SDL_FORCE_INLINE bool lunaAllowed()
{
    return g_config.luna_enable_engine;
}

void lunaReset()
{
    lunaLevelsClear();

    gFrames = 0;

    gLastDownPress = 0;
    gDownTapped = 0;
    gLastUpPress = 0;
    gUpTapped = 0;
    gLastLeftPress = 0;
    gLeftTapped = 0;
    gLastRightPress = 0;
    gRightTapped = 0;

    gLastJumpPress = 0;
    gJumpTapped = 0;
    gLastRunPress = 0;
    gRunTapped = 0;

    gEnableDemoCounterByLC = false;
    gSMBXHUDSettings = SMBXHUDSettings();

    gAutoMan.Clear();
    Renderer::Get().ClearAllDebugMessages();
    Renderer::Get().ClearAllLoadedImages();
    Renderer::Get().ClearQueue();
    gSpriteMan.ResetSpriteManager();
    gCellMan.Reset();
    Input::ResetAll();

    gDeathCounter.quit();
}

void lunaLoad()
{
    lunaReset();

    bool isGame = !GameMenu && !GameOutro && !BattleMode && !LevelEditor && !TestLevel;
    bool dcAllow = g_config.enable_fails_tracking;

    if(dcAllow && isGame)
    {
        gDeathCounter.init();
    }

    if(!LevelEditor && g_config.luna_enable_engine && lunaAllowed())
    {
        // Load autocode
        gAutoMan.LoadFiles();

        // Init var bank
        gSavedVarBank.CopyBank(&gAutoMan.m_UserVars);

        // Init some stuff
        if(g_config.luna_allow_level_codes)
            lunaLevelsInit();
        gAutoMan.m_Hearts = 2;
    }

    if(dcAllow && isGame)
        gDeathCounter.Recount();
}

void lunaLoop()
{
    if(g_config.luna_enable_engine)
    {
        // Clean up
        gAutoMan.ClearExpired();

        // Update inputs
        Input::CheckSpecialCheats();
        Input::UpdateInputTasks();
    }

    if(!LevelEditor && g_config.luna_enable_engine && lunaAllowed())
    {
#if COMPILE_PLAYGROUND
        Playground::doPlaygroundStuff();
#endif
//        g_EventHandler.hookLevelLoop();

        // Run autocode
        gAutoMan.DoEvents(false);

        // Update some stuff
        gFrames++;
        gSavedVarBank.SaveIfNeeded();

        // Run any framecode
//        TestFrameCode();
        if(g_config.luna_allow_level_codes)
            lunaLevelsDo();
    }
}


void lunaRenderHud(int screenZ)
{
    bool dcAllow = g_config.enable_fails_tracking || gEnableDemoCounterByLC;
    if(dcAllow && g_config.show_fails_counter && ShowOnScreenHUD)
        gDeathCounter.Draw(screenZ);

    Renderer::Get().RenderBelowPriority(5);
}

void lunaRender(int screenZ)
{
    if(!LevelEditor && g_config.luna_enable_engine && lunaAllowed())
    {
        Renderer::Get().StartCameraRender(screenZ);
        gSpriteMan.RunSprites();
        Renderer::Get().RenderBelowPriority(0);
    }
}

void lunaRenderStart()
{
    if(!LevelEditor && g_config.luna_enable_engine && lunaAllowed())
        Renderer::Get().StartFrameRender();
}

void lunaRenderEnd()
{
    if(!LevelEditor && g_config.luna_enable_engine && lunaAllowed())
        Renderer::Get().EndFrameRender();
}
