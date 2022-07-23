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

#include <fmt_format_ne.h>
#include "speedrunner.h"
#include "globals.h"
#include "graphics.h"
#include "core/render.h"
#include "compat.h"
#include "../controls.h"

#include "gameplay_timer.h"


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
#define bool2legacy(b) (b ? 0.7f : 0.0f), (b ? 0.7f : 0.0f),  (b ? 0.7f : 0.0f)

static Controls_t s_displayControls[maxLocalPlayers] = {Controls_t()};


void speedRun_renderTimer()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.render();
    SuperPrintRightAlign(fmt::format_ne("Mode {0}", g_speedRunnerMode), 3, ScreenW - 2, 2, 1.f, 0.3f, 0.3f, 0.5f);
}

static void GetControllerColor(int player, float& r, float& g, float& b, bool* drawLabel = nullptr)
{
    r = 0.4f; g = 0.4f; b = 0.4f;
    if(drawLabel)
        *drawLabel = false;

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

        if(drawLabel)
            *drawLabel = true;
    }
}

void RenderControls(int player, int x, int y, int w, int h)
{
    float alhpa = 0.7f;
    float alhpaB = 0.8f;
    float r, g, b;
    bool drawLabel;

    GetControllerColor(player, r, g, b, &drawLabel);

    XRender::renderRect(x, y, w, h, 0.f, 0.f, 0.f, alhpa, true);//Edge
    XRender::renderRect(x + 2, y + 2, w - 4, h - 4, r, g, b, alhpa, true);//Box

    if(player < 1 || player > maxLocalPlayers)
        return;

    const Controls_t& c = s_displayControls[player-1];
    XRender::renderRect(x + 10, y + 12, 6, 6, 0.f, 0.f, 0.f, alhpaB, true);//Cender of D-Pad
    XRender::renderRect(x + 10, y + 6, 6, 6, bool2gray(c.Up), alhpaB, true);
    XRender::renderRect(x + 10, y + 18, 6, 6, bool2gray(c.Down), alhpaB, true);
    XRender::renderRect(x + 4, y + 12, 6, 6, bool2gray(c.Left), alhpaB, true);
    XRender::renderRect(x + 16, y + 12, 6, 6, bool2gray(c.Right), alhpaB, true);

    XRender::renderRect(x + 64, y + 18, 6, 6, bool2green(c.Jump), alhpaB, true);
    XRender::renderRect(x + 66, y + 8, 6, 6, bool2red(c.AltJump), alhpaB, true);
    XRender::renderRect(x + 54, y + 16, 6, 6, bool2blue(c.Run), alhpaB, true);
    XRender::renderRect(x + 56, y + 6, 6, 6, bool2yellow(c.AltRun), alhpaB, true);

    XRender::renderRect(x + 26, y + 22, 10, 4, bool2gray(c.Drop), alhpaB, true);
    if(SharedControls.LegacyPause)
        XRender::renderRect(x + 40, y + 22, 10, 4, bool2legacy(c.Start), alhpaB, true);
    else
        XRender::renderRect(x + 40, y + 22, 10, 4, bool2gray(c.Start), alhpaB, true);

    if(drawLabel)
        SuperPrint(fmt::format_ne("P{0}", player), 3, x + 22, y + 2, 1.f, 1.f, 1.f, 0.5f);
}

