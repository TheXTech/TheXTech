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

#include "core/render.h"

#include "globals.h"
#include "player.h"
#include "gfx.h"
#include "graphics.h"

bool worldHasFrameAssets()
{
    return GFX.WorldMapFrame_Tile.inited && (!GFX.Interface[4].inited || !GFX.isCustom(37) || GFX.isCustom(73));
}

static bool s_border_valid()
{
    return GFX.WorldMapFrame_Border.tex.inited && (!GFX.isCustom(73) || GFX.isCustom(74));
}

static void s_getMargins(const Screen_t& screen, double& margin, double& marginTop, double& marginBottom)
{
    margin = 66;
    marginTop = 130;
    marginBottom = 66;

    if(screen.H < 400)
    {
        marginBottom = 24;
        marginTop = 72;
    }
    else if(screen.H < 500)
    {
        marginBottom = 32;
        marginTop = 96;
    }

    if(screen.W < 400)
        margin = 24;
    else if(screen.W < 600)
        margin = 32;
    else if(screen.W < 800)
        margin = 48;

    // make sure world map frame stays visible
    if(s_border_valid())
    {
        if(margin < GFX.WorldMapFrame_Border.le)
            margin = GFX.WorldMapFrame_Border.le;

        if(margin < GFX.WorldMapFrame_Border.re)
            margin = GFX.WorldMapFrame_Border.re;

        if(marginTop < GFX.WorldMapFrame_Border.te)
            marginTop = GFX.WorldMapFrame_Border.te;

        if(marginBottom < GFX.WorldMapFrame_Border.be)
            marginBottom = GFX.WorldMapFrame_Border.be;
    }
}

void GetvScreenWorld(vScreen_t& vscreen)
{
    const Screen_t& screen = Screens[vscreen.screen_ref];
    const WorldPlayer_t& wp = WorldPlayer[vscreen.player];
    const Location_t& wpLoc = wp.Location;

    double margin, marginTop, marginBottom;
    s_getMargins(screen, margin, marginTop, marginBottom);

    vscreen.Top = marginTop;
    vscreen.Height = screen.H - marginBottom - marginTop;
    vscreen.Left = margin;
    vscreen.Width = screen.W - (margin * 2);

    // limit the vScreen to its 800x600 size (668x404) if the new world map frame assets are missing
    const bool haveFrameAssets = worldHasFrameAssets();
    if(!haveFrameAssets)
    {
        if(vscreen.Width > 668)
        {
            vscreen.Left += (vscreen.Width - 668) / 2;
            vscreen.Width = 668;
        }

        if(vscreen.Height > 404)
        {
            vscreen.Top += (vscreen.Height - 404) / 2;
            vscreen.Height = 404;
        }
    }

    double fX = wpLoc.X + wpLoc.Width / 2;
    double fY = wpLoc.Y + wpLoc.Height / 2;

    // apply a temporary vScreen focus
    if(vscreen.TempDelay)
    {
        fX = vscreen.tempX;
        fY = vscreen.TempY;
    }

    vscreen.X = -fX + vscreen.Width / 2;
    vscreen.Y = -fY + vscreen.Height / 2;

    // begin bounds logic

    // default bounds are the canonical vScreen
    const Screen_t& c_screen = screen.canonical_screen();
    double c_margin, c_marginTop, c_marginBottom;
    s_getMargins(c_screen, c_margin, c_marginTop, c_marginBottom);

    double c_width = c_screen.W - c_margin * 2;
    double c_height = c_screen.H - c_marginTop - c_marginBottom;

    const Location_t defaultBounds = newLoc(fX - c_width / 2, fY - c_height / 2, c_width, c_height);

    // get the bounds from the player's section if possible, otherwise the defaults
    const Location_t& bounds = (wp.Section != 0)
        ? static_cast<Location_t>(WorldArea[wp.Section].Location)
        : defaultBounds;

    if(bounds.Width < vscreen.Width)
    {
        vscreen.Left += (vscreen.Width - bounds.Width) / 2;
        vscreen.Width = bounds.Width;
        vscreen.X = -bounds.X;
    }
    else if(-vscreen.X < bounds.X)
        vscreen.X = -bounds.X;
    else if(-vscreen.X > bounds.X + bounds.Width - vscreen.Width)
        vscreen.X = -(bounds.X + bounds.Width - vscreen.Width);

    if(bounds.Height < vscreen.Height)
    {
        vscreen.Top += (vscreen.Height - bounds.Height) / 2;
        vscreen.Height = bounds.Height;
        vscreen.Y = -bounds.Y;
    }
    else if(-vscreen.Y < bounds.Y)
        vscreen.Y = -bounds.Y;
    else if(-vscreen.Y > bounds.Y + bounds.Height - vscreen.Height)
        vscreen.Y = -(bounds.Y + bounds.Height - vscreen.Height);

    vscreen.ScreenTop = vscreen.Top;
    vscreen.ScreenLeft = vscreen.Left;
}

