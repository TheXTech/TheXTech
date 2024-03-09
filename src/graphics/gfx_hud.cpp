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

#include <fmt_format_ne.h>
#include "sdl_proxy/sdl_stdinc.h"

#include "../globals.h"
#include "../graphics.h"
#include "../core/render.h"
#include "../core/power.h"
#include "../gfx.h"
#include "npc_traits.h"

#include "video.h"
#include "main/speedrunner.h"

#include "compat.h"
#include "config.h"
#include "main/level_medals.h"

void DrawInterface(int Z, int numScreens)
{
    int ScreenTop = 0;
    if(vScreen[Z].Height > 600)
        ScreenTop = vScreen[Z].Height / 2 - 300;
    int CenterX = vScreen[Z].Width / 2;

    const Screen_t& screen = Screens[vScreen[Z].screen_ref];

    std::string scoreStr = std::to_string(Score);
    std::string coinsStr = std::to_string(Coins);
    std::string numStarsStr = std::to_string(numStars);

    XRender::offsetViewportIgnore(true);

    bool is_multiplayer = screen.player_count > 1;
    bool shared_screen = (is_multiplayer && numScreens == 1 && screen.Type != 6);

    // number of players to render on this vScreen
    int plr_count = shared_screen ? screen.player_count : 1;

    // do any of the players on this vScreen have a key?
    bool someone_has_key = false;

    // first onscreen player index, affects coin icon and battle mode lives
    int first_plr_idx = 0;

    // bigger player HUD sizes in 1P/2P for compatibility purposes
    int plr_hud_width = plr_count == 1 ? 148 : (114 * plr_count);
    if(plr_count > 2)
        plr_hud_width = 84 * plr_count;

    // left and right sides of hearts / held item box section
    int left_margin = CenterX - plr_hud_width / 2;
    int right_margin = CenterX + plr_hud_width / 2;

    // loop over players and draw them
    for(int i = 0; i < plr_count; i++)
    {
        // look up player index using either screen (shared) or vScreen (non-shared)
        int plr_idx = shared_screen ? screen.players[i] : vScreen[Z].player;
        Player_t& plr = Player[plr_idx];

        if(i == 0)
            first_plr_idx = plr_idx;

        // find center for player HUD info; this is the center of the i'th portion of the HUD (out of plr_count portions)
        int plr_center = left_margin + (plr_hud_width * (i * 2 + 1)) / (plr_count * 2);

        // show hearts
        if(plr.Character == 3 || plr.Character == 4 || plr.Character == 5)
        {
            auto& heart_1_gfx = plr.Hearts > 0 ? GFX.Heart[1] : GFX.Heart[2];
            auto& heart_2_gfx = plr.Hearts > 1 ? GFX.Heart[1] : GFX.Heart[2];
            auto& heart_3_gfx = plr.Hearts > 2 ? GFX.Heart[1] : GFX.Heart[2];

            int heart_offset = (plr_count > 2) ? 24 : 32;

            XRender::renderTexture(plr_center - heart_1_gfx.w / 2 - heart_offset, ScreenTop + 16, heart_1_gfx);
            XRender::renderTexture(plr_center - heart_2_gfx.w / 2               , ScreenTop + 16, heart_2_gfx);
            XRender::renderTexture(plr_center - heart_3_gfx.w / 2 + heart_offset, ScreenTop + 16, heart_3_gfx);
        }
        // show held bonus (item box)
        else
        {
            // held bonus offset, bring it towards the screen center in 2P
            if(plr_count == 2)
                plr_center += (i == 0) ? 17 : -17;

            int use_container = plr.Character;

            if(!is_multiplayer || use_container > 2)
                use_container = 0;

            XRender::renderTexture(plr_center - GFX.Container[1].w / 2, ScreenTop + 16, GFX.Container[use_container]);

            if(plr.HeldBonus > 0)
            {
                XRender::renderTexture(plr_center - GFX.Container[1].w / 2 + 12,
                                        ScreenTop + 16 + 12,
                                        NPCWidth(plr.HeldBonus), NPCHeight(plr.HeldBonus),
                                        GFXNPC[plr.HeldBonus], 0, 0);
            }
        }

        // show bombs
        if(plr.Character == 5 && plr.Bombs > 0)
        {
            // offset for bomb text, just for compatibility
            int off = (plr_count == 2) ? -1 : 0;

            XRender::renderTexture(20 + plr_center - 28 - 34 + off, ScreenTop + 52,
                GFX.Interface[8]);
            XRender::renderTexture(20 + plr_center - 28 - 10 + off, ScreenTop + 53,
                GFX.Interface[1]);
            SuperPrint(std::to_string(plr.Bombs), 1,
                       float(20 + plr_center - 28 + 12 + off),
                       ScreenTop + 53);
        }

        if(plr.HasKey)
            someone_has_key = true;
    }

    // slightly expand margins for >2P
    if(plr_count > 2)
    {
        left_margin -= 4;
        right_margin += 12;
    }

    // draw non-battle mode info
    if(!BattleMode)
    {
        int coins_x = right_margin + 14;

        // Indicate key
        if(someone_has_key)
            XRender::renderTexture(coins_x - 24, ScreenTop + 16 + 10, GFX.Interface[0]);

        // Print coins on the screen
        auto& coin_icon = (Player[first_plr_idx].Character == 5) ? GFX.Interface[6] : GFX.Interface[2];
        XRender::renderTexture(coins_x, ScreenTop + 16 + 10, coin_icon);

        XRender::renderTexture(coins_x + coin_icon.w + 8, ScreenTop + 16 + 11, GFX.Interface[1]);

        // note: the 36px at end gives 8px of padding between "x" icon and text when there are 2 digits
        int coins_score_text_right = (coins_x + coin_icon.w + 8 + GFX.Interface[1].w + 8 + 36);

        int coins_length = int(coinsStr.size()) * 18;
        SuperPrint(coinsStr, 1,
                   coins_score_text_right - coins_length,
                   ScreenTop + 16 + 11);

        // Print score below coins (match right align)
        int score_length = int(scoreStr.size()) * 18;
        SuperPrint(scoreStr, 1,
                   coins_score_text_right - score_length,
                   ScreenTop + 16 + 31);

        // Print lives on the screen
        int lives_stars_x = left_margin - 60; // excludes life / star icon width
        int lives_stars_text_left = lives_stars_x + 8 + GFX.Interface[1].w + 8;

        DrawLives(lives_stars_x, ScreenTop + 16 + 10, Lives, g_100s);

        // Print stars on the screen
        if(numStars > 0)
        {
            XRender::renderTexture(lives_stars_x - GFX.Interface[5].w, ScreenTop + 16 + 30, GFX.Interface[5]);
            XRender::renderTexture(lives_stars_x + 8, ScreenTop + 16 + 31, GFX.Interface[1]);

            SuperPrint(numStarsStr, 1,
                       lives_stars_text_left,
                       ScreenTop + 16 + 31);
        }
    }
    // draw both battle mode lives
    else if(shared_screen)
    {
        // plr 1 lives
        int plr1_lives_x = left_margin - 60; // excludes life icon width
        XRender::renderTexture(plr1_lives_x - GFX.Interface[3].w, ScreenTop + 16 + 10, GFX.Interface[3]);
        XRender::renderTexture(plr1_lives_x + 8, ScreenTop + 16 + 11, GFX.Interface[1]);
        SuperPrint(std::to_string(BattleLives[screen.players[0]]), 1,
                   plr1_lives_x + 8 + GFX.Interface[1].w + 8,
                   ScreenTop + 16 + 11);

        // plr 2 lives
        int plr2_lives_x = right_margin + 30; // excludes life icon width
        XRender::renderTexture(plr2_lives_x - GFX.Interface[7].w, ScreenTop + 16 + 10, GFX.Interface[7]);
        XRender::renderTexture(plr2_lives_x + 8, ScreenTop + 16 + 11, GFX.Interface[1]);
        SuperPrint(std::to_string(BattleLives[screen.players[1]]), 1,
                   float(plr2_lives_x + 8 + GFX.Interface[1].w + 8),
                   ScreenTop + 16 + 11);
    }
    // draw 1P battle mode lives
    else
    {
        // lives in battle mode
        int lives_x = left_margin - 60; // excludes life icon width
        auto& oneup_twoup = (first_plr_idx == 1) ? GFX.Interface[3] : GFX.Interface[7];

        XRender::renderTexture(lives_x - oneup_twoup.w,
                              ScreenTop + 16 + 10,
                              oneup_twoup);

        XRender::renderTexture(lives_x + 8,
            ScreenTop + 16 + 11,
            GFX.Interface[1]);

        SuperPrint(std::to_string(BattleLives[first_plr_idx]), 1,
                   lives_x + 8 + GFX.Interface[1].w + 8,
                   ScreenTop + 16 + 11);
    }

    if(!IsHubLevel && !BattleMode)
    {
        // draw medals at top-right side of HUD
        int medals_x = SDL_min(static_cast<int>(vScreen[Z].Width) - 16, CenterX + 400 - 16);
        int medals_y = ScreenTop + 16;

        DrawMedals(medals_x, medals_y, false, g_curLevelMedals.max, g_curLevelMedals.prev, 0, g_curLevelMedals.got, g_curLevelMedals.life);
    }

    if(BattleIntro > 0)
    {
        if(BattleIntro > 45 || BattleIntro % 2 == 1)
        {
            auto& P1_charname = GFX.CharacterName[Player[1].Character];
            auto& P2_charname = GFX.CharacterName[Player[2].Character];

            XRender::renderTexture(vScreen[Z].Width / 2.0 - GFX.BMVs.w / 2, -96 + vScreen[Z].Height / 2.0 - GFX.BMVs.h / 2, GFX.BMVs);
            XRender::renderTexture(-50 + vScreen[Z].Width / 2.0 - P1_charname.w, -96 + vScreen[Z].Height / 2.0 - P1_charname.h / 2, P1_charname);
            XRender::renderTexture(50 + vScreen[Z].Width / 2.0, -96 + vScreen[Z].Height / 2.0 - P2_charname.h / 2, P2_charname);
        }
    }

    if(BattleOutro > 0)
    {
        auto& win_charname = GFX.CharacterName[Player[BattleWinner].Character];

        XRender::renderTexture(10 + vScreen[Z].Width / 2.0, -96 + vScreen[Z].Height / 2.0 - GFX.BMWin.h / 2, GFX.BMWin);
        XRender::renderTexture(-10 + vScreen[Z].Width / 2.0 - win_charname.w, -96 + vScreen[Z].Height / 2.0 - win_charname.h / 2, win_charname);
    }

    XRender::offsetViewportIgnore(false);
}

