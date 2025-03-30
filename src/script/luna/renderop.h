/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "sdl_proxy/sdl_assert.h"

#include "xt_color.h"
#include "draw_planes.h"

static const PLANE RENDEROP_PRIORITY_MIN = PLANE_INTERNAL_BG;
static const PLANE RENDEROP_PRIORITY_MAX = PLANE_INTERNAL_FG;
static const PLANE RENDEROP_DEFAULT_PRIORITY_RENDEROP = (PLANE)(PLANE_LVL_HUD - 2); // Default priority for RenderOp and RenderImage
static const PLANE RENDEROP_DEFAULT_PRIORITY_CGFX = (PLANE)(PLANE_LVL_HUD - 1); // Default priority for Custom GFX
static const PLANE RENDEROP_DEFAULT_PRIORITY_TEXT = (PLANE)(PLANE_LVL_HUD); // Default priority for Text

static const PLANE RENDEROP_DEFAULT_PRIORITY_RENDEROP_SCENE = (PLANE)(PLANE_LVL_SECTION_FG - 2); // Default priority for RenderOp and RenderImage (scene)
static const PLANE RENDEROP_DEFAULT_PRIORITY_CGFX_SCENE = (PLANE)(PLANE_LVL_SECTION_FG - 1); // Default priority for Custom GFX (scene)
static const PLANE RENDEROP_DEFAULT_PRIORITY_TEXT_SCENE = (PLANE)(PLANE_LVL_SECTION_FG); // Default priority for Text (scene)

// Base class respresenting a rendering operation
// Rendering operations include a draw function and a count of how many frames of activity remain
class RenderOp
{
public:
    RenderOp() : m_FramesLeft(1), m_selectedCamera(0), m_renderPriority(RENDEROP_DEFAULT_PRIORITY_RENDEROP) {}
    explicit RenderOp(PLANE priority) : m_FramesLeft(1), m_selectedCamera(0), m_renderPriority(priority) {}
    virtual ~RenderOp() = default;
    virtual void Draw(Renderer* /*renderer*/) {}

    inline void* operator new(size_t size)
    {
        // Note: If you creating any chunks with a size bigger than current size, please increase it
        SDL_assert_release(size < c_rAllocChunkSize);
        auto *ret = g_rAlloc.Allocate(c_rAllocChunkSize);
        return ret;
    }

    inline void operator delete(void* memory)
    {
        g_rAlloc.Free(memory);
    }

    int m_FramesLeft;		// How many frames until this op should be destroyed
    int m_selectedCamera;
    PLANE m_renderPriority;
};

#endif // RENDEROP_H
