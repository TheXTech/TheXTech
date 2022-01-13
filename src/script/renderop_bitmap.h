/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

class RenderBitmapOp : public RenderOp
{
public:
    RenderBitmapOp();
    void Draw(Renderer* renderer);

    double x = 0.0;				// Absolute screen x position
    double y = 0.0;				// Absolute screen y position
    double sx = 0.0;				// Source x1 (left edge)
    double sy = 0.0;				// Source y1 (top edge)
    double sw = 0.0;				// Source x1 (right edge)
    double sh = 0.0;				// Source x1 (bottom edge)

    float  opacity = 0.0f;         // Opacity value
    bool   sceneCoords = false;     // If true, x and y are scene coordinates

    std::shared_ptr<LunaImage> direct_img;
};

#endif // RENDERBITMAPOP_H
