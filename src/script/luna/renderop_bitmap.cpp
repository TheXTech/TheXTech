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

#include "renderop_bitmap.h"
#include <Utils/maths.h>
#include "globals.h"


RenderBitmapOp::RenderBitmapOp() = default;

void RenderBitmapOp::Draw(Renderer *renderer)
{
    UNUSED(renderer);
    // TODO: Implement this using proper stuff

//    if (!direct_img || (direct_img->getH() == 0) || (direct_img->getW() == 0)) {
//        return;
//    }

//    float opacity = this->opacity;
//    if (opacity > 1.0f) opacity = 1.0f;
//    if (opacity < 0.0f) opacity = 0.0f;

//    double screenX = this->x;
//    double screenY = this->y;
//    if (sceneCoords) {
//        SMBX_CameraInfo::transformSceneToScreen(renderer->GetCameraIdx(), screenX, screenY);
//    }

//    // Get integer values as current rendering backends prefer that
//    int x = static_cast<int>(round(screenX));
//    int y = static_cast<int>(round(screenY));
//    int sx = static_cast<int>(round(this->sx));
//    int sy = static_cast<int>(round(this->sy));
//    int width = static_cast<int>(round(this->sw));
//    int height = static_cast<int>(round(this->sh));

//    // Trim height/width if necessary
//    if (direct_img->getW() < static_cast<unsigned int>(width + sx))
//    {
//        width = direct_img->getW() - sx;
//    }
//    if (direct_img->getH() < static_cast<unsigned int>(height + sy))
//    {
//        height = direct_img->getH() - sy;
//    }

//    // Don't render if no size
//    if ((width <= 0) || (height <= 0))
//    {
//        return;
//    }

//    direct_img->draw(x, y, width, height, sx, sy, true, true, opacity);
}
