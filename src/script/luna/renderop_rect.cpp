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

#include "renderop_rect.h"
#include "core/render.h"
#include "globals.h"


RenderRectOp::RenderRectOp() : RenderOp(),
    x1(0), y1(0), x2(0), y2(0),
    fillColor(0, 0, 0, 0),
    borderColor(255, 255, 255, 255),
    sceneCoords(false)
{
    static_assert(sizeof(RenderRectOp) <= c_rAllocChunkSize,
            "Size of RenderRectOp class must be smaller than c_rAllocChunkSize");
}

void RenderRectOp::Draw(Renderer *renderer)
{
    if(borderColor.a <= 0 && fillColor.a <= 0) return;

    // Convert coordinates
    int sx1 = this->x1, sy1 = this->y1, sx2 = this->x2, sy2 = this->y2;
    if(sceneCoords)
    {
        sx1 -= vScreen[renderer->GetCameraIdx()].CameraAddX();
        sy1 -= vScreen[renderer->GetCameraIdx()].CameraAddY();
        sx2 -= vScreen[renderer->GetCameraIdx()].CameraAddX();
        sy2 -= vScreen[renderer->GetCameraIdx()].CameraAddY();
    }
    else
    {
        if(sx1 == 0 && sx2 == 800 && sy1 == 0 && sy2 == 600)
        {
            sx2 = vScreen[1].Width;
            sy2 = vScreen[1].Height;
        }
        else
        {
            Render::TranslateScreenCoords(sx1, sy1, sx2 - sx1, sy2 - sy1);
        }
    }

    if(fillColor.a > 0)
        XRender::renderRect(sx1,
                            sy1,
                            sx2 - sx1,
                            sy2 - sy1,
                            fillColor, true);

    if(borderColor.a > 0)
        XRender::renderRect(sx1,
                            sy1,
                            sx2 - sx1,
                            sy2 - sy1,
                            borderColor, false);
}

