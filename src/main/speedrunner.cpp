/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "config.h"
#include "../controls.h"

#include "main/screen_quickreconnect.h"
#include "main/menu_main.h"

#include "gameplay_timer.h"

static      GameplayTimer s_gamePlayTimer;

void speedRun_loadStats()
{
    if(!g_config.enable_playtime_tracking)
        return; // Do nothing

    s_gamePlayTimer.load();
}

void speedRun_saveStats()
{
    if(!g_config.enable_playtime_tracking)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Do nothing when out of the game

    s_gamePlayTimer.save();
}

void speedRun_resetCurrent()
{
    if(!g_config.enable_playtime_tracking)
        return; // Do nothing

    s_gamePlayTimer.resetCurrent();
}

void speedRun_resetTotal()
{
    if(!g_config.enable_playtime_tracking)
        return; // Do nothing

    s_gamePlayTimer.reset();
}

static constexpr XTColor s_green  = XTColorF(0.0_n, 1.0_n, 0.0_n);
static constexpr XTColor s_blue   = XTColorF(0.0_n, 0.0_n, 1.0_n);
static constexpr XTColor s_red    = XTColorF(1.0_n, 0.0_n, 0.0_n);
static constexpr XTColor s_yellow = XTColorF(0.9_n, 0.9_n, 0.0_n);
static constexpr XTColor s_gray   = XTColorF(0.9_n, 0.9_n, 0.9_n);
static constexpr XTColor s_legacy = XTColorF(0.7_n, 0.7_n, 0.7_n);

static constexpr inline XTColor bool2(XTColor color, bool btn, uint8_t alpha)
{
    return btn ? color.with_alpha(alpha) : XTColor(32, 32, 32, alpha);
}

static Controls_t s_displayControls[maxLocalPlayers] = {Controls_t()};


void speedRun_renderTimer()
{
    if((g_config.speedrun_mode == SPEEDRUN_MODE_OFF && g_config.show_playtime_counter == Config_t::PLAYTIME_COUNTER_OFF) || !g_config.enable_playtime_tracking)
        return; // Do nothing

    if(g_config.speedrun_mode != SPEEDRUN_MODE_OFF)
    {
        // place speedrun mode below game version display on main menu
        int topY = (GameMenu) ? 22 : 2;

        SuperPrintRightAlign(fmt::format_ne(Cheater ? "CMode {0}" : "Mode {0}", g_config.speedrun_mode), 3, XRender::TargetW - XRender::TargetOverscanX - 2, topY, XTColorF(1.0_n, 0.3_n, 0.3_n, 0.5_n));

        if(g_config.allow_multires)
            SuperPrintRightAlign(fmt::format_ne("{0}x{1}", l_screen->W, l_screen->H), 3, XRender::TargetW - XRender::TargetOverscanX - 2, topY + 20, XTColorF(1.0_n, 0.3_n, 0.3_n, 0.5_n));
    }

    if(GameMenu || GameOutro || BattleMode || LevelEditor)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.render();
}

