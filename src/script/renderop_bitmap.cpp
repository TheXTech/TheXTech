#include "renderop_bitmap.h"
#include <Utils/maths.h>
#include "globals.h"


RenderBitmapOp::RenderBitmapOp()
{}

void RenderBitmapOp::Draw(Renderer *renderer)
{
    UNUSED(renderer);
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
