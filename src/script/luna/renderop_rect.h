/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef RENDERRECTOP_H
#define RENDERRECTOP_H

#include "renderop.h"

class RenderRectOp final : public RenderOp
{
public:
    RenderRectOp() : x1(0), y1(0), x2(0), y2(0),
        fillColor(0.0, 0.0, 0.0, 0.0),
        borderColor(1.0f, 1.0f, 1.0f, 1.0f),
        sceneCoords(false)
    {};

    void Draw(Renderer *renderer) override;

    double x1;
    double y1;
    double x2;
    double y2;
    RenderOpColor fillColor;
    RenderOpColor borderColor;
    bool   sceneCoords;        // If true, x and y are scene coordinates
};


#endif // RENDERRECTOP_H