void DrawPlayerWorld(Player_t& p, int X, int Y)
{
    p.Direction = -1;
    p.Location.SpeedY = 0;
    p.Location.SpeedX = -1;
    p.Controls.Left = false;
    p.Controls.Right = false;
    p.SpinJump = false;
    p.Dead = false;
    p.Immune2 = false;
    p.Fairy = false;
    p.TimeToLive = 0;
    p.Effect = 0;
    p.MountSpecial = 0;
    p.HoldingNPC = 0;
    if(p.Duck)
        UnDuck(p);
    PlayerFrame(p);

    p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
    p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
    SizeCheck(p);
    p.Location.X = X;
    p.Location.Y = Y - p.Location.Height;

    if(p.MountType == 3)
    {
        p.YoshiWingsFrameCount += 1;
        p.YoshiWingsFrame = 0;
        if(p.YoshiWingsFrameCount <= 12)
            p.YoshiWingsFrame = 1;
        else if(p.YoshiWingsFrameCount >= 24)
            p.YoshiWingsFrameCount = 0;
        if(p.Direction == 1)
            p.YoshiWingsFrame += 2;
    }

    DrawPlayer(p, 0);
}

void DrawWorldMapFrame(const vScreen_t& vscreen)
{
    const Screen_t& screen = Screens[vscreen.screen_ref];

    if(worldHasFrameAssets())
    {
        RenderFrameBorder(newLoc(0, 0, screen.W, screen.H), newLoc(vscreen.ScreenLeft, vscreen.ScreenTop, vscreen.Width, vscreen.Height),
            GFX.WorldMapFrame_Tile, s_border_valid() ? &GFX.WorldMapFrame_Border : nullptr);
    }
    else
    {
        // render a legacy background, in MANY careful segments...
        constexpr bool do_stretch = true;

        // top-left
        XRender::renderTexture(vscreen.ScreenLeft - 66, vscreen.ScreenTop - 130, 66, 130, GFX.Interface[4], 0, 0);

        // top
        int orig_width = GFX.Interface[4].w - 66 - 66;
        if(do_stretch)
        {
            XRender::renderTextureScaleEx(vscreen.ScreenLeft, vscreen.ScreenTop - 130,
                vscreen.Width, 130,
                GFX.Interface[4], 66, 0, orig_width, 130);
        }
        else
            for(int offset = 0; offset < vscreen.Width; offset += orig_width)
        {
            XRender::renderTexture(vscreen.ScreenLeft + offset, vscreen.ScreenTop - 130,
                SDL_min((int)vscreen.Width - offset, orig_width), 130,
                GFX.Interface[4], 66, 0);
        }

        // top-right
        XRender::renderTexture(vscreen.ScreenLeft + vscreen.Width, vscreen.ScreenTop - 130, 66, 130 + 20, GFX.Interface[4], GFX.Interface[4].w - 66, 0);

        // left
        int orig_height = GFX.Interface[4].h - 130 - 66;
        if(do_stretch)
        {
            XRender::renderTextureScaleEx(vscreen.ScreenLeft - 66, vscreen.ScreenTop,
                66, vscreen.Height,
                GFX.Interface[4], 0, 130, 66, orig_height);
        }
        else
            for(int offset = 0; offset < vscreen.Height; offset += orig_height)
        {
            XRender::renderTexture(vscreen.ScreenLeft - 66, vscreen.ScreenTop + offset,
                66, SDL_min((int)vscreen.Height - offset, orig_height),
                GFX.Interface[4], 0, 130);
        }

        // right
        orig_height = GFX.Interface[4].h - (130 + 20) - 66;
        if(do_stretch)
        {
            XRender::renderTextureScaleEx(vscreen.ScreenLeft + vscreen.Width, vscreen.ScreenTop + 20,
                66, vscreen.Height - 20,
                GFX.Interface[4], GFX.Interface[4].w - 66, 130 + 20, 66, orig_height);
        }
        else
            for(int offset = 20; offset < vscreen.Height; offset += orig_height)
        {
            XRender::renderTexture(vscreen.ScreenLeft + vscreen.Width, vscreen.ScreenTop + offset,
                66, SDL_min((int)vscreen.Height - offset, orig_height),
                GFX.Interface[4], GFX.Interface[4].w - 66, 130 + 20);
        }

        // bottom-left
        XRender::renderTexture(vscreen.ScreenLeft - 66, vscreen.ScreenTop + vscreen.Height, 66 + 34, 66, GFX.Interface[4], 0, GFX.Interface[4].h - 66);

        // bottom
        orig_width = GFX.Interface[4].w - (66 + 34) - 66;
        if(do_stretch)
        {
            XRender::renderTextureScaleEx(vscreen.ScreenLeft + 34, vscreen.ScreenTop + vscreen.Height,
                vscreen.Width - 34, 66,
                GFX.Interface[4], 100, GFX.Interface[4].h - 66, orig_width, 66);
        }
        else
            for(int offset = 34; offset < vscreen.Width; offset += orig_width)
        {
            XRender::renderTexture(vscreen.ScreenLeft + offset, vscreen.ScreenTop + vscreen.Height,
                SDL_min((int)vscreen.Width - offset, orig_width), 66,
                GFX.Interface[4], 100, GFX.Interface[4].h - 66);
        }

        // bottom-right
        XRender::renderTexture(vscreen.ScreenLeft + vscreen.Width, vscreen.ScreenTop + vscreen.Height, 66, 66, GFX.Interface[4], GFX.Interface[4].w - 66, GFX.Interface[4].h - 66);
    }
}
