/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "renderop_bitmap.h"
#include "core/render.h"
#include <Utils/maths.h>
#include "globals.h"
#include "lunaimgbox.h"


RenderBitmapOp::RenderBitmapOp() : RenderOp()
{
    static_assert(sizeof(RenderBitmapOp) <= c_rAllocChunkSize,
                  "Size of RenderBitmapOp class must be smaller than c_rAllocChunkSize");
}

void RenderBitmapOp::Draw(Renderer *renderer)
{
    if(!direct_img || (direct_img->getH() == 0) || (direct_img->getW() == 0))
        return;

    float opacity = this->opacity;
    if(opacity > 1.0f) opacity = 1.0f;
    if(opacity < 0.0f) opacity = 0.0f;

    double screenX = this->x;
    double screenY = this->y;

    if(sceneCoords)
    {
        screenX -= vScreenX[renderer->GetCameraIdx()];
        screenY -= vScreenY[renderer->GetCameraIdx()];
    }

    // Get integer values as current rendering backends prefer that
    int x = Maths::iRound(screenX);
    int y = Maths::iRound(screenY);
    int sx = Maths::iRound(this->sx);
    int sy = Maths::iRound(this->sy);
    int width = Maths::iRound(this->sw);
    int height = Maths::iRound(this->sh);

    // Trim height/width if necessary
    if(direct_img->getW() < width + sx)
        width = direct_img->getW() - sx;

    if(direct_img->getH() < height + sy)
        height = direct_img->getH() - sy;

    // Don't render if no size
    if((width <= 0) || (height <= 0))
        return;

    XRender::renderTexture(x, y, width, height, direct_img->m_image, sx, sy, 1.f, 1.f, 1.f, opacity);
}
