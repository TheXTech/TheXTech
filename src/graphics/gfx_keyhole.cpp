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

void RenderKeyhole(int Z)
{
    const unsigned keyholeMax = 192; // Was 300
    unsigned keyholeDone = keyholeMax - 65;
    unsigned ratio = 328 * LevelMacroCounter / keyholeDone;
    if(ratio > 327)
        ratio = 327;

    Background_t& keyhole = Background[LevelMacroWhich];

    num_t realKeyholeBottom = keyhole.Location.Y + 24;
    num_t idealKeyholeBottom = 32 * num_t::ceil(realKeyholeBottom / 32);

    // basis of 128
    unsigned keyholeGrowthCoord = ratio;

    if(keyholeGrowthCoord > 128)
        keyholeGrowthCoord = 128;

    // basis of 128
    unsigned keyholeScale = keyholeGrowthCoord * 12;

    if(ratio < 164)
        keyholeScale += (128 - keyholeGrowthCoord);

    num_t keyholeBottom = (realKeyholeBottom * (128 - keyholeGrowthCoord) + idealKeyholeBottom * keyholeGrowthCoord) / 128;

    RenderTexturePlayerScale(Z,
        num_t::round(vScreen[Z].X + keyhole.Location.X + keyhole.Location.Width / 2 - keyhole.Location.Width * keyholeScale / 256),
        num_t::round(vScreen[Z].Y + keyholeBottom - 24 * keyholeScale / 128),
        (int)(keyhole.Location.Width) * keyholeScale / 128,
        (int)(keyhole.Location.Height) * keyholeScale / 128,
        GFXBackgroundBMP[keyhole.Type],
        0,
        0,
        (int)(keyhole.Location.Width),
        (int)(keyhole.Location.Height));

    // hide the real keyhole underneath
    if(ratio >= 164)
        keyhole.Hidden = true;
}

void RenderTexturePlayerScale(int Z, int dst_x, int dst_y, int dst_w, int dst_h,
                         StdPicture& tex,
                         int src_x, int src_y, int src_w, int src_h,
                         XTColor color)
{
    if(LevelMacro != LEVELMACRO_KEYHOLE_EXIT || LevelMacroWhich == 0)
    {
        if(src_w == -1 || src_h == -1)
            return XRender::renderTextureBasic(dst_x, dst_y, dst_w, dst_h, tex, src_x, src_y, color);
        else
            return XRender::renderTextureScaleEx(dst_x, dst_y, dst_w, dst_h, tex, src_x, src_y, src_w, src_h, 0, nullptr, X_FLIP_NONE, color);
    }

    if(src_w == -1 || src_h == -1)
    {
        src_w = dst_w;
        src_h = dst_h;
    }

    const unsigned keyholeMax = 192; // Was 300
    unsigned keyholeDone = keyholeMax - 65;
    unsigned ratio = 328 * LevelMacroCounter / keyholeDone;
    if(ratio > 327)
        ratio = 327;

    Background_t& keyhole = Background[LevelMacroWhich];

    // basis of 128
    int scale = (ratio < 199) ? 128 : (327 - ratio);

    num_t cx = vScreen[Z].X + keyhole.Location.X + keyhole.Location.Width / 2;
    num_t cy = vScreen[Z].Y + keyhole.Location.Y + 12;

    return XRender::renderTextureScaleEx(
                (int)(dst_x * scale + cx * (128 - scale)) / 128,
                (int)(dst_y * scale + cy * (128 - scale)) / 128,
                dst_w * scale / 128, dst_h * scale / 128,
                tex,
                src_x,
                src_y,
                src_w, src_h, 0, nullptr, X_FLIP_NONE,
                color);
}

void RenderTexturePlayer(int Z, int dst_x, int dst_y, int dst_w, int dst_h,
                         StdPicture& tex,
                         int src_x, int src_y, XTColor color)
{
    RenderTexturePlayerScale(Z, dst_x, dst_y, dst_w, dst_h,
        tex,
        src_x, src_y, -1, -1,
        color);
}
