#include "renderop_rect.h"
#include "core/render.h"
#include "globals.h"


void RenderRectOp::Draw(Renderer *renderer)
{
    if (borderColor.a <= 0.0 && fillColor.a <= 0.0) return;

    // Convert coordinates
    double x1 = this->x1, y1 = this->y1, x2 = this->x2, y2 = this->y2;
    if (sceneCoords) {
        x1 -= vScreenX[renderer->GetCameraIdx()];
        y1 -= vScreenY[renderer->GetCameraIdx()];
        x2 -= vScreenX[renderer->GetCameraIdx()];
        y2 -= vScreenY[renderer->GetCameraIdx()];
    }

    if(fillColor.a > 0.0f)
        XRender::renderRect(x1, y1, x2 - x1, y2 - y1,
                            fillColor.r, fillColor.g, fillColor.b, fillColor.a, true);

    if(borderColor.a > 0.0f)
        XRender::renderRect(x1, y1, x2 - x1, y2 - y1,
                            borderColor.r, borderColor.g, borderColor.b, borderColor.a, false);
}
