/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "Logger/logger.h"

#include "globals.h"
#include "graphics.h"
#include "collision.h"
#include "core/render.h"
#include "graphics/gfx_background_xt.h"

static int s_floor_div(int num, unsigned den)
{
    if(num < 0)
        num -= (den - 1);

    return num / (int)den;
}

XTBackgroundContext::XTBackgroundContext(int S, int Z)
{
    const vScreen_t& vscreen = vScreen[Z];
    const Screen_t& screen = Screens[vscreen.screen_ref];
    const auto& sect = LevelREAL[S];

    int camX = vscreen.CameraAddX_i();
    int camY = vscreen.CameraAddY_i();

    vscreenw_minus1 = vscreen.Width - 1;
    vscreenh_minus1 = vscreen.Height - 1;

    camX_sectX = camX + sect.X;
    camY_sectY = camY + sect.Y;
    section_w = sect.Width - sect.X;
    section_h = sect.Height - sect.Y;

    screenx = 0;
    screeny = 0;
    screenw = vscreen.Width;
    screenh = vscreen.Height;

    if(screen.Type == ScreenTypes::Dynamic && (screen.DType == DScreenTypes::LeftRight || screen.DType == DScreenTypes::RightLeft))
    {
        screenw *= 2;
        screenx = -vscreen.Left;
    }

    if(screen.Type == ScreenTypes::Dynamic && (screen.DType == DScreenTypes::TopBottom || screen.DType == DScreenTypes::BottomTop))
    {
        screenh *= 2;
        screeny = -vscreen.Top;
    }
}

