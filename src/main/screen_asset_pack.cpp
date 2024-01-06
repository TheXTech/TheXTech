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

#include <Logger/logger.h>
#include <fmt_format_ne.h>
#include <Integrator/integrator.h>

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

void drawGameVersion(bool disable_git);

namespace ScreenAssetPack
{

bool g_LoopActive = false;
bool s_AnimatingBack = false;

static int s_cur_idx = -1;

static bool s_ensure_idx_valid()
{
    const auto& asset_packs = GetAssetPacks();

    if(s_cur_idx >= 0 && s_cur_idx < (int)asset_packs.size())
        return true;

    s_cur_idx = 0;

    for(const auto& pack : asset_packs)
    {
        if(pack.path == AppPath && pack.id == g_AssetPackID)
            return true;

        s_cur_idx++;
    }

    return false;
}

// this method is public so that it is possible to fade in the asset pack screen from the main menu
void DrawBackground(double fade)
{
    if(!s_ensure_idx_valid())
        return;

    XTColor color = XTAlphaF(static_cast<float>(fade));

    const AssetPack_t& pack = GetAssetPacks()[s_cur_idx];

    if(!pack.gfx)
        return;

    int menu_logo_y = XRender::TargetH / 2 - 230;

    // place manually on small screens
    if(XRender::TargetH < 500)
        menu_logo_y = 16;
    else if(XRender::TargetH <= 600)
        menu_logo_y = 40;

    // logic for the menu curtain draw
    int curtain_draw_w = GFX.MenuGFX[1].w;
    if(curtain_draw_w == 800)
        curtain_draw_w = 768;
    int curtain_horiz_reps = XRender::TargetW / curtain_draw_w + 2;

    if(s_AnimatingBack)
    {
        for(int i = 0; i < curtain_horiz_reps; i++)
            XRender::renderTexture(curtain_draw_w * i, -GFX.MenuGFX[1].h * fade, curtain_draw_w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);

        XRender::renderTexture(XRender::TargetW / 2 - GFX.MenuGFX[2].w / 2, menu_logo_y, GFX.MenuGFX[2]);
    }

    AssetPack_t::Gfx& gfx = *pack.gfx.get();

    // zoom background to screen size
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
                                      0.0, nullptr, X_FLIP_NONE,
                                      color);
    }
    else
    {
        XRender::renderRect(0, 0, XRender::TargetW, XRender::TargetH, XTColor(0, 0, 0) * color);
    }

    // draw curtain during transition
    if(!g_LoopActive && !s_AnimatingBack)
    {
        for(int i = 0; i < curtain_horiz_reps; i++)
            XRender::renderTexture(curtain_draw_w * i, -GFX.MenuGFX[1].h * fade, curtain_draw_w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);
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

    if(prev_pack.gfx && prev_pack.gfx->logo.inited)
        XRender::renderTexture(100 - prev_pack.gfx->logo.w, XRender::TargetH / 2 - prev_pack.gfx->logo.h / 2, prev_pack.gfx->logo, color * XTAlpha(127));

    if(next_pack.gfx && next_pack.gfx->logo.inited)
        XRender::renderTexture(XRender::TargetW - 100, XRender::TargetH / 2 - next_pack.gfx->logo.h / 2, next_pack.gfx->logo, color * XTAlpha(127));

    // draw current logo in all cases (needed for cases where title card is not in GFX.MenuGFX[2])
    if(!gfx.logo.inited)
    {
        SuperPrintScreenCenter(pack.id, 3, XRender::TargetH / 2 - 20, color);
        SuperPrintScreenCenter(pack.path, 3, XRender::TargetH / 2 + 2, color);
    }
    else if(g_LoopActive || s_AnimatingBack)
    {
        XRender::renderTexture(XRender::TargetW / 2 - gfx.logo.w / 2, XRender::TargetH / 2 - gfx.logo.h / 2, gfx.logo, color);
    }
    else
    {
        int center_Y = XRender::TargetH / 2 - gfx.logo.h / 2;
        int place_Y = (center_Y * fade) + (menu_logo_y * (1.0 - fade));

        XRender::renderTexture(XRender::TargetW / 2 - gfx.logo.w / 2, place_Y, gfx.logo);
    }

    // show scroll indicators
    if(g_LoopActive && CommonFrame % 90 >= 45)
    {
        int offset = SDL_min(XRender::TargetW / 2 - 8, 250);
        if(GFX.CharSelIcons.inited)
        {
            XRender::renderTextureFL(XRender::TargetW / 2 - offset, XRender::TargetH / 2 - 24 / 2, 24, 24, GFX.CharSelIcons, 72, 0, 0.0, nullptr, X_FLIP_HORIZONTAL);
            XRender::renderTexture(XRender::TargetW / 2 + offset - 24, XRender::TargetH / 2, 24, 24, GFX.CharSelIcons, 72, 0);
        }
        else
        {
            XRender::renderTextureFL(XRender::TargetW / 2 - offset, XRender::TargetH / 2 - GFX.MCursor[1].w / 2, GFX.MCursor[1].w, GFX.MCursor[1].h, GFX.MCursor[1], 0, 0, -90.0);
            XRender::renderTextureFL(XRender::TargetW / 2 + offset - GFX.MCursor[1].h, XRender::TargetH / 2 - GFX.MCursor[2].w / 2, GFX.MCursor[2].w, GFX.MCursor[2].h, GFX.MCursor[2], 0, 0, -90.0);
        }
    }

    drawGameVersion(true);

    // cancel back animation when alpha is low enough
    if(s_AnimatingBack && fade < 2.0 / 60.0)
    {
        s_AnimatingBack = false;
        s_cur_idx = -1;
    }
}

void Render()
{
    XRender::setTargetTexture();
    XRender::resetViewport();

    XRender::setDrawPlane(PLANE_GAME_MENUS);

    CommonFrame++;

    DrawBackground(1.0);

    // Mouse cursor
    XRender::renderTexture(int(SharedCursor.X), int(SharedCursor.Y), GFX.ECursor[2]);

    DrawDeviceBattery();

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

    bool leftPressed = SharedControls.MenuLeft || SharedCursor.ScrollUp;
    bool rightPressed = SharedControls.MenuRight || SharedCursor.ScrollDown;

    bool menuDoPress = SharedControls.MenuDo || SharedControls.Pause;
    bool menuBackPress = SharedControls.MenuBack;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        Controls_t &c = Player[i + 1].Controls;

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
        if(cur_idx != s_cur_idx)
        {
            s_cur_idx = cur_idx;
            CommonFrame = 0; // don't show indicators
            if(!ReloadAssetsFrom(GetAssetPacks()[cur_idx].path))
            {
                s_cur_idx = -1;
                s_AnimatingBack = true;
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
    else if(menuBackPress)
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
        s_cur_idx--;
        if(s_cur_idx < 0)
            s_cur_idx = GetAssetPacks().size() - 1;
    }
    else if(rightPressed)
    {
        MenuCursorCanMove = false;

        PlaySoundMenu(SFX_Climbing);
        s_cur_idx++;
        if(s_cur_idx >= (int)GetAssetPacks().size())
            s_cur_idx = 0;
    }

    if(SharedCursor.Primary || SharedCursor.Secondary)
        MenuMouseRelease = false;

    MenuMouseClick = false;

    return false;
}

void Loop()
{
    Controls::PollInputMethod();
    Controls::Update();

    if(Logic())
        return;

    UpdateSound();

    Integrator::sync();

    Render();
}

} // namespace ScreenAssetPack
