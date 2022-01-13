#include "renderop_string.h"
#include "core/render.h"

RenderStringOp::RenderStringOp() : RenderStringOp(std::string(), 1, 400.f, 400.f) {}

RenderStringOp::RenderStringOp(const std::string &str, short font_type, float X, float Y) :
    RenderOp(RENDEROP_DEFAULT_PRIORITY_TEXT),
    m_String(str),
    m_FontType(font_type),
    m_X(X),
    m_Y(Y),
    sceneCoords(false)
{}

void RenderStringOp::Draw(Renderer *renderer)
{
    //        VB6StrPtr text(m_String);
    float x = m_X, y = m_Y;

    if(sceneCoords)
    {
        x -= vScreenX[renderer->GetCameraIdx()];
        y -= vScreenY[renderer->GetCameraIdx()];
    }

    if(!sceneCoords)
        XRender::offsetViewportIgnore(true);

    SuperPrint(m_String, m_FontType, x, y);

    if(!sceneCoords)
        XRender::offsetViewportIgnore(false);
}