void DrawXTBackgroundLayer(StdPicture& source, const XTBackgroundLayer& layer, const XTBackgroundContext& context)
{
    // step -1: get BG2 dimensions
    int sourceWidth = layer.sourceWidth;
    if(!sourceWidth)
        sourceWidth = source.w;

    int sourceHeight = layer.sourceHeight;
    if(!sourceHeight)
    {
        if(layer.frameStride)
            sourceHeight = layer.frameStride;
        else
        {
            sourceHeight = source.h;
            if(layer.frames)
                sourceHeight /= layer.frames;
        }
    }

    int unitHeight = layer.unitHeight;
    if(!unitHeight)
        unitHeight = sourceHeight;

    if(sourceWidth == 0 || sourceHeight == 0 || unitHeight == 0)
        return;

    // step 1: calculate X anchor
    int anchorX = 0;
    // fitX -- if the screen is bigger than the item, fit the original item plus padding to the left/right. this leads to the 2 * screenw - sourceWidth condition.
    if((layer.flags & XTBackgroundLayer::ALIGN_CENTERX) && (layer.flags & XTBackgroundLayer::ALIGN_RIGHT_OR_FITX) && sourceWidth < context.section_w && 2 * context.screenw - sourceWidth < context.section_w)
    {
        num_t section_ratio = (num_t)(-context.camX_sectX + context.screenx) / (context.section_w - context.screenw);
        int offset_num = sourceWidth - context.screenw;
        anchorX = num_t::round(-offset_num * section_ratio) + context.screenx;
        if(sourceWidth < context.screenw)
            anchorX = -anchorX + (context.screenw - sourceWidth);
    }
    // screen coordinates
    else if(layer.parallaxX == 0)
    {
        anchorX = context.screenx;

        if(layer.flags & XTBackgroundLayer::ALIGN_CENTERX)
            anchorX += (context.screenw - sourceWidth) / 2;
        else if(layer.flags & XTBackgroundLayer::ALIGN_RIGHT_OR_FITX)
            anchorX += context.screenw - sourceWidth;
    }
    // camera coordinates
    else
    {
        int zero_point = 0;

        if(layer.flags & XTBackgroundLayer::ALIGN_CENTERX)
        {
            anchorX = (context.section_w - sourceWidth) / 2;
            zero_point = (context.screenw - sourceWidth) / 2;
        }
        else if(layer.flags & XTBackgroundLayer::ALIGN_RIGHT_OR_FITX)
        {
            anchorX = (context.section_w - sourceWidth);
            zero_point = (context.screenw - sourceWidth);
        }

        zero_point += context.screenx;
        anchorX += context.camX_sectX;
        anchorX -= zero_point;

        anchorX *= layer.parallaxX;
        if(anchorX < 0)
            anchorX -= 49;
        else
            anchorX += 50;
        anchorX /= 100;

        anchorX += zero_point;
    }

    // step 2: calculate Y anchor
    int anchorY = 0;
    // fitY -- if the screen is bigger than the item, fit the original item plus padding above/below. this leads to the 2 * screenh - unitHeight condition.
    // when the section is too small, fall back to a bottom-alignment (like SMBX 1.3)
    if((layer.flags & XTBackgroundLayer::ALIGN_CENTERY) && (layer.flags & XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY) && unitHeight < context.section_h && 2 * context.screenh - unitHeight < context.section_h)
    {
        num_t section_ratio = (num_t)(-context.camY_sectY + context.screeny) / (context.section_h - context.screenh);
        int offset_num = unitHeight - context.screenh;
        anchorY = num_t::round(-offset_num * section_ratio) + context.screeny;
        if(unitHeight < context.screenh)
            anchorY = -anchorY + (context.screenh - unitHeight);
    }
    // screen coordinates
    else if(layer.parallaxY == 0)
    {
        if(layer.flags & XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY)
            anchorY = context.screenh - unitHeight;
        else if(layer.flags & XTBackgroundLayer::ALIGN_CENTERY)
            anchorY = (context.screenh - unitHeight) / 2;

        anchorY += context.screeny;
    }
    // camera coordinates
    else
    {
        int zero_point = 0;

        if(layer.flags & XTBackgroundLayer::ALIGN_BOTTOM_OR_FITY && (!(layer.flags & XTBackgroundLayer::ALIGN_CENTERY) || (layer.flags & XTBackgroundLayer::FALLBACK_BOTTOM)))
        {
            anchorY = (context.section_h - unitHeight);
            zero_point = (context.screenh - unitHeight);
        }
        else if(layer.flags & XTBackgroundLayer::ALIGN_CENTERY)
        {
            anchorY = (context.section_h - unitHeight) / 2;
            zero_point = (context.screenh - unitHeight) / 2;
        }

        zero_point += context.screeny;
        anchorY += context.camY_sectY;
        anchorY -= zero_point;

        anchorY *= layer.parallaxY;
        if(anchorY < 0)
            anchorY -= 49;
        else
            anchorY += 50;
        anchorY /= 100;

        anchorY += zero_point;
    }

    // step 3: apply offsets
    anchorX += layer.offsetX * 32;
    anchorY += layer.offsetY;

    // step 4: apply speed
    anchorX += (CommonFrame_NotFrozen * layer.speedX + 5) / 10;
    anchorY += (CommonFrame_NotFrozen * layer.speedY + 5) / 10;

    // step 5: calculate frame offset
    unsigned frame_offset_y = 0;
    if(layer.frames && layer.framespeed)
    {
        unsigned frames = layer.frames;
        if((layer.flags & XTBackgroundLayer::FRAME_PINGPONG) && layer.frames >= 2)
            frames += layer.frames - 2;

        unsigned frame_counter = CommonFrame_NotFrozen / layer.framespeed + layer.startingFrame;
        unsigned frame = frame_counter % frames;
        if((layer.flags & XTBackgroundLayer::FRAME_PINGPONG) && frame >= layer.frames)
            frame = layer.frames * 2 - 1 - frame;

        frame_offset_y = (layer.frameStride ? layer.frameStride : sourceHeight) * frame;
    }

    // step 6: calculate repeats
    unsigned inUnitOffsetY = layer.inUnitOffsetY;
    if(layer.flags & XTBackgroundLayer::REPEAT_STRIPY)
    {
        anchorY += inUnitOffsetY;
        inUnitOffsetY = 0;
        unitHeight = sourceHeight;
    }

    int firstX = s_floor_div(-anchorX, sourceWidth);
    int lastX = s_floor_div(-anchorX + context.vscreenw_minus1, sourceWidth);

    int firstY = s_floor_div(-anchorY, unitHeight);
    int lastY = s_floor_div(-anchorY + context.vscreenh_minus1, unitHeight);

    if(!(layer.flags & XTBackgroundLayer::REPEAT_LEFT))
    {
        if(lastX < 0)
            return;
        else if(firstX < 0)
            firstX = 0;
    }

    if(!(layer.flags & XTBackgroundLayer::REPEAT_RIGHT))
    {
        if(firstX > 0)
            return;
        else if(lastX > 0)
            lastX = 0;
    }

    if(!(layer.flags & XTBackgroundLayer::REPEAT_TOP))
    {
        if(lastY < 0)
            return;
        else if(firstY < 0)
            firstY = 0;
    }

    if(!(layer.flags & XTBackgroundLayer::REPEAT_BOTTOM))
    {
        if(firstY > 0)
            return;
        else if(lastY > 0)
            lastY = 0;
    }

    // step 7: render!
    int sourceX = layer.sourceX * 32;
    int sourceY = layer.sourceY + frame_offset_y;

    int first_drawX = anchorX + firstX * sourceWidth;
    int first_unitY = anchorY + firstY * unitHeight;

    // separated loop for complicated case
    if((layer.flags & XTBackgroundLayer::REPEAT_FLIPX) | (layer.flags & XTBackgroundLayer::REPEAT_FLIPY))
    {
        for(int y = firstY, unitY = first_unitY; y <= lastY; y++, unitY += unitHeight)
        {
            unsigned flip = X_FLIP_NONE;
            int drawY = unitY;
            if((layer.flags & XTBackgroundLayer::REPEAT_FLIPY) && (y & 1))
            {
                flip |= X_FLIP_VERTICAL;
                drawY += unitHeight - inUnitOffsetY - sourceHeight;
            }
            else
                drawY += inUnitOffsetY;

            for(int x = firstX, drawX = first_drawX; x <= lastX; x++, drawX += sourceWidth)
            {
                if((layer.flags & XTBackgroundLayer::REPEAT_FLIPX) && (x & 1))
                    flip |= X_FLIP_HORIZONTAL;

                XRender::renderTextureFL(drawX, drawY,
                    sourceWidth, sourceHeight,
                    source,
                    sourceX, sourceY,
                    0, nullptr, flip);
            }
        }
    }
    else
    {
        int first_drawY = first_unitY + inUnitOffsetY;
        int last_drawX = anchorX + lastX * sourceWidth;
        int last_drawY = anchorY + inUnitOffsetY + lastY * unitHeight;

        for(int drawY = first_drawY; drawY <= last_drawY; drawY += unitHeight)
        {
            for(int drawX = first_drawX; drawX <= last_drawX; drawX += sourceWidth)
            {
                XRender::renderTextureBasic(drawX, drawY,
                    sourceWidth, sourceHeight,
                    source,
                    sourceX, sourceY);
            }
        }
    }
}

