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

#include "globals.h"
#include "compat.h"
#include "core/render.h"

#include "graphics/gfx_keyhole.h"

void RenderKeyhole(int Z)
{
    if(!g_compatibility.fix_keyhole_framerate)
        return;

    int keyholeMax = g_compatibility.fix_keyhole_framerate ? 192 : 300;
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

    RenderTexturePlayerScale(Z,
        vScreenX[Z] + keyhole.Location.X + keyhole.Location.Width / 2 - keyhole.Location.Width * keyholeScale / 2,
        vScreenY[Z] + keyholeBottom - 24 * keyholeScale,
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

void RenderTexturePlayerScale(int Z, double dst_x, double dst_y, double dst_w, double dst_h,
                         StdPicture& tex,
                         int src_x, int src_y, int src_w, int src_h,
                         float r, float g, float b, float a)
{
    if(LevelMacro != LEVELMACRO_KEYHOLE_EXIT || LevelMacroWhich == 0 || !g_compatibility.fix_keyhole_framerate)
    {
        if(src_w == -1 || src_h == -1)
            return XRender::renderTexture(dst_x, dst_y, dst_w, dst_h, tex, src_x, src_y, r, g, b, a);
        else
            return XRender::renderTextureScaleEx(dst_x, dst_y, dst_w, dst_h, tex, src_x, src_y, src_w, src_h, 0, nullptr, X_FLIP_NONE, r, g, b, a);
    }

    if(src_w == -1 || src_h == -1)
    {
        src_w = dst_w;
        src_h = dst_h;
    }

    int keyholeMax = g_compatibility.fix_keyhole_framerate ? 192 : 300;
    int keyholeDone = keyholeMax - 65;
    int ratio = 256 * LevelMacroCounter / keyholeDone;
    if(ratio > 255)
        ratio = 255;

    Background_t& keyhole = Background[LevelMacroWhich];

    double scale = ratio < 155 ? 1.0 : (double)(255 - ratio) / 100;

    double cx = vScreenX[Z] + keyhole.Location.X + keyhole.Location.Width / 2;
    double cy = vScreenY[Z] + keyhole.Location.Y + 12;

    return XRender::renderTextureScaleEx(
                dst_x * scale + cx * (1 - scale),
                dst_y * scale + cy * (1 - scale),
                dst_w * scale, dst_h * scale,
                tex,
                src_x,
                src_y,
                src_w, src_h, 0, nullptr, X_FLIP_NONE,
                r, g, b, a);
}

void RenderTexturePlayer(int Z, double dst_x, double dst_y, double dst_w, double dst_h,
                         StdPicture& tex,
                         int src_x, int src_y, float r, float g, float b, float a)
{
    RenderTexturePlayerScale(Z, dst_x, dst_y, dst_w, dst_h,
        tex,
        src_x, src_y, -1, -1,
        r, g, b, a);
}
