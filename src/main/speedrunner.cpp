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
#include "compat.h"

#include "gameplay_timer.h"
#include "game_main.h"

static      GameplayTimer s_gamePlayTimer;
int                       g_speedRunnerMode = SPEEDRUN_MODE_OFF;
bool                      g_drawController = false;

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


void speedRun_renderTimer()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.render();

    SuperPrintRightAlign(fmt::format_ne("Mode {0}", g_speedRunnerMode), 3, ScreenW - 2, 2, 1.f, 0.3f, 0.3f, 0.5f);
}

void speedRun_renderControls(int player, int screenZ, int align)
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF && !g_drawController)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode || LevelEditor)
        return; // Don't draw things at Menu and Outro

    if(player < 1 || player > 2)
        return;

    if(screenZ < 0)
        return;

    auto &scr = vScreen[screenZ];
    int x = 4;
    int y = (int)scr.Height - 34;
    int w = 76;
    int h = 30;
    bool drawLabel = false;

    if(align == SPEEDRUN_ALIGN_AUTO)
    {
        align = scr.Left > 0 ? SPEEDRUN_ALIGN_RIGHT : SPEEDRUN_ALIGN_LEFT;
    }

    if(align == SPEEDRUN_ALIGN_LEFT)
    {
        x = 4;
    }
    else
    {
        x = (int)scr.Width - (w + 4);
    }

    float alhpa = 0.7f;
    float alhpaB = 0.8f;
    float r = 0.4f, g = 0.4f, b = 0.4f;

    if(ScreenType == 5)  // TODO: VERIFY THIS
    {
        auto &p = Player[player];

        switch(p.Character) // TODO: Add changing of these colors by gameinfo.ini
        {
        case 1:
            r = 0.7f;
            g = 0.3f;
            b = 0.3f;
            break;
        case 2:
            r = 0.3f;
            g = 0.7f;
            b = 0.3f;
            break;
        case 3:
            r = 1.0f;
            g = 0.6f;
            b = 0.7f;
            break;
        case 4:
            r = 0.04f;
            g = 0.43f;
            b = 1.0f;
            break;
        case 5:
            r = 0.752941176f;
            g = 0.658823529f;
            b = 0.282352941f;
            break;
        }

        drawLabel = true;
    }

    const auto &c = s_displayControls[player - 1];

    frmMain.renderRect(x, y, w, h, 0.f, 0.f, 0.f, alhpa, true);//Edge
    frmMain.renderRect(x + 2, y + 2, w - 4, h - 4, r, g, b, alhpa, true);//Box

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

    if(drawLabel)
        SuperPrint(fmt::format_ne("P{0}", player), 3, x + 22, y + 2, 1.f, 1.f, 1.f, 0.5f);
}

#undef bool2alpha

void speedRun_tick()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.tick();
}

void speedRun_triggerEnter()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(g_compatibility.speedrun_stop_timer_by != Compatibility_t::SPEEDRUN_STOP_ENTER_LEVEL)
        return;

    if(SDL_strcasecmp(FileName.c_str(), g_compatibility.speedrun_stop_timer_at) == 0)
        speedRun_bossDeadEvent();
}

void speedRun_triggerLeave()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(g_compatibility.speedrun_stop_timer_by != Compatibility_t::SPEEDRUN_STOP_LEAVE_LEVEL)
        return;

    if(SDL_strcasecmp(FileName.c_str(), g_compatibility.speedrun_stop_timer_at) == 0)
        speedRun_bossDeadEvent();
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
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF && !g_drawController)
        return; // Do nothing

    SDL_assert(plr >= 0 && plr < 2);
    SDL_memcpy(&s_displayControls[plr], &keys, sizeof(Controls_t));
}
