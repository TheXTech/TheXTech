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

#include <Logger/logger.h>
#include <fmt_format_ne.h>
#include <Integrator/integrator.h>

#include "Archives/archives.h"
#include "sdl_proxy/sdl_stdinc.h"

#include "main/screen_asset_pack.h"
#include "main/asset_pack.h"

#include "core/render.h"

#include "sound.h"
#include "controls.h"
#include "draw_planes.h"
#include "graphics.h"
#include "gfx.h"
#include "game_main.h"

void drawGameVersion(bool disable_git, bool git_only);

namespace ScreenAssetPack
{

bool g_LoopActive = false;
bool s_AnimatingBack = false;

// the player's target asset pack index
static int s_target_idx = -1;

// the current asset pack index
static int s_cur_idx = -1;

// how far to scroll the logos. ranges from -1 (halfway through a right rotation) to +1 (halfway through a left rotation)
static num_t s_switch_coord = 0.0_n;

static bool s_ensure_idx_valid()
{
    const auto& asset_packs = GetAssetPacks();

    if(s_cur_idx >= 0 && s_cur_idx < (int)asset_packs.size())
        return true;

    s_cur_idx = 0;

    std::string real_app_path;

    // restore real path from loaded assets
    if(AppPath[0] == ':' && AppPath[1] == 'a')
    {
        real_app_path.push_back('@');
        real_app_path.append(Archives::assets_archive_path());
        real_app_path.push_back(':');
        real_app_path.append(AppPath.c_str() + 2, AppPath.size() - 2);
    }
    else
        real_app_path = AppPath;

    for(const auto& pack : asset_packs)
    {
        if(pack.path == real_app_path && pack.full_id() == g_AssetPackID)
        {
            if(s_target_idx == -1)
                s_target_idx = s_cur_idx;

            return true;
        }

        s_cur_idx++;
    }

    return false;
}

static void s_renderBackground(AssetPack_t::Gfx& gfx, XTColor bg_color)
{
    if(gfx.background.inited)
    {
        int background_height = gfx.background.h / gfx.bg_frames;
        int background_frame = (CommonFrame / gfx.bg_frame_ticks) % gfx.bg_frames;

        int show_width = SDL_min(background_height * XRender::TargetW / XRender::TargetH, gfx.background.w);
        int show_height = SDL_min(gfx.background.w * XRender::TargetH / XRender::TargetW, background_height);

        XRender::renderTextureScaleEx(0, 0, XRender::TargetW, XRender::TargetH,
                                      gfx.background,
                                      (gfx.background.w - show_width) / 2, background_height * background_frame + (background_height - show_height) / 2,
                                      show_width, show_height,
                                      0, nullptr, X_FLIP_NONE,
                                      bg_color);
    }
    else
    {
        XRender::renderRect(0, 0, XRender::TargetW, XRender::TargetH, XTColor(0, 0, 0) * bg_color);
    }
}

// this method is public so that it is possible to fade in the asset pack screen from the main menu
void DrawBackground(int fade)
{
    if(!s_ensure_idx_valid())
        return;

    // used for things that are scaled in special ways during switch
    XTColor color_no_switch = XTAlpha(fade * 4 - 1);

    // used for things linked to the central pack
    XTColor color = XTAlphaF(fade * (1.0_n - num_t::abs(s_switch_coord)) / 64);

    const AssetPack_t& pack = GetAssetPacks()[s_cur_idx];

    if(!pack.gfx)
        return;

    int menu_logo_y = XRender::TargetH / 2 - 230;

    // place manually on small screens
    if(XRender::TargetH < 400)
        menu_logo_y = 16 + (XRender::TargetH - 320) / 2;
    else if(XRender::TargetH < 500)
        menu_logo_y = 16;
    else if(XRender::TargetH <= 600)
        menu_logo_y = 40;

    // logic for the menu curtain draw
    int curtain_draw_w = GFX.MenuGFX[1].w;
    if(curtain_draw_w == 800)
        curtain_draw_w = 768;
    int curtain_horiz_reps = XRender::TargetW / curtain_draw_w + 2;

    // draw background curtain fade
    if(s_AnimatingBack)
    {
        for(int i = 0; i < curtain_horiz_reps; i++)
            XRender::renderTextureBasic(curtain_draw_w * i, -(GFX.MenuGFX[1].h * fade / 64), curtain_draw_w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);
    }

    // draw background logo
    if(s_AnimatingBack || pack.logo_override)
        XRender::renderTextureBasic(XRender::TargetW / 2 - GFX.MenuGFX[2].w / 2, menu_logo_y, GFX.MenuGFX[2]);

    AssetPack_t::Gfx& gfx = *pack.gfx;

#ifdef __3DS__
    XRender::setTargetLayer(2);
#endif

    // draw background
    s_renderBackground(gfx, color_no_switch);

    // draw curtain during transition
    if(!g_LoopActive && !s_AnimatingBack)
    {
        for(int i = 0; i < curtain_horiz_reps; i++)
            XRender::renderTextureBasic(curtain_draw_w * i, -(GFX.MenuGFX[1].h * fade / 64), curtain_draw_w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);
    }

    // show previous / next packs
    int prev_index = s_cur_idx - 1;
    int next_index = s_cur_idx + 1;

    if(prev_index < 0)
        prev_index = GetAssetPacks().size() - 1;
    if(next_index >= (int)GetAssetPacks().size())
        next_index = 0;

    const AssetPack_t& prev_pack = GetAssetPacks()[prev_index];
    const AssetPack_t& next_pack = GetAssetPacks()[next_index];

    // cross-fade background
    if(s_switch_coord > 0 && prev_pack.gfx)
    {
        XTColor bg_color = color_no_switch * XTAlphaF(s_switch_coord / 2);
        s_renderBackground(*prev_pack.gfx, bg_color);
    }
    else if(s_switch_coord < 0 && next_pack.gfx)
    {
        XTColor bg_color = color_no_switch * XTAlphaF(-s_switch_coord / 2);
        s_renderBackground(*next_pack.gfx, bg_color);
    }

#ifdef __3DS__
    XRender::setTargetLayer(3);

    // fade the existing display
    if(!g_LoopActive && XRender::TargetH <= 480)
        XRender::targetFade(256 - fade * 4);
#endif

    // calculate how much the pack logos should be shifted
    int logo_shift = (XRender::TargetW / 2) - 100;

    if(prev_pack.gfx && prev_pack.gfx->logo.inited)
    {
        num_t cX = 100 + logo_shift * s_switch_coord / 2;
        XTColor pack_color = (s_switch_coord == 0) ? color * XTAlpha(127) : color_no_switch * XTAlphaF(0.25_n + (s_switch_coord + 1.0_n) / 4);
        XRender::renderTextureBasic((int)(cX + prev_pack.gfx->logo.w * (s_switch_coord / 4 - 1)),
            XRender::TargetH / 2 - prev_pack.gfx->logo.h / 2,
            prev_pack.gfx->logo,
            pack_color);
    }

    if(next_pack.gfx && next_pack.gfx->logo.inited)
    {
        num_t cX = (XRender::TargetW - 100) + logo_shift * s_switch_coord / 2;
        XTColor pack_color = (s_switch_coord == 0) ? color * XTAlpha(127) : color_no_switch * XTAlphaF(0.25_n + (1.0_n - s_switch_coord) / 4);
        XRender::renderTextureBasic((int)(cX + prev_pack.gfx->logo.w * (s_switch_coord / 4)),
            XRender::TargetH / 2 - next_pack.gfx->logo.h / 2,
            next_pack.gfx->logo,
            pack_color);
    }

    // draw current logo in all cases (needed for cases where title card is not in GFX.MenuGFX[2])
    if(!gfx.logo.inited)
    {
        SuperPrintScreenCenter(pack.full_id(), 3, XRender::TargetH / 2 - 20, color);
        SuperPrintScreenCenter(pack.path, 3, XRender::TargetH / 2 + 2, color);
    }
    else if(g_LoopActive || s_AnimatingBack || pack.logo_override)
    {
        num_t cX = (XRender::TargetW / 2) + logo_shift * s_switch_coord / 2;
        XTColor main_color = color_no_switch * XTAlphaF(0.75_n + (1.0_n - num_t::abs(s_switch_coord)) / 4);
        XRender::renderTextureBasic((int)(cX + gfx.logo.w * (s_switch_coord / 4 - 0.5_n)), XRender::TargetH / 2 - gfx.logo.h / 2, gfx.logo, main_color);
    }
    else
    {
        num_t cX = (XRender::TargetW / 2) + logo_shift * s_switch_coord * fade / (64 * 2);

        int center_Y = XRender::TargetH / 2 - gfx.logo.h / 2;
        int place_Y = (center_Y * fade / 64) + (menu_logo_y * (64 - fade) / 64);

        XRender::renderTextureBasic((int)(cX + gfx.logo.w * (s_switch_coord * fade / (64 * 4) - 0.5_n)),
            place_Y,
            gfx.logo);
    }

    // show version if appropriate
    bool show_version = pack.show_version && !pack.version.empty();
    if(pack.show_id || show_version || pack.id.empty())
    {
        num_t cX = (XRender::TargetW / 2) + logo_shift * s_switch_coord / 2;
        int text_Y = XRender::TargetH / 2 + gfx.logo.h / 2 + 8;

        const std::string& display = (pack.show_id) ? pack.full_id() : ((show_version) ? pack.version : "<legacy>");
        SuperPrintCenter(display, 3, (int)cX, text_Y, color);
    }

    // show scroll indicators
    if(g_LoopActive && CommonFrame % 90 >= 45 && s_cur_idx == s_target_idx && s_switch_coord == 0)
    {
        int offset = SDL_min(XRender::TargetW / 2 - 8, 250);
        if(GFX.CharSelIcons.inited)
        {
            XRender::renderTextureFL(XRender::TargetW / 2 - offset, XRender::TargetH / 2 - 24 / 2, 24, 24, GFX.CharSelIcons, 72, 0, 0, nullptr, X_FLIP_HORIZONTAL);
            XRender::renderTextureBasic(XRender::TargetW / 2 + offset - 24, XRender::TargetH / 2, 24, 24, GFX.CharSelIcons, 72, 0);
        }
        else
        {
            XRender::renderTextureFL(XRender::TargetW / 2 - offset, XRender::TargetH / 2 - GFX.MCursor[1].w / 2, GFX.MCursor[1].w, GFX.MCursor[1].h, GFX.MCursor[1], 0, 0, -90);
            XRender::renderTextureFL(XRender::TargetW / 2 + offset - GFX.MCursor[1].h, XRender::TargetH / 2 - GFX.MCursor[2].w / 2, GFX.MCursor[2].w, GFX.MCursor[2].h, GFX.MCursor[2], 0, 0, -90);
        }
    }

    drawGameVersion(true, false);

    // reset all variables when alpha is low enough during exit
    if(fade < 3)
    {
        s_AnimatingBack = false;
        s_cur_idx = -1;
        s_target_idx = -1;
        s_switch_coord = 0;
    }

    if(g_LoopActive)
        s_AnimatingBack = false;
}

void Render(bool now_loading = false)
{
    XRender::setTargetTexture();
    XRender::resetViewport();
    XRender::clearBuffer();

    XRender::setDrawPlane(PLANE_GAME_MENUS);

    CommonFrame++;

    DrawBackground(64);

    // Mouse cursor
    XRender::renderTextureBasic(int(SharedCursor.X), int(SharedCursor.Y), GFX.ECursor[2]);

    DrawDeviceBattery();

    if(now_loading)
    {
        int R = XRender::TargetW / 2 + 400;
        int B = XRender::TargetH / 2 + 300;

        if(R > XRender::TargetW)
            R = XRender::TargetW;

        if(B > XRender::TargetH)
            B = XRender::TargetH;

        R -= (GFX.Loader.w + 50);
        B -= (GFX.Loader.h + 8);

        XRender::renderTextureBasic(R, B, GFX.Loader);
    }

    XRender::repaint();

    if(TakeScreen)
        ScreenShot();
}

bool Logic()
{
    // escape if the current asset pack no longer exists
    if(!s_ensure_idx_valid())
    {
        g_LoopActive = false;
        GameMenu = true;
        return true;
    }

    if(!SharedCursor.Primary && !SharedCursor.Secondary)
        MenuMouseRelease = true;
    // replicates legacy behavior allowing clicks to be detected
    if(SharedCursor.Primary || SharedCursor.Secondary || SharedCursor.Tertiary)
        SharedCursor.Move = true;

    bool leftPressed = l_SharedControls.MenuLeft || SharedCursor.ScrollUp;
    bool rightPressed = l_SharedControls.MenuRight || SharedCursor.ScrollDown;

    bool menuDoPress = l_SharedControls.MenuDo || l_SharedControls.Pause;
    bool menuBackPress = l_SharedControls.MenuBack;

    for(int i = 0; i < l_screen->player_count; i++)
    {
        Controls_t &c = Controls::g_RawControls[i];

        menuDoPress |= c.Start || c.Jump;
        menuBackPress |= c.Run;

        leftPressed |= c.Left;
        rightPressed |= c.Right;
    }

    menuBackPress |= SharedCursor.Secondary && MenuMouseRelease;
    menuDoPress |= SharedCursor.Primary && MenuMouseRelease;

    if(menuBackPress && menuDoPress)
        menuDoPress = false;

    if(!MenuCursorCanMove)
    {
        bool k = false;
        k |= menuBackPress;
        k |= menuDoPress;
        k |= leftPressed;
        k |= rightPressed;

        if(!k)
            MenuCursorCanMove = true;
    }
    else if(menuDoPress)
    {
        // check if we are still on the same asset pack
        int cur_idx = s_cur_idx;
        s_cur_idx = -1;
        s_ensure_idx_valid();

        // if not still on the same asset pack, reload assets!!
        if(cur_idx != s_cur_idx || !g_AssetsLoaded)
        {
            s_cur_idx = cur_idx;

            if(!g_AssetsLoaded)
            {
                XRender::setTargetTexture();
                XRender::clearBuffer();
                XRender::repaint();
            }
            else
                Render(true);

            CommonFrame = 0; // don't show indicators
            if(!ReloadAssetsFrom(GetAssetPacks()[cur_idx]))
            {
                s_cur_idx = -1;

                // stay in the asset pack screen if we haven't loaded any assets yet
                if(!g_AssetsLoaded)
                    return false;

                s_AnimatingBack = true;
            }
            else
            {
                // update recent asset pack
                SaveConfig();
            }
        }
        else
            PlaySoundMenu(SFX_Do);

        // proceed to game menu
        g_LoopActive = false;
        GameMenu = true;
        MenuCursor = 0;
        MenuCursorCanMove = false;
        MenuMouseRelease = false;
        return true;
    }
    else if(menuBackPress && g_AssetsLoaded)
    {
        PlaySoundMenu(SFX_Slide);

        // check if we are still on the same asset pack
        int cur_idx = s_cur_idx;
        s_cur_idx = -1;
        s_ensure_idx_valid();

        // if not still on the same asset pack, animate the back
        if(cur_idx != s_cur_idx)
            s_AnimatingBack = true;

        s_cur_idx = cur_idx;

        // return to game menu
        g_LoopActive = false;
        GameMenu = true;
        MenuCursorCanMove = false;
        MenuMouseRelease = false;
        return true;
    }
    else if(leftPressed)
    {
        MenuCursorCanMove = false;

        PlaySoundMenu(SFX_Climbing);
        s_target_idx--;
    }
    else if(rightPressed)
    {
        MenuCursorCanMove = false;

        PlaySoundMenu(SFX_Climbing);
        s_target_idx++;
    }

    if(SharedCursor.Primary || SharedCursor.Secondary)
        MenuMouseRelease = false;

    MenuMouseClick = false;

    // 16 frames to switch
    constexpr num_t move_rate = 0.125_n;

    // make s_switch_coord approach the target
    if(s_target_idx != s_cur_idx)
        s_switch_coord -= move_rate * (s_target_idx - s_cur_idx);
    else if(s_switch_coord < -move_rate)
        s_switch_coord += move_rate;
    else if(s_switch_coord > move_rate)
        s_switch_coord -= move_rate;
    else
        s_switch_coord = 0.0_n;

    // actually switch the index when switch coord grows enough
    if(s_switch_coord >= 1.0_n)
    {
        s_cur_idx--;
        if(s_cur_idx < 0)
        {
            if(s_target_idx < 0)
                s_target_idx += (int)GetAssetPacks().size();

            s_cur_idx = (int)GetAssetPacks().size() - 1;
        }

        s_switch_coord -= 2.0_n;
    }
    else if(s_switch_coord <= -1.0_n)
    {
        s_cur_idx++;
        if(s_cur_idx >= (int)GetAssetPacks().size())
        {
            if(s_target_idx >= (int)GetAssetPacks().size())
                s_target_idx -= (int)GetAssetPacks().size();

            s_cur_idx = 0;
        }

        s_switch_coord += 2.0_n;
    }

    return false;
}

void Loop()
{
    Controls::PollInputMethod();
    Controls::Update(false);

    if(Logic())
        return;

    UpdateSound();

    Integrator::sync();

    Render();
}

} // namespace ScreenAssetPack
