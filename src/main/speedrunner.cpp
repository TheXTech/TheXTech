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

#include "sdl_proxy/sdl_stdinc.h"
#include <fmt_format_ne.h>
#include "speedrunner.h"
#include "globals.h"
#include "graphics.h"
#include "core/render.h"
#include "compat.h"
#include "../controls.h"

#include "main/screen_quickreconnect.h"

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
    SuperPrintRightAlign(fmt::format_ne(Cheater ? "CMode {0}" : "Mode {0}", g_speedRunnerMode), 3, ScreenW - 2, 2, 1.f, 0.3f, 0.3f, 0.5f);
}

static void GetControllerColor(int player, float& r, float& g, float& b, bool* drawLabel = nullptr)
{
    r = 0.4f; g = 0.4f; b = 0.4f;
    if(drawLabel)
        *drawLabel = false;

    if(player < 1 || player > maxLocalPlayers)
        return;

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

void RenderControls(int player, int x, int y, int w, int h, bool missing, float alpha)
{
    if(player < 1 || player > maxLocalPlayers)
        return;

    float alphaBox = 0.7f;
    float alphaBtn = missing ? 0.4f : 0.8f;
    float alphaText = 0.5f;

    alphaBox  *= alpha;
    alphaBtn  *= alpha;
    alphaText *= alpha;

    float r, g, b;
    bool drawLabel;

    GetControllerColor(player, r, g, b, &drawLabel);

    XRender::renderRect(x, y, w, h, 0.f, 0.f, 0.f, alpha, true);//Edge
    XRender::renderRect(x + 2, y + 2, w - 4, h - 4, r, g, b, alpha, true);//Box

    if(missing)
    {
        float tick = (CommonFrame % 128) / 128.0f * 2.0f * static_cast<float>(M_PI);
        float coord = (sinf(tick) + 1.0f) * 0.5f + 0.25f;
        alphaBtn *= coord;
        alphaText *= coord;
    }

    const Controls_t& c = s_displayControls[player-1];

    XRender::renderRect(x + 10, y + 12, 6, 6, 0.f, 0.f, 0.f, alphaBtn, true);//Cender of D-Pad

    XRender::renderRect(x + 10, y + 6, 6, 6, bool2gray(c.Up), alphaBtn, true);
    XRender::renderRect(x + 10, y + 18, 6, 6, bool2gray(c.Down), alphaBtn, true);
    XRender::renderRect(x + 4, y + 12, 6, 6, bool2gray(c.Left), alphaBtn, true);
    XRender::renderRect(x + 16, y + 12, 6, 6, bool2gray(c.Right), alphaBtn, true);

    XRender::renderRect(x + 64, y + 18, 6, 6, bool2green(c.Jump), alphaBtn, true);
    XRender::renderRect(x + 66, y + 8, 6, 6, bool2red(c.AltJump), alphaBtn, true);
    XRender::renderRect(x + 54, y + 16, 6, 6, bool2blue(c.Run), alphaBtn, true);
    XRender::renderRect(x + 56, y + 6, 6, 6, bool2yellow(c.AltRun), alphaBtn, true);

    XRender::renderRect(x + 26, y + 22, 10, 4, bool2gray(c.Drop), alphaBtn, true);
    if(SharedControls.LegacyPause)
        XRender::renderRect(x + 40, y + 22, 10, 4, bool2legacy(c.Start), alphaBtn, true);
    else
        XRender::renderRect(x + 40, y + 22, 10, 4, bool2gray(c.Start), alphaBtn, true);

    if(drawLabel || missing)
    {
        const char* label_fmt = (missing ? "P{0}?" : "P{0}");
        SuperPrintCenter(fmt::format_ne(label_fmt, player), 3, x + w / 2, y + 2, 1.f, 1.f, 1.f, alphaText);
    }
}

void RenderPowerInfo(int player, int bx, int by, int bw, int bh, float alpha, const XPower::StatusInfo* status)
{
    // force to 2x
    bx &= ~1;
    by &= ~1;
    bw &= ~1;
    bh &= ~1;

    float alphaBox = 0.7f;
    float alphaBtn = 0.8f;

    alphaBox *= alpha;
    alphaBtn *= alpha;

    float r, g, b;
    GetControllerColor(player, r, g, b);

    XPower::StatusInfo status_info;

    if(status)
        status_info = *status;
    else if(player >= 1 && player <= maxLocalPlayers)
        status_info = Controls::GetStatus(player);
    else
        status_info = XPower::devicePowerStatus();

    // don't draw segments for states without battery info
    if(status_info.power_status == XPower::StatusInfo::POWER_WIRED || status_info.power_level == XPower::StatusInfo::POWER_UNKNOWN)
        status_info.power_level = 0.0f;

    if(status_info.power_status != XPower::StatusInfo::POWER_DISABLED)
    {
        XRender::renderRect(bx, by, bw - 4, bh, 0.f, 0.f, 0.f, alphaBox, true);//Edge
        XRender::renderRect(bx + 2, by + 2, bw - 8, bh - 4, r, g, b, alphaBox, true);//Box
        XRender::renderRect(bx + 36, by + 6, 4, 10, 0.f, 0.f, 0.f, alphaBox, true);//Edge
        XRender::renderRect(bx + 34, by + 8, 4, 6, r, g, b, alphaBox, true);//Box

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

        if(status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
        {
            g = 1.f;
            g -= r;
        }

        if(status_info.power_status == XPower::StatusInfo::POWER_CHARGED)
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
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 2;
            XRender::renderRect(bx + 34, by + 10, s, 2, r, g, b, alphaBtn, true); // fallthrough
        case 3:
            s = 4.f * (status_info.power_level - 0.6f) / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 24, by + 4, s*2, 14, r, g, b, alphaBtn, true); // fallthrough
        case 2:
            s = 4.f * (status_info.power_level - 0.3f) / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 14, by + 4, s*2, 14, r, g, b, alphaBtn, true); // fallthrough
        case 1:
            s = 4.f * status_info.power_level / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 4, by + 4, s*2, 14, r, g, b, alphaBtn, true);
            break;
        }
        if(status_info.power_status == XPower::StatusInfo::POWER_UNKNOWN)
            SuperPrintCenter("?", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 8) & ~1, 1.0f, 1.0f, 1.0f, alpha);
        if(status_info.power_status == XPower::StatusInfo::POWER_WIRED)
            SuperPrintCenter("W", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 8) & ~1, 1.0f, 1.0f, 1.0f, alpha);
        if(status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            SuperPrintCenter("+", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 7) & ~1, 1.0f, 1.0f, 1.0f, alpha);
    }

    if(status_info.info_string)
        SuperPrintCenter(status_info.info_string, 3, bx + bw / 2, by - 30, 1.0f, 1.0f, 1.0f, alpha);
}

