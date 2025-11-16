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

#include "globals.h"
#include "core/render.h"

#include "graphics/gfx_keyhole.h"

static inline int s_round2int(double d)
{
    return std::floor(d + 0.5);
}

void RenderKeyhole(int Z)
{
    const int keyholeMax = 192; // Was 300
    int keyholeDone = keyholeMax - 65;
    int ratio = 256 * LevelMacroCounter / keyholeDone;
    if(ratio > 255)
        ratio = 255;

    Background_t& keyhole = Background[LevelMacroWhich];

    double realKeyholeBottom = keyhole.Location.Y + 24;
    double idealKeyholeBottom = 32 * std::ceil(realKeyholeBottom / 32);

    double keyholeGrowthCoord = (double)ratio / 100;

    if(keyholeGrowthCoord > 1.0)
        keyholeGrowthCoord = 1.0;

    double keyholeScale = keyholeGrowthCoord * 12;

    if(ratio < 128)
        keyholeScale += (1 - keyholeGrowthCoord);

    double keyholeBottom = realKeyholeBottom * (1 - keyholeGrowthCoord) + idealKeyholeBottom * keyholeGrowthCoord;

    RenderTexturePlayerScale(Z, Player[0],
        vScreen[Z].X + keyhole.Location.X + keyhole.Location.Width / 2 - keyhole.Location.Width * keyholeScale / 2,
        vScreen[Z].Y + keyholeBottom - 24 * keyholeScale,
        keyhole.Location.Width * keyholeScale,
        keyhole.Location.Height * keyholeScale,
        GFXBackgroundBMP[keyhole.Type],
        0,
        0,
        keyhole.Location.Width,
        keyhole.Location.Height);

    if(ratio >= 128)
        keyhole.Hidden = true;
}

// WARP LOGIC: moved from PlayerWarpGFX (graphics.cpp)
static inline bool s_warp_gfx(int Z, const Player_t& p,
                         int& dst_x, int& dst_y, int& dst_w, int& dst_h,
                         int& src_x, int& src_y)
{
    // .Effect = 3      -- Warp Pipe
    // .Effect2 = 0     -- Entering
    // .Effect2 = 1     -- Move to next spot
    // .Effect2 => 100  -- Delay at next spot
    // .Effect2 = 2     -- Exiting
    // .Effect2 = 3     -- Done

    if(p.Effect2 == 1 || p.Effect2 >= 100)
        return true;

    if(p.Effect2 == 0 || p.Effect2 == 2)
    {
        int camX = vScreen[Z].CameraAddX();
        int camY = vScreen[Z].CameraAddY();

        bool use_exit = (p.WarpBackward) == (p.Effect2 == 0);
        const Warp_t& warp = Warp[p.Warp];
        const SpeedlessLocation_t& warp_loc = (use_exit) ? warp.Exit : warp.Entrance;
        auto warp_dir = (use_exit) ? warp.Direction2 : warp.Direction;

        int warp_l = camX + s_round2int(warp_loc.X);
        int warp_r = warp_l + s_round2int(warp_loc.Width);
        int warp_t = camY + s_round2int(warp_loc.Y);
        int warp_b = warp_t + s_round2int(warp_loc.Height);

        if(warp_dir == 3) // warp below player
        {
            if(dst_h > warp_b - dst_y)
                dst_h = warp_b - dst_y;
        }
        else if(warp_dir == 1) // warp above player
        {
            if(dst_y < warp_t)
            {
                src_y += (warp_t - dst_y);
                dst_h -= (warp_t - dst_y);
                dst_y = warp_t;
            }
        }
        else if(warp_dir == 4) // warp to right of player
        {
            if(dst_w > warp_r - dst_x)
                dst_w = warp_r - dst_x;
        }
        else if(warp_dir == 2) // warp to left of player
        {
            if(dst_x < warp_l)
            {
                src_x += (warp_l - dst_x);
                dst_w -= (warp_l - dst_x);
                dst_x = warp_l;
            }
        }

        if(dst_w < 0 || dst_h < 0)
            return true;
    }

    return false;
}

void RenderTexturePlayerScale(int Z, const Player_t& p,
                              int dst_x, int dst_y, int dst_w, int dst_h,
                              StdPicture& tex,
                              int src_x, int src_y, int src_w, int src_h,
                              XTColor color)
{
    if(LevelMacro != LEVELMACRO_KEYHOLE_EXIT || LevelMacroWhich == 0)
    {
        if(src_w == -1 || src_h == -1)
        {
            if(p.Effect == PLREFF_WARP_PIPE)
            {
                // apply warp to bounding box and return if sprite should not be drawn
                if(s_warp_gfx(Z, p, dst_x, dst_y, dst_w, dst_h, src_x, src_y))
                    return;
            }

            return XRender::renderTextureBasic(dst_x, dst_y, dst_w, dst_h, tex, src_x, src_y, color);
        }
        else
            return XRender::renderTextureScaleEx(dst_x, dst_y, dst_w, dst_h, tex, src_x, src_y, src_w, src_h, 0, nullptr, X_FLIP_NONE, color);
    }

    if(src_w == -1 || src_h == -1)
    {
        src_w = dst_w;
        src_h = dst_h;
    }

    const int keyholeMax = 192; // Was 300
    int keyholeDone = keyholeMax - 65;
    int ratio = 256 * LevelMacroCounter / keyholeDone;
    if(ratio > 255)
        ratio = 255;

    Background_t& keyhole = Background[LevelMacroWhich];

    double scale = ratio < 155 ? 1.0 : (double)(255 - ratio) / 100;

    double cx = vScreen[Z].X + keyhole.Location.X + keyhole.Location.Width / 2;
    double cy = vScreen[Z].Y + keyhole.Location.Y + 12;

    return XRender::renderTextureScaleEx(
                dst_x * scale + cx * (1 - scale),
                dst_y * scale + cy * (1 - scale),
                dst_w * scale, dst_h * scale,
                tex,
                src_x,
                src_y,
                src_w, src_h, 0, nullptr, X_FLIP_NONE,
                color);
}

void RenderTexturePlayer(int Z, const Player_t& p,
                         int dst_x, int dst_y, int dst_w, int dst_h,
                         StdPicture& tex,
                         int src_x, int src_y, XTColor color)
{
    RenderTexturePlayerScale(Z, p,
        dst_x, dst_y, dst_w, dst_h,
        tex,
        src_x, src_y, -1, -1,
        color);
}
