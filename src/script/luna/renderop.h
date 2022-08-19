/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#pragma once
#ifndef RENDEROP_H
#define RENDEROP_H

#include "lunarender.h"
#include "core/std.h"


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
class RenderOp
{
public:
    RenderOp() : m_FramesLeft(1), m_selectedCamera(0), m_renderPriority(RENDEROP_DEFAULT_PRIORITY_RENDEROP) {}
    explicit RenderOp(double priority) : m_FramesLeft(1), m_selectedCamera(0), m_renderPriority(priority) {}
    virtual ~RenderOp() = default;
    virtual void Draw(Renderer* /*renderer*/) {}

    inline void* operator new(size_t size)
    {
        // Note: If you creating any chunks with a size bigger than current size, please increase it
        TXT_assert_release(size < c_rAllocChunkSize);
        auto *ret = g_rAlloc.Allocate(c_rAllocChunkSize);
        return ret;
    }

    inline void operator delete(void* memory)
    {
        g_rAlloc.Free(memory);
    }

    int m_FramesLeft;		// How many frames until this op should be destroyed
    int m_selectedCamera;
    double m_renderPriority;
};

#endif // RENDEROP_H
