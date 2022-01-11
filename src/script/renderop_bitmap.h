#pragma once
#ifndef RENDERBITMAPOP_H
#define RENDERBITMAPOP_H

#include <memory>
#include "renderop.h"

class RenderBitmapOp : public RenderOp
{
public:
    RenderBitmapOp();
    void Draw(Renderer* renderer);

    double x = 0.0;				// Absolute screen x position
    double y = 0.0;				// Absolute screen y position
    double sx = 0.0;				// Source x1 (left edge)
    double sy = 0.0;				// Source y1 (top edge)
    double sw = 0.0;				// Source x1 (right edge)
    double sh = 0.0;				// Source x1 (bottom edge)

    float  opacity = 0.0f;         // Opacity value
    bool   sceneCoords = false;     // If true, x and y are scene coordinates

    std::shared_ptr<LunaImage> direct_img;
};

#endif // RENDERBITMAPOP_H
