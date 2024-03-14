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

static constexpr XTColor s_green  = XTColorF(0.0f, 1.0f, 0.0f);
static constexpr XTColor s_blue   = XTColorF(0.0f, 0.0f, 1.0f);
static constexpr XTColor s_red    = XTColorF(1.0f, 0.0f, 0.0f);
static constexpr XTColor s_yellow = XTColorF(0.9f, 0.9f, 0.0f);
static constexpr XTColor s_gray   = XTColorF(0.9f, 0.9f, 0.9f);
static constexpr XTColor s_legacy = XTColorF(0.7f, 0.7f, 0.7f);

static constexpr inline XTColor bool2(XTColor color, bool btn, uint8_t alpha)
{
    return btn ? color.with_alpha(alpha) : XTColor(0, 0, 0, alpha);
}

static Controls_t s_displayControls[maxLocalPlayers] = {Controls_t()};


void speedRun_renderTimer()
{
    if(g_speedRunnerMode == SPEEDRUN_MODE_OFF)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.render();
    SuperPrintRightAlign(fmt::format_ne(Cheater ? "CMode {0}" : "Mode {0}", g_speedRunnerMode), 3, XRender::TargetW - XRender::TargetOverscanX - 2, 2, XTColorF(1.f, 0.3f, 0.3f, 0.5f));
}

static void GetControllerColor(int player, XTColor& color, bool* drawLabel = nullptr)
{
    color = XTColorF(0.4f, 0.4f, 0.4f);
    if(drawLabel)
        *drawLabel = false;

    if(player < 1 || player > maxLocalPlayers)
        return;

    if(numPlayers > 1 && !g_ClonedPlayerMode)
    {
        auto &p = Player[player];

        switch(p.Character) // TODO: Add changing of these colors by gameinfo.ini
        {
        case 1:
            color = XTColorF(0.7f, 0.3f, 0.3f);
            break;
        case 2:
            color = XTColorF(0.3f, 0.7f, 0.3f);
            break;
        case 3:
            color = XTColorF(1.0f, 0.6f, 0.7f);
            break;
        case 4:
            color = XTColorF(0.04f, 0.43f, 1.0f);
            break;
        case 5:
            color = {192, 168, 72};
            break;
        }

        if(drawLabel)
            *drawLabel = true;
    }
}

void RenderControls_priv(int player, const Controls_t* controls, int x, int y, int w, int h, bool missing, uint8_t alpha, bool connect_screen)
{
    if(!controls && (player < 1 || player > maxLocalPlayers))
        return;

    uint8_t alphaBtn = uint8_t((missing ? 0.4f : 0.8f) * alpha);
    uint8_t alphaText = alpha / 2;

    XTColor color;
    bool drawLabel;

    if(!connect_screen)
        GetControllerColor(player, color, &drawLabel);
    else if(player == 0)
        color = XTColorF(0.4f, 0.4f, 0.4f);

    XRender::renderRect(x, y, w, h, {0, 0, 0, alpha}, true);//Edge
    XRender::renderRect(x + 2, y + 2, w - 4, h - 4, {color, alpha}, true);//Box

    if(missing)
    {
        float tick = (CommonFrame % 128) / 128.0f * 2.0f * static_cast<float>(M_PI);
        float coord = (sinf(tick) + 1.0f) * 0.5f + 0.25f;
        alphaBtn *= coord;
        alphaText *= coord;
    }

    const Controls_t& c = (controls) ? *controls : s_displayControls[player - 1];

    XRender::renderRect(x + 10, y + 12, 6, 6, {0, 0, 0, alphaBtn}, true);//Cender of D-Pad

    XRender::renderRect(x + 10, y + 6, 6, 6, bool2(s_gray, c.Up, alphaBtn), true);
    XRender::renderRect(x + 10, y + 18, 6, 6, bool2(s_gray, c.Down, alphaBtn), true);
    XRender::renderRect(x + 4, y + 12, 6, 6, bool2(s_gray, c.Left, alphaBtn), true);
    XRender::renderRect(x + 16, y + 12, 6, 6, bool2(s_gray, c.Right, alphaBtn), true);

    XRender::renderRect(x + 64, y + 18, 6, 6, bool2(s_green, c.Jump, alphaBtn), true);
    XRender::renderRect(x + 66, y + 8, 6, 6, bool2(s_red, c.AltJump, alphaBtn), true);
    XRender::renderRect(x + 54, y + 16, 6, 6, bool2(s_blue, c.Run, alphaBtn), true);
    XRender::renderRect(x + 56, y + 6, 6, 6, bool2(s_yellow, c.AltRun, alphaBtn), true);

    XRender::renderRect(x + 26, y + 22, 10, 4, bool2(s_gray, c.Drop, alphaBtn), true);
    if(SharedControls.LegacyPause)
        XRender::renderRect(x + 40, y + 22, 10, 4, bool2(s_legacy, c.Start, alphaBtn), true);
    else
        XRender::renderRect(x + 40, y + 22, 10, 4, bool2(s_gray, c.Start, alphaBtn), true);

    if(!connect_screen && (drawLabel || missing))
    {
        const char* label_fmt = (missing ? "P{0}?" : "P{0}");
        SuperPrintCenter(fmt::format_ne(label_fmt, player), 3, x + w / 2, y + 2, XTAlpha(alphaText));
    }
    else if(missing)
    {
        SuperPrintCenter("?", 3, x + w / 2, y + 4, XTAlpha(alphaText));
    }
}