void speedRun_renderControls(int player, int screenZ)
{
    if(GameMenu || GameOutro)
        return; // Don't draw things at Menu and Outro

    if(player < 1 || player > maxLocalPlayers)
        return;

    const bool player_missing = (player - 1 >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[player - 1]);
    const bool player_newly_connected = !player_missing && QuickReconnectScreen::g_active && QuickReconnectScreen::g_toast_duration[player - 1];

    bool rightAlign = false;

    // Controller
    int x = 4;
    int y = ScreenH - 34;
    int w = 76;
    int h = 30;

    // Battery status
    int bx = x + w + 4;
    int by = y + 4;
    int bw = 40;
    int bh = 22;

    if(screenZ >= 0)
    {
        auto &scr = vScreen[screenZ];
        x = scr.Left > 0 ? (int)(scr.Left + scr.Width) - (w + 4) : (int)scr.Left + 4;
        rightAlign = scr.Left > 0;
        y = (int)(scr.Top + scr.Height) - 34;
        bx = scr.Left > 0 ? x - (bw + 4) : (x + w + 4);
        by = y + 4;
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
        switch(player)
        {
        case 1:
            x = 4;
            bx = x + w + 4;
            break;
        case 2:
            x = (ScreenW - (w + 4));
            bx = x - (bw + 4);
            rightAlign = true;
            break;
        }
#endif
    }

    bool show_always = (g_speedRunnerMode != SPEEDRUN_MODE_OFF || g_drawController);
    bool show_controls = false;

    // render controls if enabled or quick-reconnect logic is active
    if(show_always || player_missing || player_newly_connected)
    {
        show_controls = true;
    }
    else
    {
        // reposition battery correctly
        if(x < bx)
            bx = x;
        else
            bx = x + w - bw;
    }

    XPower::StatusInfo status_info = Controls::GetStatus(player);

    float controls_alpha = 1.0f;
    float battery_alpha = 1.0f;

    if(player_newly_connected)
    {
        const Controls::InputMethod* input_method = Controls::g_InputMethods[player - 1];

        const std::string& profile_name = (input_method->Profile ? input_method->Profile->Name : "");

        float alpha = 1.0f;
        int toast_duration = QuickReconnectScreen::g_toast_duration[player - 1];
        int time_from_edge = SDL_min(toast_duration, QuickReconnectScreen::MAX_TOAST_DURATION - toast_duration);

        if(time_from_edge < 33)
        {
            float linear_coord = (33 - time_from_edge) / 33.0f;
            alpha = cosf(linear_coord * M_PI) * 0.5f + 0.5f;

            if(!show_always && toast_duration < 33)
                controls_alpha = alpha;

            // battery just appeared
            if(toast_duration > 33)
                battery_alpha = alpha;
        }

        // code for lower-resolution case
        if(ScreenW < 600 || (ScreenW < 800 && status_info.power_status != XPower::StatusInfo::POWER_DISABLED))
        {
            if(rightAlign)
                SuperPrintRightAlign(profile_name, 3, x + w, y - 20, 1.0f, 1.0f, 1.0f, alpha);
            else
                SuperPrint(profile_name, 3, x, y - 20, 1.0f, 1.0f, 1.0f, alpha);
        }
        // code for higher resolution case, including battery
        else if(status_info.power_status != XPower::StatusInfo::POWER_DISABLED)
        {
            if(rightAlign)
                SuperPrintRightAlign(profile_name, 3, bx - 4, by + 2, 1.0f, 1.0f, 1.0f, alpha);
            else
                SuperPrint(profile_name, 3, bx + bw + 4, by + 2, 1.0f, 1.0f, 1.0f, alpha);
        }
        // code for normal case
        else
        {
            if(rightAlign)
                SuperPrintRightAlign(profile_name, 3, x - 4, by + 2, 1.0f, 1.0f, 1.0f, alpha);
            else
                SuperPrint(profile_name, 3, x + w + 4, by + 2, 1.0f, 1.0f, 1.0f, alpha);
        }
    }

    RenderPowerInfo(player, bx, by, bw, bh, battery_alpha, &status_info);

    if(show_controls)
        RenderControls(player, x, y, w, h, player_missing, controls_alpha);
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