void RenderControllerBattery(int player, int bx, int by, int bw, int bh)
{
    // force to 2x
    bx &= ~1;
    by &= ~1;
    bw &= ~1;
    bh &= ~1;

    float alhpa = 0.7f;
    float alhpaB = 0.8f;
    float r, g, b;

    GetControllerColor(player, r, g, b);

    Controls::StatusInfo status_info = Controls::GetStatus(player);

    if(status_info.power_status != Controls::StatusInfo::POWER_DISABLED)
    {
        XRender::renderRect(bx, by, bw - 4, bh, 0.f, 0.f, 0.f, alhpa, true);//Edge
        XRender::renderRect(bx + 2, by + 2, bw - 8, bh - 4, r, g, b, alhpa, true);//Box
        XRender::renderRect(bx + 36, by + 6, 4, 10, 0.f, 0.f, 0.f, alhpa, true);//Edge
        XRender::renderRect(bx + 34, by + 8, 4, 6, r, g, b, alhpa, true);//Box

        int segments;

        if(status_info.power_level > .90f)
            segments = 4;
        else if(status_info.power_level > .60f)
            segments = 3;
        else if(status_info.power_level > .30f)
            segments = 2;
        else
            segments = 1;

        // new color for the inner segments
        r = 0.f;
        g = 0.f;
        b = 0.f;
        if(status_info.power_level <= .5f)
        {
            r = (.5f - status_info.power_level) / .5f;
        }
        if(status_info.power_status == Controls::StatusInfo::POWER_CHARGING)
        {
            g = 1.f;
            g -= r;
        }
        if(status_info.power_status == Controls::StatusInfo::POWER_CHARGED)
        {
            b = 0.8f;
            g = 0.4f;
        }

        int s;

        switch(segments)
        {
        case 4:
            s = 2.f * (status_info.power_level - 0.9f) / 0.1f;
            if(s > 2) s = 2;
            // if(flash && status_info.power_status == Controls::StatusInfo::POWER_CHARGING)
            //     s = 2;
            XRender::renderRect(bx + 34, by + 10, s, 2, r, g, b, alhpaB, true); // fallthrough
        case 3:
            s = 4.f * (status_info.power_level - 0.6f) / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == Controls::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 24, by + 4, s*2, 14, r, g, b, alhpaB, true); // fallthrough
        case 2:
            s = 4.f * (status_info.power_level - 0.3f) / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == Controls::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 14, by + 4, s*2, 14, r, g, b, alhpaB, true); // fallthrough
        case 1:
            s = 4.f * status_info.power_level / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == Controls::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 4, by + 4, s*2, 14, r, g, b, alhpaB, true);
            break;
        }
        if(status_info.power_status == Controls::StatusInfo::POWER_UNKNOWN)
            SuperPrintCenter("?", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 8) & ~1);
        if(status_info.power_status == Controls::StatusInfo::POWER_WIRED)
            SuperPrintCenter("W", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 8) & ~1);
        if(status_info.power_status == Controls::StatusInfo::POWER_CHARGING)
            SuperPrintCenter("+", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 7) & ~1);
    }

    if(status_info.info_string)
        SuperPrintCenter(status_info.info_string, 3, bx + bw / 2, by - 30);
}

void speedRun_renderControls(int player, int screenZ, int align)
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF && !g_drawController)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode || LevelEditor)
        return; // Don't draw things at Menu and Outro

    if(player < 1 || player > 2)
        return;

    // Controller
    int x, y;
    int w = 76;
    int h = 30;

    // Battery status
    int bx, by;
    int bw = 40;
    int bh = 22;

    if(screenZ >= 0)
    {
        auto &scr = vScreen[screenZ];
        y = (int)scr.Height - 34;
        by = y + 4;

        if(align == SPEEDRUN_ALIGN_AUTO)
        {
            align = scr.Left > 0 ? SPEEDRUN_ALIGN_RIGHT : SPEEDRUN_ALIGN_LEFT;
        }

        // this keeps the locations fixed even when the vScreens expand/contract
        if(align == SPEEDRUN_ALIGN_LEFT)
        {
            if((int)scr.Width < 800)
            {
                x = 4;
                bx = x + w + 4;
            }
            else
            {
                x = ScreenW / 2 - 800 / 2 - scr.ScreenLeft + 4;
                bx = x + w + 4;
            }
        }
        else
        {
            if((int)scr.Width < 800)
            {
                x = (int)scr.Width - (w + 4);
                bx = x - (bw + 4);
            }
            else
            {
                x = ScreenW / 2 + 800 / 2 - scr.ScreenLeft - (w + 4);
                bx = x - (bw + 4);
            }
        }
    }
    else
    {
#if 0
        bool firstLefter =   Player[1].Location.X + (Player[1].Location.Width / 2)
                           < Player[2].Location.X + (Player[2].Location.Width / 2);

        switch(player)
        {
        case 1:
            x = firstLefter ? 4 : (ScreenW - (w + 4));
            break;
        case 2:
            x = firstLefter ? (ScreenW - (w + 4)) : 4;
            break;
        }
#else
        if(align == SPEEDRUN_ALIGN_LEFT)
            player = 1;
        else if(align == SPEEDRUN_ALIGN_RIGHT)
            player = 2;

        switch(player)
        {
        case 1:
            x = 4;
            bx = x + w + 4;
            break;
        case 2:
            x = (ScreenW - (w + 4));
            bx = x - (bw + 4);
            break;
        }

        y = ScreenH - 34;
        by = y + 4;
#endif
    }

    RenderControls(player, x, y, w, h);

    RenderControllerBattery(player, bx, by, bw, bh);
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

void speedRun_setBlinkEffect(int be)
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    s_gamePlayTimer.setBlinkEffect(be);
}

void speedRun_syncControlKeys(int plr, const Controls_t &keys)
{
    // there are still reasons to sync control keys (eg control tests)
    // if(g_speedRunnerMode == SPEEDRUN_MODE_OFF && !g_drawController)
    //     return; // Do nothing

    SDL_assert(plr >= 0 && plr < maxLocalPlayers);
    SDL_memcpy(&s_displayControls[plr], &keys, sizeof(Controls_t));
}
