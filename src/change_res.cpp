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

#include "globals.h"
#include "video.h"
#include "config.h"
#include "change_res.h"
#include "load_gfx.h"
#include "core/render.h"
#include "core/window.h"
#ifdef __EMSCRIPTEN__
#include "core/events.h"
#endif

void SetOrigRes()
{
    XWindow::setFullScreen(false);
    resChanged = false;
    XWindow::setWindowSize(ScreenW, ScreenH);

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
#ifndef THEXTECH_FIXED_RES
    if(g_config.InternalW == 0 || g_config.InternalH == 0)
    {
        int int_w, int_h;

        XWindow::getWindowSize(&int_w, &int_h);

        if(g_config.InternalH != 0)
        {
            int_w = int_w*g_config.InternalH/int_h;
            int_h = g_config.InternalH;
        }
        else if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
        {
            int_w *= 2;
            int_h *= 2;
        }
        else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        {
            int_w /= 2;
            int_h /= 2;
        }
        else if(g_videoSettings.scaleMode == SCALE_DYNAMIC_INTEGER)
        {
            int i = 1;
            while(int_w/(i+1) > 800 && int_h/(i+1) > 600)
                i++;
            int_w /= i;
            int_h /= i;
        }

        if(int_w < 480) int_w = 480;
        if(int_h < 320) int_h = 320;

        // maximum height 720p, rescale width accordingly so long as above 800p
        if(int_h > 720)
        {
            // still a couple cases to smooth out, such as the cases where we're a little wide and over 720p tall
            if((g_videoSettings.scaleMode == SCALE_DYNAMIC_NEAREST
                || g_videoSettings.scaleMode == SCALE_DYNAMIC_LINEAR))
            {
                int_w = int_w * 720 / int_h;
                if(int_w < 800)
                    int_w = 800;
            }
            if(g_videoSettings.scaleMode == SCALE_DYNAMIC_INTEGER
                && int_w / std::floor(int_h / 720) > 800)
            {
                int_w = int_w / std::floor(int_h / 720);
            }
            int_h = 720;
        }
        // maximum 2.4 (cinematic) aspect ratio
        if(int_w > int_h*2.4)
            int_w = int_h*2.4;

        // force even dimensions
        int_w -= int_w & 1;
        int_h -= int_h & 1;

        ScreenW = int_w;
        ScreenH = int_h;
    }
    else
    {
        ScreenW = g_config.InternalW;
        ScreenH = g_config.InternalH;
    }
#endif // #ifndef THEXTECH_FIXED_RES

    XRender::updateViewport();
}
