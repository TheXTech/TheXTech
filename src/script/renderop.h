#pragma once
#ifndef RENDEROP_H
#define RENDEROP_H

#include "lunarender.h"

static const double RENDEROP_PRIORITY_MIN = -100.0;
static const double RENDEROP_PRIORITY_MAX = 10.0;
static const double RENDEROP_DEFAULT_PRIORITY_RENDEROP = 1.0; // Default priority for RenderOp and RenderImage
static const double RENDEROP_DEFAULT_PRIORITY_CGFX = 2.0; // Default priority for Custom GFX
static const double RENDEROP_DEFAULT_PRIORITY_TEXT = 3.0; // Default priority for Text

struct RenderOpColor
{
    float r;
    float g;
    float b;
    float a;

    inline RenderOpColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {};
};

// Base class respresenting a rendering operation
// Rendering operations include a draw function and a count of how many frames of activity remain
class RenderOp {
public:
    RenderOp() : m_FramesLeft(1), m_selectedCamera(0), m_renderPriority(RENDEROP_DEFAULT_PRIORITY_RENDEROP) { }
    RenderOp(double priority) : m_FramesLeft(1), m_selectedCamera(0), m_renderPriority(priority) { }
    virtual ~RenderOp() {}
    virtual void Draw(Renderer* /*renderer*/) {}

    int m_FramesLeft;		// How many frames until this op should be destroyed
    int m_selectedCamera;
    double m_renderPriority;
};

#endif // RENDEROP_H
