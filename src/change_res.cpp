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
#include "change_res.h"
#include "load_gfx.h"
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
        XWindow::setWindowSize(ScreenW / 2, ScreenH / 2);
    else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        XWindow::setWindowSize(ScreenW * 2, ScreenH * 2);
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
    XRender::updateViewport();
}

void UpdateWindowRes()
{
    if(XWindow::isFullScreen() || XWindow::isMaximized())
        return;

    int w = ScreenW;
    int h = ScreenH;

    if(g_videoSettings.scaleMode == SCALE_FIXED_05X)
        XWindow::setWindowSize(w / 2, h / 2);
    else if(g_videoSettings.scaleMode == SCALE_FIXED_1X)
        XWindow::setWindowSize(w, h);
    else if(g_videoSettings.scaleMode == SCALE_FIXED_2X)
        XWindow::setWindowSize(w * 2, h * 2);
}