static void GetControllerColor(int l_player_i, XTColor& color, bool* drawLabel = nullptr)
{
    color = XTColorF(0.4_n, 0.4_n, 0.4_n);
    if(drawLabel)
        *drawLabel = false;

    if(GameMenu)
        return;

    if(l_player_i < 0 || l_player_i >= l_screen->player_count)
        return;

    int player = l_screen->players[l_player_i];

    if(player < 1 || player > numPlayers)
        return;

    if(numPlayers > 1 && !g_ClonedPlayerMode)
    {
        auto &p = Player[player];

        switch(p.Character) // TODO: Add changing of these colors by gameinfo.ini
        {
        case 1:
            color = XTColorF(0.7_n, 0.3_n, 0.3_n);
            break;
        case 2:
            color = XTColorF(0.3_n, 0.7_n, 0.3_n);
            break;
        case 3:
            color = XTColorF(1.0_n, 0.6_n, 0.7_n);
            break;
        case 4:
            color = XTColorF(0.04_n, 0.43_n, 1.0_n);
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
    if(!controls && (player < 0 || player >= maxLocalPlayers))
        return;

    uint8_t alphaBtn = missing ? alpha / 2 : alpha;
    uint8_t alphaText = alpha / 2;

    XTColor color;
    bool drawLabel = false;

    if(!connect_screen)
        GetControllerColor(player, color, &drawLabel);
    else if(player == -1)
        color = XTColorF(0.4_n, 0.4_n, 0.4_n);

    XRender::renderRect(x, y, w, h, {0, 0, 0, alpha}, true);//Edge
    XRender::renderRect(x + 2, y + 2, w - 4, h - 4, {color, alpha}, true);//Box

    if(missing)
    {
        num_t tick = 2 * num_t::PI() * (CommonFrame % 128) / 128;
        num_t coord = (num_t::sin(tick) + 1) / 2 + 0.25_n;
        alphaBtn = (uint8_t)(coord * alphaBtn);
        alphaText = (uint8_t)(coord * alphaText);
    }

    const Controls_t& c = (controls) ? *controls : s_displayControls[player];

    XRender::renderRect(x + 10, y + 12, 6, 6, {32, 32, 32, alphaBtn}, true);//Cender of D-Pad

    XRender::renderRect(x + 10, y + 6, 6, 6, bool2(s_gray, c.Up, alphaBtn), true);
    XRender::renderRect(x + 10, y + 18, 6, 6, bool2(s_gray, c.Down, alphaBtn), true);
    XRender::renderRect(x + 4, y + 12, 6, 6, bool2(s_gray, c.Left, alphaBtn), true);
    XRender::renderRect(x + 16, y + 12, 6, 6, bool2(s_gray, c.Right, alphaBtn), true);

    // This was suggested before, but perhaps don't do it: AltJump is still used for dismounting.
    // auto altjump_color = (g_config.disable_spin_jump) ? s_green : s_red;

    XRender::renderRect(x + 64, y + 18, 6, 6, bool2(s_green, c.Jump, alphaBtn), true);
    XRender::renderRect(x + 66, y + 8, 6, 6, bool2(s_red, c.AltJump, alphaBtn), true);
    XRender::renderRect(x + 54, y + 16, 6, 6, bool2(s_blue, c.Run, alphaBtn), true);
    XRender::renderRect(x + 56, y + 6, 6, 6, bool2(s_yellow, c.AltRun, alphaBtn), true);

    XRender::renderRect(x + 26, y + 22, 10, 4, bool2(s_gray, c.Drop, alphaBtn), true);
    if(SharedPauseLegacy)
        XRender::renderRect(x + 40, y + 22, 10, 4, bool2(s_legacy, c.Start, alphaBtn), true);
    else
        XRender::renderRect(x + 40, y + 22, 10, 4, bool2(s_gray, c.Start, alphaBtn), true);

    if(!connect_screen && (drawLabel || missing))
    {
        const char* label_fmt = (missing ? "P{0}?" : "P{0}");
        SuperPrintCenter(fmt::format_ne(label_fmt, player + 1), 3, x + w / 2, y + 2, XTAlpha(alphaText));
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
    return RenderControls_priv(-1, &controls, x, y, w, h, missing, alpha, true);
}



void RenderPowerInfo(int player, int bx, int by, int bw, int bh, uint8_t alpha, const XPower::StatusInfo* status)
{
    // force to 2x
    bx &= ~1;
    by &= ~1;
    bw &= ~1;
    bh &= ~1;

    uint8_t alphaBox = alpha;
    uint8_t alphaBtn = alpha;

    XTColor color;
    GetControllerColor(player, color);
    if(color.r >= 128 || color.g >= 128 || color.b >= 128)
    {
        // make it darker so it can be distinguished from segments
        color.r /= 2;
        color.g /= 2;
        color.b /= 2;
    }

    XPower::StatusInfo status_info;

    if(status)
        status_info = *status;
    else if(player >= 0 && player < maxLocalPlayers)
        status_info = Controls::GetStatus(player);
    else
        status_info = XPower::devicePowerStatus();

    // don't draw segments for states without battery info
    if(status_info.power_status == XPower::StatusInfo::POWER_WIRED || status_info.power_status == XPower::StatusInfo::POWER_UNKNOWN)
        status_info.power_level = 0;

    if(status_info.power_status != XPower::StatusInfo::POWER_DISABLED)
    {
        XRender::renderRect(bx, by, bw - 4, bh, {0, 0, 0, alphaBox}, true);//Edge
        XRender::renderRect(bx + 2, by + 2, bw - 8, bh - 4, {color, alphaBox}, true);//Box
        XRender::renderRect(bx + 36, by + 6, 4, 10, {0, 0, 0, alphaBox}, true);//Edge
        XRender::renderRect(bx + 34, by + 8, 4, 6, {color, alphaBox}, true);//Box

        // segment count
        int segments;

        if(status_info.power_level > 0.90_nf)
            segments = 4;
        else if(status_info.power_level > 0.60_nf)
            segments = 3;
        else if(status_info.power_level > 0.30_nf)
            segments = 2;
        else
            segments = 1;

        // new color for the inner segments
        color = XTColor(0, 0, 0);

        if(status_info.power_level <= 0.5_nf)
            color.r = (255 - uint8_t(status_info.power_level * 510));

        if(status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
        {
            color.g = 255;
            color.g -= color.r;
        }

        if(status_info.power_status == XPower::StatusInfo::POWER_CHARGED)
        {
            color.b = XTColor::from_num(0.8_n);
            color.g = XTColor::from_num(0.4_n);
        }

        // size for segment
        int s;

        switch(segments)
        {
        case 4:
            // 2 is the width of the segment, 10 lets us scale to the last 0.1_nf
            s = (int32_t)((status_info.power_level - 0.9_nf) * (2 * 10));
            if(s > 2) s = 2;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 2;
            XRender::renderRect(bx + 34, by + 10, s, 2, {color, alphaBtn}, true); // fallthrough
        case 3:
            // 4 is the width of the segment, 10 / 3 lets us scale to the 0.6-0.9 segment
            s = (int32_t)((status_info.power_level - 0.6_nf) * (4 * 10) / 3);
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 24, by + 4, s*2, 14, {color, alphaBtn}, true); // fallthrough
        case 2:
            // 4 is the width of the segment, 10 / 3 lets us scale to the 0.3-0.6 segment
            s = (int32_t)((status_info.power_level - 0.3_nf) * (4 * 10) / 3);
            if(s > 4) s = 4;
            // if(flash && status_info.power_status == XPower::StatusInfo::POWER_CHARGING)
            //     s = 4;
            XRender::renderRect(bx + 14, by + 4, s*2, 14, {color, alphaBtn}, true); // fallthrough
        case 1:
            // 4 is the width of the segment, 10 / 3 lets us scale to the 0.0-0.3 segment
            s = (int32_t)(status_info.power_level * (4 * 10) / 3);
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
    if(GameOutro || LevelEditor)
        return; // Don't draw things at Editor and Outro

    if(GameMenu && (!g_config.show_controllers || MenuMode == MENU_CHARACTER_SELECT_NEW || MenuMode == MENU_CHARACTER_SELECT_NEW_BM))
        return;

    // player is an index into the local screen's player array, not a global player index
    if(player < 0 || player >= maxLocalPlayers)
        return;

    if(SingleCoop)
        player = 0;

    const bool player_missing = (player >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[player]);
    const bool player_newly_connected = !player_missing && QuickReconnectScreen::g_active && QuickReconnectScreen::g_toast_duration[player];
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
        const Screen_t& plr_screen = *l_screen;
        num_players = plr_screen.player_count;

        int plr_i = player;

        if(GameMenu)
        {
            num_players = SDL_min(num_players, (int)Controls::g_InputMethods.size());

            if(plr_i >= num_players)
                return;
        }

        if(align == SPEEDRUN_ALIGN_AUTO && !GameMenu)
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
        else if(GameMenu)
        {
            x = XRender::TargetOverscanX + 4 + (w + 4) * plr_i;
            bx = x + w + 4;
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

    bool show_always = g_config.show_controllers;
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
        const Controls::InputMethod* input_method = Controls::g_InputMethods[player];

        const std::string& profile_name = (input_method->Profile ? input_method->Profile->Name : "");

        uint8_t alpha = 255;
        int toast_duration = QuickReconnectScreen::g_toast_duration[player];
        int time_from_edge = SDL_min(toast_duration, QuickReconnectScreen::MAX_TOAST_DURATION - toast_duration);

        if(time_from_edge < 33)
        {
            num_t linear_coord = num_t::PI() * (33 - time_from_edge) / 33;
            XTColor alpha = XTAlphaF(num_t::cos(linear_coord) / 2 + 0.5_n);

            if(!show_always && toast_duration < 33)
                controls_alpha = alpha.a;

            // battery just appeared
            if(toast_duration > 33)
                battery_alpha = alpha.a;
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

    if(!GameMenu)
        RenderPowerInfo(player, bx, by, bw, bh, battery_alpha, &status_info);

    if(show_controls)
        RenderControls(player, x, y, w, h, player_missing, controls_alpha);
}

void speedRun_tick()
{
    if(!g_config.enable_playtime_tracking)
        return; // Do nothing

    s_gamePlayTimer.tick();
}

void speedRun_triggerEnter()
{
    if(!g_config.enable_playtime_tracking)
        return; // Do nothing

    if(g_config.speedrun_stop_timer_by != Config_t::SPEEDRUN_STOP_ENTER_LEVEL)
        return;

    if(SDL_strcasecmp(FileName.c_str(), static_cast<const std::string&>(g_config.speedrun_stop_timer_at).c_str()) == 0)
        speedRun_bossDeadEvent();
}

void speedRun_triggerLeave()
{
    if(!g_config.enable_playtime_tracking)
        return; // Do nothing

    if(g_config.speedrun_stop_timer_by != Config_t::SPEEDRUN_STOP_LEAVE_LEVEL)
        return;

    if(SDL_strcasecmp(FileName.c_str(), static_cast<const std::string&>(g_config.speedrun_stop_timer_at).c_str()) == 0)
        speedRun_bossDeadEvent();
}


void speedRun_bossDeadEvent()
{
    if(!g_config.enable_playtime_tracking)
        return; // Do nothing

    if(GameMenu || GameOutro || BattleMode)
        return; // Don't draw things at Menu and Outro

    s_gamePlayTimer.onBossDead();
}

void speedRun_syncControlKeys(int plr, const Controls_t &keys)
{
    // there are still reasons to sync control keys (eg control tests)
    // if(g_speedRunnerMode == SPEEDRUN_MODE_OFF && !g_drawController)
    //     return; // Do nothing

    SDL_assert(plr >= 0 && plr < maxLocalPlayers);
    SDL_memcpy(&s_displayControls[plr], &keys, sizeof(Controls_t));
}