void DrawLives(int X, int Y, int lives, int hunds)
{
    bool show_times = true;
    int count = 0;
    int text_X = X + 8 + GFX.Interface[1].w + 8;

    if(g_compatibility.modern_lives_system)
    {
        bool debt = (hunds < 0);
        count = (debt) ? -hunds : hunds;

        if(GFX.Balance.inited)
            XRender::renderTexture(X - GFX.Balance.w / 2, Y, GFX.Balance.w / 2, GFX.Balance.h, GFX.Balance, debt * GFX.Balance.w / 2, 0);
        else
        {
            XRender::renderTexture(X - GFX.Interface[2].w,      Y, GFX.Interface[2], XTColor(0x7F, 0x7F * !debt, 0x7F * !debt));
            XRender::renderTexture(X - GFX.Interface[2].w - 8,  Y, GFX.Interface[2], XTColor(0xBF, 0xBF * !debt, 0xBF * !debt));
            XRender::renderTexture(X - GFX.Interface[2].w - 16, Y, GFX.Interface[2], XTColor(0xFF, 0xFF * !debt, 0xFF * !debt));
        }

        if(count >= 100)
        {
            show_times = false;
            text_X -= 18;
        }

        if(count >= 1000)
            text_X -= 10;
    }
    else
    {
        count = lives;
        XRender::renderTexture(X - GFX.Interface[3].w, Y, GFX.Interface[3]);
    }

    if(show_times)
        XRender::renderTexture(X + 8, Y + 1, GFX.Interface[1]);

    SuperPrint(std::to_string(count), 1,
               text_X,
               Y + 1);
}