void RenderControls(int player, int x, int y, int w, int h, bool missing, uint8_t alpha, bool connect_screen)
{
    return RenderControls_priv(player, nullptr, x, y, w, h, missing, alpha, connect_screen);
}

void RenderControls(const Controls_t& controls, int x, int y, int w, int h, bool missing, uint8_t alpha)
{
    return RenderControls_priv(0, &controls, x, y, w, h, missing, alpha, true);
}



void RenderPowerInfo(int player, int bx, int by, int bw, int bh, uint8_t alpha, const XPower::StatusInfo* status)
{
    // force to 2x
    bx &= ~1;
    by &= ~1;
    bw &= ~1;
    bh &= ~1;

    uint8_t alphaBox = uint8_t(0.7f * alpha);
    uint8_t alphaBtn = uint8_t(0.8f * alpha);

    XTColor color;
    GetControllerColor(player, color);

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
        XRender::renderRect(bx, by, bw - 4, bh, {0, 0, 0, alphaBox}, true);//Edge
        XRender::renderRect(bx + 2, by + 2, bw - 8, bh - 4, {color, alphaBox}, true);//Box
        XRender::renderRect(bx + 36, by + 6, 4, 10, {0, 0, 0, alphaBox}, true);//Edge
        XRender::renderRect(bx + 34, by + 8, 4, 6, {color, alphaBox}, true);//Box

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
        color = XTColor(0, 0, 0);

        if(status_info.power_level <= .5f)
            color.r = (255 - uint8_t(status_info.power_level * 510));

        if(status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
        {
            color.g = 255;
            color.g -= color.r;
        }

        if(status_info.power_status == XPower::StatusInfo::POWER_CHARGED)
        {
            color.b = XTColor::from_float(0.8f);
            color.g = XTColor::from_float(0.4f);
        }

        int s;

        switch(segments)
        {
        case 4:
            s = 2.f * (status_info.power_level - 0.9f) / 0.1f;
            if(s > 2) s = 2;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 2;
            XRender::renderRect(bx + 34, by + 10, s, 2, {color, alphaBtn}, true); // fallthrough
        case 3:
            s = 4.f * (status_info.power_level - 0.6f) / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 24, by + 4, s*2, 14, {color, alphaBtn}, true); // fallthrough
        case 2:
            s = 4.f * (status_info.power_level - 0.3f) / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 14, by + 4, s*2, 14, {color, alphaBtn}, true); // fallthrough
        case 1:
            s = 4.f * status_info.power_level / 0.3f;
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 4, by + 4, s*2, 14, {color, alphaBtn}, true);
            break;
        }
        if(status_info.power_status == XPower::StatusInfo::POWER_UNKNOWN)
            SuperPrintCenter("?", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 8) & ~1, XTAlpha(alpha));
        if(status_info.power_status == XPower::StatusInfo::POWER_WIRED)
            SuperPrintCenter("W", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 8) & ~1, XTAlpha(alpha));
        if(status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            SuperPrintCenter("+", 3, (bx + bw / 2) & ~1, (by + bh / 2 - 7) & ~1, XTAlpha(alpha));
    }

    if(status_info.info_string)
        SuperPrintCenter(status_info.info_string, 3, bx + bw / 2, by - 30, XTAlpha(alpha));
}

