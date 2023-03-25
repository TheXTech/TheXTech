/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "globals.h"
#include "video.h"
#include "compat.h"
#include "config.h"
#include "change_res.h"
#include "load_gfx.h"
#include "graphics.h"
#include "sound.h"
#include "game_main.h"
#include "core/render.h"
#include "core/window.h"
#ifdef __EMSCRIPTEN__
#include "core/events.h"
#endif

void SetOrigRes()
{
    XWindow::setFullScreen(false);
    resChanged = false;

#ifndef __EMSCRIPTEN__
    if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
        XWindow::setWindowSize(ScreenW/2, ScreenH/2);
    else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        XWindow::setWindowSize(ScreenW*2, ScreenH*2);
    else
        XWindow::setWindowSize(ScreenW, ScreenH);
#endif

#ifdef __EMSCRIPTEN__
    XEvents::eventResize();
#endif

    if(LoadingInProcess)
        UpdateLoad();

    if(!GameMenu && !MagicHand && !LevelEditor)
        XWindow::showCursor(1);
}

void ChangeRes(int, int, int, int)
{
    XWindow::setFullScreen(true);

    if(LoadingInProcess)
        UpdateLoad();
}

//void SaveIt(int ScX, int ScY, int ScC, int ScF, std::string ScreenChanged)
//{

//}

void UpdateInternalRes()
{
    int req_w = g_config.InternalW;
    int req_h = g_config.InternalH;

    if((!g_compatibility.free_level_res && !LevelSelect && !GameMenu)
        || (!g_compatibility.free_world_res && LevelSelect && !GameMenu))
    {
        if((req_w != 0 && req_w < 800) || (req_h != 0 && req_h < 600))
        {
            req_w = 800;
            req_h = 600;

#ifdef THEXTECH_FIXED_RES
            PlaySoundMenu(SFX_BowserKilled);
            MessageText = "Sorry! The requested compatibility mode was not enabled because your copy of TheXTech was not built to support a resolution of 800x600.";
            PauseGame(PauseCode::Message);
            MessageText.clear();
#endif
        }
    }

#ifndef THEXTECH_FIXED_RES
    if(req_w == 0 || req_h == 0)
    {
        int int_w, int_h, orig_int_h;

        XRender::getRenderSize(&int_w, &int_h);
        orig_int_h = int_h;

        // set internal height first
        if(req_h != 0)
        {
            int_h = req_h;
        }
        else if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
        {
            int_h *= 2;
        }
        else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        {
            int_h /= 2;
        }
        else if(g_videoSettings.scaleMode == SCALE_DYNAMIC_INTEGER)
        {
            if(int_h >= 600)
            {
                // constrains height to be in the 600-720p range
                int scale_factor = int_h / 600;
                int_h /= scale_factor;
            }
        }

        // minimum height constraint
        if(int_h < 320)
            int_h = 320;

        // maximum height constraint
        if(int_h > 720 && req_h <= 720)
            int_h = 720;

        // now, set width based on height and scaling mode
        if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
        {
            int_w = int_w * 2;
        }
        else if(g_videoSettings.scaleMode == SCALE_FIXED_1X)
        {
            int_w = int_w;
        }
        else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        {
            int_w = int_w / 2;
        }
        else if(g_videoSettings.scaleMode == SCALE_DYNAMIC_INTEGER)
        {
            int scale_factor = orig_int_h / int_h;
            if(scale_factor == 0)
            {
                int_w = int_w;
            }
            else if(int_w / scale_factor >= 800)
            {
                int_w = int_w / scale_factor;
            }
            else
            {
                // scale based on width
                int scale_factor = int_w / 800;
                if(scale_factor != 0)
                    int_w = int_w / scale_factor;

                // rescale the height if possible
                if(scale_factor != 0 && req_h == 0)
                {
                    int_h = orig_int_h / scale_factor;
                    if(int_h < 600)
                        int_h = 600;
                    if(int_h > 720)
                        int_h = 720;
                }
            }
        }
        else
        {
            int_w = (int_w * int_h) / orig_int_h;
        }

        // force >800x600 resolution if required
        if((!g_compatibility.free_level_res && !LevelSelect && !GameMenu)
            || (!g_compatibility.free_world_res && LevelSelect && !GameMenu))
        {
            if(int_w < 800)
            {
                int_h = int_h * 800 / int_w;
                int_w = 800;
                if(int_h > 720)
                    int_h = 720;
            }
            if(int_h < 600)
            {
                int_w = int_w * 600 / int_h;
                int_h = 600;
            }
        }

        // minimum width constraint
        if(int_w < 480)
            int_w = 480;

        // maximum 2.4 (cinematic) aspect ratio
        if(int_w > int_h*2.4)
            int_w = (int)(int_h*2.4);

        // force even dimensions
        int_w -= int_w & 1;
        int_h -= int_h & 1;

        ScreenW = int_w;
        ScreenH = int_h;
    }
    else
    {
        ScreenW = req_w;
        ScreenH = req_h;
    }
#endif // #ifndef THEXTECH_FIXED_RES

    XRender::updateViewport();

    // recenter the game menu graphics
    if(GameMenu)
    {
        SetupScreens();
        CenterScreens();
        GameMenu = false;
        GetvScreenAverage();
        GameMenu = true;
    }
}

void UpdateWindowRes()
{
#ifndef THEXTECH_FIXED_RES
    if(resChanged)
        return;

    int h = g_config.InternalH;
    if(h == 0)
        return;

    int w = g_config.InternalW;
    if(w == 0 && h == ScreenH)
        w = ScreenW;
    else if(w == 0)
        return;

    if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
        XWindow::setWindowSize(w / 2, h / 2);
    else if(g_videoSettings.scaleMode == SCALE_FIXED_1X)
        XWindow::setWindowSize(w, h);
    else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        XWindow::setWindowSize(w * 2, h * 2);
#endif // #ifndef THEXTECH_FIXED_RES
}

