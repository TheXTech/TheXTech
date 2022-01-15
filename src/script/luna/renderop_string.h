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

#ifndef RENDEROP_STRING_H
#define RENDEROP_STRING_H

#include "renderop.h"
#include "graphics.h"

// String object to be rendered later
class RenderStringOp final : public RenderOp
{
public:
    // Quick ctor
    RenderStringOp();

    RenderStringOp(const std::string &str, int font_type, float X, float Y);

    ~RenderStringOp() override = default;

    void Draw(Renderer *renderer) override;

    std::string m_String;
    int m_FontType;
    float m_X;
    float m_Y;
    bool   sceneCoords;     // If true, x and y are scene coordinates
};

#endif // RENDEROP_STRING_H