void speedRun_renderControls(int player, int screenZ, int align)
{
    if(GameMenu || GameOutro || LevelEditor)
        return; // Don't draw things at Menu and Outro

    if(player < 1 || player > maxLocalPlayers)
        return;

    if(SingleCoop)
        player = 1;

    const bool player_missing = (player - 1 >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[player - 1]);
    const bool player_newly_connected = !player_missing && QuickReconnectScreen::g_active && QuickReconnectScreen::g_toast_duration[player - 1];
    XPower::StatusInfo status_info = Controls::GetStatus(player);

    // Controller
    int x, y;
    int w = 76;
    int h = 30;

    // Battery status
    int bx, by;
    int bw = 40;
    int bh = 22;

    int num_players = 1;

    if(screenZ >= 0)
    {
        auto &scr = vScreen[screenZ];
        y = (int)scr.Height - 34;
        by = y + 4;

        if(align == SPEEDRUN_ALIGN_AUTO)
            align = scr.Left > 0 ? SPEEDRUN_ALIGN_RIGHT : SPEEDRUN_ALIGN_LEFT;

        // this keeps the locations fixed even when the vScreens expand/contract
        if(align == SPEEDRUN_ALIGN_LEFT)
        {
            x = 4;
            bx = x + w + 4;
        }
        else
        {
            x = (int)scr.Width - (w + 4);
            bx = x - (bw + 4);
        }
    }
    else
    {
        const Screen_t& plr_screen = ScreenByPlayer(player);
        num_players = plr_screen.player_count;
        int plr_i = 0;
        for(; plr_i < plr_screen.player_count; plr_i++)
        {
            if(plr_screen.players[plr_i] == player)
                break;
        }


        if(align == SPEEDRUN_ALIGN_AUTO)
        {
            if(plr_i == 0 || num_players <= 1)
                align = SPEEDRUN_ALIGN_LEFT;
            else if(plr_i >= num_players - 1)
                align = SPEEDRUN_ALIGN_RIGHT;
        }

        if(align == SPEEDRUN_ALIGN_LEFT)
        {
            x = XRender::TargetOverscanX + 4;
            bx = x + w + 4;
        }
        else if(align == SPEEDRUN_ALIGN_RIGHT)
        {
            x = (XRender::TargetW - XRender::TargetOverscanX - (w + 4));
            bx = x - (bw + 4);
        }
        else
        {
            int display_w = (status_info.power_status != XPower::StatusInfo::POWER_DISABLED) ? (w + 4 + bw + 4) : (w + 4);

            x = (XRender::TargetW - display_w) * plr_i / (num_players - 1);
            bx = x + (w + 4);
        }

        y = XRender::TargetH - 34;
        by = y + 4;
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

    uint8_t controls_alpha = 255;
    uint8_t battery_alpha = 255;

    if(player_newly_connected)
    {
        const Controls::InputMethod* input_method = Controls::g_InputMethods[player - 1];

        const std::string& profile_name = (input_method->Profile ? input_method->Profile->Name : "");

        uint8_t alpha = 255;
        int toast_duration = QuickReconnectScreen::g_toast_duration[player - 1];
        int time_from_edge = SDL_min(toast_duration, QuickReconnectScreen::MAX_TOAST_DURATION - toast_duration);

        if(time_from_edge < 33)
        {
            float linear_coord = (33 - time_from_edge) / 33.0f;
            alpha = XTColor::from_float(cosf(linear_coord * M_PI) * 0.5f + 0.5f);

            if(!show_always && toast_duration < 33)
                controls_alpha = alpha;

            // battery just appeared
            if(toast_duration > 33)
                battery_alpha = alpha;
        }

        // code for lower-resolution case
        if(XRender::TargetW < 600 || (XRender::TargetW < 800 && status_info.power_status != XPower::StatusInfo::POWER_DISABLED) || num_players > 2)
        {
            if(align == SPEEDRUN_ALIGN_RIGHT)
                SuperPrintRightAlign(profile_name, 3, x + w, y - 20, XTAlpha(alpha));
            else if(align == SPEEDRUN_ALIGN_LEFT)
                SuperPrint(profile_name, 3, x, y - 20, XTAlpha(alpha));
            else
                SuperPrintCenter(profile_name, 3, x + w / 2, y - 20, XTAlpha(alpha));
        }
        // code for higher resolution case, including battery
        else if(status_info.power_status != XPower::StatusInfo::POWER_DISABLED)
        {
            if(align == SPEEDRUN_ALIGN_RIGHT)
                SuperPrintRightAlign(profile_name, 3, bx - 4, by + 2, XTAlpha(alpha));
            else if(align == SPEEDRUN_ALIGN_LEFT)
                SuperPrint(profile_name, 3, bx + bw + 4, by + 2, XTAlpha(alpha));
            else
                SuperPrintCenter(profile_name, 3, x + w, y - 20, XTAlpha(alpha));
        }
        // code for normal case
        else
        {
            if(align == SPEEDRUN_ALIGN_RIGHT)
                SuperPrintRightAlign(profile_name, 3, x - 4, by + 2, XTAlpha(alpha));
            else if(align == SPEEDRUN_ALIGN_LEFT)
                SuperPrint(profile_name, 3, x + w + 4, by + 2, XTAlpha(alpha));
            else
                SuperPrintCenter(profile_name, 3, x + w / 2, y - 20, XTAlpha(alpha));
        }
    }

    RenderPowerInfo(player, bx, by, bw, bh, battery_alpha, &status_info);

    if(show_controls)
        RenderControls(player, x, y, w, h, player_missing, controls_alpha);
}

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
