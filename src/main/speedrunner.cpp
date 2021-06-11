/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <fmt_format_ne.h>
#include "speedrunner.h"
#include "globals.h"
#include "graphics.h"

#include "gameplay_timer.h"


static      GameplayTimer s_gamePlayTimer;
int                       g_speedRunnerMode = SPEEDRUN_MODE_OFF;

void speedRun_loadStats()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.load();
}

void speedRun_saveStats()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing
    if(GameMenu || GameOutro || BattleMode)
        return; // Do nothing when out of the game

    s_gamePlayTimer.save();
}

void speedRun_resetCurrent()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.resetCurrent();
}

void speedRun_resetTotal()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.reset();
}

#define bool2alpha(b) (b ? 1.f : 0.1f)

#define bool2color(b) (b ? 1.f : 0.0f)
#define bool2colorLt(b) (b ? 0.9f : 0.0f)

#define bool2green(b) 0.f,              (b ? 1.f : 0.0f),   0.f
#define bool2blue(b)  0.f,              0.f,                (b ? 1.f : 0.0f)
#define bool2red(b)  (b ? 1.f : 0.0f),  0.f,                0.f
#define bool2yellow(b) (b ? 1.f : 0.0f), (b ? 1.f : 0.0f),  0.f
#define bool2gray(b) (b ? 0.9f : 0.0f), (b ? 0.9f : 0.0f),  (b ? 0.9f : 0.0f)

static Controls_t s_displayControls[2] = {Controls_t()};

void speedRun_render()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.render();

    int plrs = numPlayers > 2 ? 1 : numPlayers;
    int plrs_x[] = {8, ScreenW - 80};

    int w = 76;
    int h = 30;
    float alhpa = 0.7f;
    float alhpaB = 0.8f;

    for(int i = 0; i < plrs; ++i)
    {
        const auto &c = s_displayControls[i];
        int x = plrs_x[i];
        int y = ScreenH - 32;

        frmMain.renderRect(x, y, w, h, 0.4f, 0.4f, 0.4f, alhpa, true);//Box
        frmMain.renderRect(x, y, w, h, 0.f, 0.f, 0.f, alhpa, false);//Edge

#if 1
        frmMain.renderRect(x + 10, y + 12, 6, 6, 0.f, 0.f, 0.f, alhpaB, true);//Cender of D-Pad
        frmMain.renderRect(x + 10, y + 6, 6, 6, bool2gray(c.Up), alhpaB, true);
        frmMain.renderRect(x + 10, y + 18, 6, 6, bool2gray(c.Down), alhpaB, true);
        frmMain.renderRect(x + 4, y + 12, 6, 6, bool2gray(c.Left), alhpaB, true);
        frmMain.renderRect(x + 16, y + 12, 6, 6, bool2gray(c.Right), alhpaB, true);

        frmMain.renderRect(x + 64, y + 18, 6, 6, bool2green(c.Jump), alhpaB, true);
        frmMain.renderRect(x + 66, y + 8, 6, 6, bool2red(c.AltJump), alhpaB, true);
        frmMain.renderRect(x + 54, y + 16, 6, 6, bool2blue(c.Run), alhpaB, true);
        frmMain.renderRect(x + 56, y + 6, 6, 6, bool2yellow(c.AltRun), alhpaB, true);

        frmMain.renderRect(x + 26, y + 22, 10, 4, bool2gray(c.Drop), alhpaB, true);
        frmMain.renderRect(x + 40, y + 22, 10, 4, bool2gray(c.Start), alhpaB, true);

#elif 1
        frmMain.renderRect(x + 10, y + 6, 6, 6, bool2color(c.Up), bool2color(c.Up), 0.f, alhpaB, false);
        if(c.Up)
            SuperPrint("]", 3, x, ScreenH - 24);

        if(c.Down)
            SuperPrint("v", 3, x + 16, y);

        if(c.Left)
            SuperPrint("<", 3, x + 32, y);

        if(c.Right)
            SuperPrint(">", 3, x + 48, y);

        if(c.AltJump)
            SuperPrint("P", 3, x + 64, y);

        if(c.Jump)
            SuperPrint("J", 3, x + 80, y);

        if(c.AltRun)
            SuperPrint("N", 3, x + 96, y);

        if(c.Run)
            SuperPrint("R", 3, x + 112, y);

        if(c.Start)
        {
            SuperPrint("S", 3, x + 128, y);
            SuperPrint("T", 3, x + 144, y);
        }

        if(c.Drop)
        {
            SuperPrint("S", 3, x + 160, y);
            SuperPrint("E", 3, x + 176, y);
        }

#else
        SuperPrint("]", 3, 8, 576, 1.f, 1.f, 1.f, bool2alpha(c.Up));
        SuperPrint("v", 3, 24, 584, 1.f, 1.f, 1.f, bool2alpha(c.Down));
        SuperPrint("<", 3, 40, 584, 1.f, 1.f, 1.f, bool2alpha(c.Left));
        SuperPrint(">", 3, 56, 584, 1.f, 1.f, 1.f, bool2alpha(c.Right));
        SuperPrint("P", 3, 72, 584, 1.f, 1.f, 1.f, bool2alpha(c.AltJump));
        SuperPrint("J", 3, 88, 584, 1.f, 1.f, 1.f, bool2alpha(c.Jump));
        SuperPrint("N", 3, 104, 584, 1.f, 1.f, 1.f, bool2alpha(c.AltRun));
        SuperPrint("R", 3, 120, 584, 1.f, 1.f, 1.f, bool2alpha(c.Run));

        SuperPrint("S", 3, 136, 584, 1.f, 1.f, 1.f, bool2alpha(c.Start));
        SuperPrint("T", 3, 152, 584, 1.f, 1.f, 1.f, bool2alpha(c.Start));

        SuperPrint("S", 3, 168, 584, 1.f, 1.f, 1.f, bool2alpha(c.Drop));
        SuperPrint("E", 3, 184, 584, 1.f, 1.f, 1.f, bool2alpha(c.Drop));
#endif
    }

    SuperPrintRightAlign(fmt::format_ne("Mode {0}", g_speedRunnerMode), 3, ScreenW - 2, 2, 1.f, 0.3f, 0.3f, 0.5f);
}

#undef bool2alpha

void speedRun_tick()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.tick();
}

void speedRun_bossDeadEvent()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.onBossDead();
}

void speedRun_setSemitransparentRender(bool r)
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.setSemitransparent(r);
}

void speedRun_syncControlKeys(int plr, Controls_t &keys)
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    SDL_assert(plr >= 0 && plr < 2);
    SDL_memcpy(&s_displayControls[plr], &keys, sizeof(Controls_t));
}
