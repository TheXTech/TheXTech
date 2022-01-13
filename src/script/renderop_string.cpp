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

#include "renderop_string.h"
#include "core/render.h"

RenderStringOp::RenderStringOp() : RenderStringOp(std::string(), 1, 400.f, 400.f) {}

RenderStringOp::RenderStringOp(const std::string &str, short font_type, float X, float Y) :
    RenderOp(RENDEROP_DEFAULT_PRIORITY_TEXT),
    m_String(str),
    m_FontType(font_type),
    m_X(X),
    m_Y(Y),
    sceneCoords(false)
{}

void RenderStringOp::Draw(Renderer *renderer)
{
    //        VB6StrPtr text(m_String);
    float x = m_X, y = m_Y;

    if(sceneCoords)
    {
        x -= vScreenX[renderer->GetCameraIdx()];
        y -= vScreenY[renderer->GetCameraIdx()];
    }

    if(!sceneCoords)
        XRender::offsetViewportIgnore(true);

    SuperPrint(m_String, m_FontType, x, y);

    if(!sceneCoords)
        XRender::offsetViewportIgnore(false);
}
