#ifndef RENDERRECTOP_H
#define RENDERRECTOP_H

#include "renderop.h"

class RenderRectOp : public RenderOp
{
public:
    RenderRectOp() : x1(0), y1(0), x2(0), y2(0),
        borderColor(1.0f, 1.0f, 1.0f, 1.0f),
        fillColor(0.0, 0.0, 0.0, 0.0), sceneCoords(false) {};

    void Draw(Renderer *renderer);

    double x1;
    double y1;
    double x2;
    double y2;
    RenderOpColor fillColor;
    RenderOpColor borderColor;
    bool   sceneCoords;        // If true, x and y are scene coordinates
};


#endif // RENDERRECTOP_H
