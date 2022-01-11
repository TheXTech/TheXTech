#ifndef RENDEROP_STRING_H
#define RENDEROP_STRING_H

#include "renderop.h"
#include "graphics.h"

// String object to be rendered later
class RenderStringOp : public RenderOp
{
public:
    // Quick ctor
    RenderStringOp() : RenderStringOp("", 1, 400.f, 400.f) {}

    RenderStringOp(const std::string &str, short font_type, float X, float Y) :
        RenderOp(RENDEROP_DEFAULT_PRIORITY_TEXT),
        m_String(str),
        m_FontType(font_type),
        m_X(X),
        m_Y(Y),
        sceneCoords(false)
    {}

    virtual ~RenderStringOp() { }
    virtual void Draw(Renderer* renderer)
    {
//        VB6StrPtr text(m_String);
        float x = m_X, y = m_Y;
        if(sceneCoords)
        {
            x -= vScreenX[renderer->GetCameraIdx()];
            y -= vScreenY[renderer->GetCameraIdx()];
        }
        SuperPrint(m_String, m_FontType, x, y);
    }

    std::string m_String;
    short m_FontType;
    float m_X;
    float m_Y;
    bool   sceneCoords;     // If true, x and y are scene coordinates
};

#endif // RENDEROP_STRING_H
