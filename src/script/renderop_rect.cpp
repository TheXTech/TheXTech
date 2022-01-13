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

#include "renderop_rect.h"
#include "core/render.h"
#include "globals.h"


void RenderRectOp::Draw(Renderer *renderer)
{
    if(borderColor.a <= 0.0 && fillColor.a <= 0.0) return;

    // Convert coordinates
    double x1 = this->x1, y1 = this->y1, x2 = this->x2, y2 = this->y2;
    if(sceneCoords)
    {
        x1 -= vScreenX[renderer->GetCameraIdx()];
        y1 -= vScreenY[renderer->GetCameraIdx()];
        x2 -= vScreenX[renderer->GetCameraIdx()];
        y2 -= vScreenY[renderer->GetCameraIdx()];
    }

    if(!sceneCoords)
        XRender::offsetViewportIgnore(true);

    if(fillColor.a > 0.0f)
        XRender::renderRect(x1, y1, x2 - x1, y2 - y1,
                            fillColor.r, fillColor.g, fillColor.b, fillColor.a, true);

    if(borderColor.a > 0.0f)
        XRender::renderRect(x1, y1, x2 - x1, y2 - y1,
                            borderColor.r, borderColor.g, borderColor.b, borderColor.a, false);

    if(!sceneCoords)
        XRender::offsetViewportIgnore(false);
}