enum class MedalDrawLevel
{
    Off = 0, Prev, Got, Shiny
};

//! helper function to draw a single medal at a specific top-left position and acquisition level
static inline void s_DrawMedal(int i, int x, int y, int coin_width, int coin_height, MedalDrawLevel level)
{
    if(GFX.Medals.inited)
    {
        XRender::renderTexture(x, y, coin_width, coin_height, GFX.Medals, coin_width * (int)level, 0);
    }
    else
    {
        if(level == MedalDrawLevel::Shiny || level == MedalDrawLevel::Got)
            XRender::renderTexture(x, y, GFX.Interface[2]);
        else if(level == MedalDrawLevel::Prev)
            XRender::renderTexture(x, y, GFX.Interface[2], XTColorF(0.5f, 0.5f, 0.5f));
        else
            XRender::renderTexture(x, y, GFX.Interface[2], XTColorF(0.5f, 0.5f, 0.5f, 0.5f));
    }

    // render sparkles for shiny
    if(level == MedalDrawLevel::Shiny && coin_width > 8 && coin_height > 8)
    {
        int sparkle_1_idx = ((CommonFrame + i * 16 * 37) % 1024) / 16; // on frame 3

        for(int i = 0; i < 3; ++i)
        {
            int sparkle_idx = (sparkle_1_idx + i) % 64;

            if(sparkle_idx % 2)
                continue;

            int sparkle_frame = 2 - i;

            int sparkle_X = (9 * sparkle_idx) % (coin_width - 8) + 4;
            int sparkle_Y = (13 * (sparkle_idx % 16) + 29 * (sparkle_idx / 16)) % (coin_height - 8) + 4;

            sparkle_X -= EffectWidth[78] / 2;
            sparkle_Y -= EffectHeight[78] / 2;

            sparkle_X &= ~1;
            sparkle_Y &= ~1;

            XRender::renderTexture(x + sparkle_X, y + sparkle_Y, EffectWidth[78], EffectHeight[78], GFXEffect[78], 0, EffectHeight[78] * sparkle_frame, XTAlphaF(0.8f));
        }
    }
}

