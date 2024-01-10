/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef RENDER_OP_SDL_H
#define RENDER_OP_SDL_H

#include <cstdint>
#include <algorithm>
#include <vector>

#include <SDL2/SDL_assert.h>

#include "std_picture.h"

#include "rand.h"


struct RenderOp
{
    enum class Type : uint8_t
    {
        texture = 0,
        rect,
        circle,
        circle_hole
    };

    struct Traits
    {
        static constexpr uint8_t flip_X = 1;
        static constexpr uint8_t flip_Y = 2;
        static constexpr uint8_t rotation = 4;
        static constexpr uint8_t src_rect = 8;
        static constexpr uint8_t filled = 16;

        static constexpr uint8_t rotoflip = flip_X | flip_Y | rotation;
    };

    Type type = Type::texture;
    uint8_t traits = 0;

    int16_t xDst, yDst, wDst, hDst;

    XTColor color;

    // re-use wDst for circle radius since it is near the start of the struct
    inline int16_t& radius()
    {
        return wDst;
    }

    inline const int16_t& radius() const
    {
        return wDst;
    }

    int16_t xSrc, ySrc, wSrc, hSrc; // used if (traits & Traits::src_rect) is set

    uint16_t angle; // used if (traits & Traits::rotation) is set

    StdPicture* texture;
};

struct RenderQueue
{
    std::vector<RenderOp> ops;
    std::vector<int32_t> indices;
    uint16_t size = 0;

    inline void clear()
    {
        ops.clear();
        indices.clear();
        size = 0;
    }

    inline RenderOp& push(uint8_t plane)
    {
        if(size == UINT16_MAX)
        {
            SDL_assert_release(false); // render queue overflow
            return ops[size - 1];
        }

        ops.emplace_back();
        indices.push_back((int32_t)(((uint32_t)plane << 16) | size));
        return ops[size++];
    }

    inline void sort()
    {
        std::sort(indices.begin(), indices.end());
    }
};

#endif // #ifndef RENDER_OP_SDL_H
