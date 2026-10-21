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

#pragma once
#ifndef GFX_BACKGROUND_XT_H
#define GFX_BACKGROUND_XT_H

#include <cstdint>
#include <array>

struct StdPicture;

struct XTBackgroundLayer
{
    uint16_t sourceHeight;
    uint16_t sourceWidth;
    uint16_t sourceY;
    uint8_t sourceX; // * 32

    int8_t offsetX; // * 32
    int16_t offsetY;
    uint16_t unitHeight;
    int16_t inUnitOffsetY;

    uint8_t parallaxX; // / 100
    uint8_t parallaxY; // / 100
    int8_t speedX; // / 10
    int8_t speedY; // / 10

    uint8_t frames;
    uint8_t framespeed;
    uint8_t startingFrame;
    uint8_t frameStride; // * 32

    uint16_t flags;

    enum flags
    {
        // if CENTERX is set, RIGHT_OR_FITX determines FITX. otherwise, determines left/right.
        ALIGN_CENTERX       = (1 <<  0),
        ALIGN_RIGHT_OR_FITX = (1 <<  1),
        // if CENTERY is set, BOTTOM_OR_FITY determines FITY. otherwise, determines top/bottom.
        ALIGN_CENTERY       = (1 <<  2),
        ALIGN_BOTTOM_OR_FITY= (1 <<  3),
        REPEAT_TOP          = (1 <<  4),
        REPEAT_BOTTOM       = (1 <<  5),
        REPEAT_LEFT         = (1 <<  6),
        REPEAT_RIGHT        = (1 <<  7),
        REPEAT_FLIPX        = (1 <<  8),
        REPEAT_FLIPY        = (1 <<  9),
        FOREGROUND          = (1 << 10),
        FRAME_PINGPONG      = (1 << 11),
        REPEAT_STRIPY       = (1 << 12), // repeat Y by strips, instead of by units
        FALLBACK_BOTTOM     = (1 << 13), // internal, fit falls back to bottom alignment
    };
};

struct XTBackground
{
    XTColor bg_color;
    std::array<XTBackgroundLayer, 16> layers;
};

struct XTBackgroundContext
{
    int vscreenw_minus1;
    int vscreenh_minus1;

    int camX_sectX;
    int camY_sectY;
    int section_w;
    int section_h;

    int screenx;
    int screeny;
    int screenw;
    int screenh;

    XTBackgroundContext(int S, int Z);
};

void DrawXTBackgroundLayer(StdPicture& source, const XTBackgroundLayer& layer, const XTBackgroundContext& context);

// void DrawXTBackground(StdPicture& source, const XTBackground& bg, int S, int Z, bool FG);

#endif // #ifndef GFX_BACKGROUND_XT_H
