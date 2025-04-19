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
#ifndef RENDEROP_EFFECT_H
#define RENDEROP_EFFECT_H

#include "renderop.h"

typedef uint32_t COLORREF;

enum RENDER_EFFECT
{
    RNDEFF_ScreenGlow,
    RNDEFF_Flip
};

enum BLEND_TYPE
{
    BLEND_Additive,
    BLEND_Subtractive
};

enum FLIP_TYPE
{
    FLIP_TYPE_NONE = 0x0,
    FLIP_TYPE_X = 0x1,
    FLIP_TYPE_Y = 0x2,
    FLIP_TYPE_XY = 0x3,
};

class RenderEffectOp final : public RenderOp
{
public:
    RenderEffectOp();
    explicit RenderEffectOp(RENDER_EFFECT effect, BLEND_TYPE blend, COLORREF col, int intensity);

    ~RenderEffectOp() override = default;

    void Draw(Renderer *renderer) override;

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

    // Effects //
    void ScreenGlow(Renderer *renderer);
    void Flip(Renderer *renderer);

    // Members //
    RENDER_EFFECT effect_type;
    BLEND_TYPE blend_type;
    COLORREF color;
    int intensity;
    FLIP_TYPE flip_type;
};


#endif // RENDEROP_EFFECT_H