void DrawMedals(int X, int Y, bool warp, uint8_t max, uint8_t prev, uint8_t ckpt, uint8_t got, uint8_t best)
{
    if(g_config.medals_show_policy == Config_t::MEDALS_SHOW_OFF)
        return;

    if(g_config.medals_show_policy == Config_t::MEDALS_SHOW_GOT && got == 0)
        return;

    if(max == 0)
        return;

    int coin_width = GFX.Interface[2].w;
    int coin_height = GFX.Interface[2].h;

    if(GFX.Medals.inited)
    {
        coin_width = GFX.Medals.w / 4;
        coin_height = GFX.Medals.h;
    }

    if(max > c_max_track_medals)
        max = c_max_track_medals;

    // don't spoil the maximum count, make it shiny if all of the discovered medals are shiny
    bool show_max = g_config.medals_show_policy >= Config_t::MEDALS_SHOW_COUNTS;

    // whether to use the shiny effect for medals; for warps, check if best is all 1s (up to bit max); in HUD, always draw shiny medals
    bool show_shiny = (warp && show_max) ? (best == ((1 << max) - 1)) : true;

    // slot-based display
    if(g_config.medals_show_policy == Config_t::MEDALS_SHOW_FULL)
    {
        // position scene
        if(warp)
            X -= ((coin_width * max) / 2) & ~1;
        else
            X -= (coin_width * max);

        // draw coins
        for(int i = 0; i < max; ++i)
        {
            int bit = (1 << i);
            int X_i = X + coin_width * i;

            if((best & bit) && show_shiny)
                s_DrawMedal(i, X_i, Y, coin_width, coin_height, MedalDrawLevel::Shiny);
            else if(got & bit)
                s_DrawMedal(i, X_i, Y, coin_width, coin_height, MedalDrawLevel::Got);
            else if(ckpt & bit && (CommonFrame % 64) < 32)
                s_DrawMedal(i, X_i, Y, coin_width, coin_height, MedalDrawLevel::Got);
            else if(prev & bit)
                s_DrawMedal(i, X_i, Y, coin_width, coin_height, MedalDrawLevel::Prev);
            else
                s_DrawMedal(i, X_i, Y, coin_width, coin_height, MedalDrawLevel::Off);
        }

        return;
    }

    // text-based display

    // get counts
    int best_count = 0;
    int got_count = 0;
    int prev_count = 0;

    for(int i = 0; i < max; ++i)
    {
        int bit = (1 << i);

        if(best & bit)
            best_count++;
        if(got & bit)
            got_count++;
        if(prev & bit)
            prev_count++;
    }

    // make labels and construct / position scene
    std::string label;
    int total_len = 0;

    if(g_config.medals_show_policy == Config_t::MEDALS_SHOW_COUNTS)
        label = fmt::format_ne("{0}/{1}", got_count, max);
    else
        label = fmt::format_ne("{0}", got_count);

    total_len += coin_width + 8 + GFX.Interface[1].w + 4;
    total_len += SuperTextPixLen(label, 3);

    if(warp)
        X -= (total_len / 2) & ~1;
    else
        X -= total_len;

    // draw scene
    s_DrawMedal(0, X, Y, coin_width, coin_height, show_shiny ? MedalDrawLevel::Shiny : MedalDrawLevel::Got);
    X += coin_width + 8;
    XRender::renderTexture(X, Y, GFX.Interface[1]);
    X += GFX.Interface[1].w + 4;
    SuperPrint(label, 3, X, Y);
}

void DrawDeviceBattery()
{
#ifdef RENDER_FULLSCREEN_ALWAYS
    constexpr bool isFullScreen = true;
#else
    const bool isFullScreen = resChanged;
#endif

    if(g_videoSettings.batteryStatus == BATTERY_STATUS_OFF)
        return;

    if(!isFullScreen)
    {
        if(g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_WHEN_LOW || g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_ON)
            return;
    }

    XPower::StatusInfo status_info = XPower::devicePowerStatus();

    if(status_info.power_status == XPower::StatusInfo::POWER_DISABLED || status_info.power_status == XPower::StatusInfo::POWER_UNKNOWN || status_info.power_status == XPower::StatusInfo::POWER_WIRED)
        return;

    bool isLow = (status_info.power_status <= 0.35f);

    bool showBattery = false;

    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_ALWAYS_ON);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_ANY_WHEN_LOW && isLow);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_WHEN_LOW && isLow && isFullScreen);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_ON && isFullScreen);

    if(showBattery)
    {
        int bw = 40;
        int bh = 22;
        int bx = XRender::TargetW - XRender::TargetOverscanX - (bw + 8);
        int by = 24;

        RenderPowerInfo(0, bx, by, bw, bh, 255, &status_info);
    }
}
