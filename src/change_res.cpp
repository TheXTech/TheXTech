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
#include "core/render.h"
#ifdef __EMSCRIPTEN__
#include "core/events.h"
#endif

void SetOrigRes()
{
    XWindow::setFullScreen(false);
    resChanged = false;

#ifndef __EMSCRIPTEN__
    if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
        XWindow::setWindowSize(XRender::TargetW / 2, XRender::TargetH / 2);
    else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        XWindow::setWindowSize(XRender::TargetW * 2, XRender::TargetH * 2);
    else
        XWindow::setWindowSize(XRender::TargetW, XRender::TargetH);
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
    resChanged = true;
    XWindow::setFullScreen(true);

    if(LoadingInProcess)
        UpdateLoad();

    XWindow::showCursor(0);
}

//void SaveIt(int ScX, int ScY, int ScC, int ScF, std::string ScreenChanged)
//{

//}

void UpdateInternalRes()
{
    int req_w = g_config.InternalW;
    int req_h = g_config.InternalH;

#ifdef __3DS__
    req_w += XRender::TargetOverscanX * 2;
#endif

    // TODO: use the correct canonical screen's resolution here
    int canon_w = 800;
    int canon_h = 600;

    if(!g_compatibility.allow_multires)
    {
        if((req_w != 0 && req_w < canon_w) || (req_h != 0 && req_h < canon_h))
        {
            req_w = canon_w;
            req_h = canon_h;
        }
    }

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
            int_w *= 2;
        }
        else if(g_videoSettings.scaleMode == SCALE_FIXED_1X)
        {
            // keep as-is
            // int_w = int_w;
        }
        else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        {
            int_w /= 2;
        }
        else if(g_videoSettings.scaleMode == SCALE_DYNAMIC_INTEGER)
        {
            int scale_factor = orig_int_h / int_h;
            if(scale_factor == 0)
            {
                // keep as-is
                // int_w = int_w;
            }
            else if(int_w / scale_factor >= 800)
            {
                int_w /= scale_factor;
            }
            else
            {
                // scale based on width
                int scale_factor = int_w / 800;
                if(scale_factor != 0)
                    int_w /= scale_factor;

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
        if(!g_compatibility.allow_multires)
        {
            if(int_w < canon_w)
            {
                int_h = (int_h * canon_w) / int_w;
                int_w = canon_w;
                if(int_h > 720)
                    int_h = 720;
            }
            if(int_h < canon_h)
            {
                int_w = (int_w * canon_h) / int_h;
                int_h = canon_h;
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

        XRender::TargetW = int_w;
        XRender::TargetH = int_h;
    }
    else
    {
        XRender::TargetW = req_w;
        XRender::TargetH = req_h;
    }

    // TODO: above should tweak render target resolution. This should tweak game's screen resolution.
    if(g_compatibility.allow_multires)
    {
        l_screen->W = XRender::TargetW;
        l_screen->H = XRender::TargetH;
    }
    else
    {
        l_screen->W = canon_w;
        l_screen->H = canon_h;
    }

    XRender::updateViewport();

    // recenter the game menu graphics
    if(GameMenu)
    {
        SetupScreens();
        CenterScreens();
        GameMenu = false;
        GetvScreenAverage(Screens[0].vScreen(1));
        if(!Screens[0].is_canonical())
            GetvScreenAverage(Screens[0].canonical_screen().vScreen(1));
        GameMenu = true;
    }

    // disable world map qScreen if active
    if(LevelSelect && qScreen)
        qScreen = false;
}

void UpdateWindowRes()
{
    if(XWindow::isFullScreen() || XWindow::isMaximized())
        return;

    int h = g_config.InternalH;
    if(h == 0)
        return;

    int w = g_config.InternalW;

    if(w == 0 && h == XRender::TargetH)
        w = XRender::TargetW;
    else if(w == 0)
        return;

    if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
        XWindow::setWindowSize(w / 2, h / 2);
    else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        XWindow::setWindowSize(w * 2, h * 2);
    else
        XWindow::setWindowSize(w, h);
}

