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

#pragma once
#ifndef RENDERBITMAPOP_H
#define RENDERBITMAPOP_H

#include <memory>
#include "renderop.h"

class RenderBitmapOp final : public RenderOp
{
public:
    RenderBitmapOp();
    ~RenderBitmapOp() override = default;

    void Draw(Renderer* renderer) override;

    int x = 0;				// Absolute screen x position
    int y = 0;				// Absolute screen y position
    int sx = 0;				// Source x1 (left edge)
    int sy = 0;				// Source y1 (top edge)
    int sw = 0;				// Source x1 (right edge)
    int sh = 0;				// Source x1 (bottom edge)

    XTColor color = XTAlpha(255);         // Opacity value
    bool    sceneCoords = false;     // If true, x and y are scene coordinates

    LunaImage *direct_img = nullptr;
};

#endif // RENDERBITMAPOP_H
