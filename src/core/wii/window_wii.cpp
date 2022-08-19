/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <gccore.h>

#include "globals.h"
#include "core/window.h"

namespace XRender
{
    extern int g_rmode_w, g_rmode_h;
};

namespace XWindow
{

bool init()
{
    return true;
}

void quit() {}

void show() {}
void hide() {}
int showCursor(int show) { return show; }
void setCursor(Cursor_t /*cursor*/) {}
Cursor_t getCursor() { return CURSOR_NONE; }
void placeCursor(int /*window_x*/, int /*window_y*/) {}
bool isFullScreen() { return true; }
int setFullScreen(bool /*fs*/) { return 1; }
void restoreWindow() {}
void setWindowSize(int /*w*/, int /*h*/) {}

void getWindowSize(int *w, int *h)
{
    // widescreen_stretch || widescreen_zoom
    int eff_h = (true) ? XRender::g_rmode_w * 9 / 16 : XRender::g_rmode_h;
    *w = XRender::g_rmode_w * 2;
    *h = eff_h * 2;
}

bool hasWindowInputFocus() { return true; }
bool hasWindowMouseFocus() { return true; }


}; // namespace XWindow
