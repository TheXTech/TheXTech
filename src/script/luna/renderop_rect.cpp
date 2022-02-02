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

#include "renderop_rect.h"
#include "core/render.h"
#include "globals.h"


RenderRectOp::RenderRectOp() : RenderOp(),
    x1(0), y1(0), x2(0), y2(0),
    fillColor(0.0, 0.0, 0.0, 0.0),
    borderColor(1.0f, 1.0f, 1.0f, 1.0f),
    sceneCoords(false)
{
    static_assert(sizeof(RenderRectOp) <= c_rAllocChunkSize,
            "Size of RenderRectOp class must be smaller than c_rAllocChunkSize");
}

void RenderRectOp::Draw(Renderer *renderer)
{
    if(borderColor.a <= 0.0 && fillColor.a <= 0.0) return;

    // Convert coordinates
    double sx1 = this->x1, sy1 = this->y1, sx2 = this->x2, sy2 = this->y2;
    if(sceneCoords)
    {
        sx1 -= vScreenX[renderer->GetCameraIdx()];
        sy1 -= vScreenY[renderer->GetCameraIdx()];
        sx2 -= vScreenX[renderer->GetCameraIdx()];
        sy2 -= vScreenY[renderer->GetCameraIdx()];
    }

    if(!sceneCoords)
        XRender::offsetViewportIgnore(true);

    if(fillColor.a > 0.0f)
        XRender::renderRect(Maths::iRound(sx1),
                            Maths::iRound(sy1),
                            Maths::iRound(sx2 - sx1),
                            Maths::iRound(sy2 - sy1),
                            fillColor.r, fillColor.g, fillColor.b, fillColor.a, true);

    if(borderColor.a > 0.0f)
        XRender::renderRect(Maths::iRound(sx1),
                            Maths::iRound(sy1),
                            Maths::iRound(sx2 - sx1),
                            Maths::iRound(sy2 - sy1),
                            borderColor.r, borderColor.g, borderColor.b, borderColor.a, false);

    if(!sceneCoords)
        XRender::offsetViewportIgnore(false);
}

