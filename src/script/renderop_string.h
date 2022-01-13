#ifndef RENDEROP_STRING_H
#define RENDEROP_STRING_H

#include "renderop.h"
#include "graphics.h"

// String object to be rendered later
class RenderStringOp : public RenderOp
{
public:
    // Quick ctor
    RenderStringOp();

    RenderStringOp(const std::string &str, short font_type, float X, float Y);

    virtual ~RenderStringOp() = default;

    virtual void Draw(Renderer *renderer);

    std::string m_String;
    short m_FontType;
    float m_X;
    float m_Y;
    bool   sceneCoords;     // If true, x and y are scene coordinates
};

#endif // RENDEROP_STRING_H
