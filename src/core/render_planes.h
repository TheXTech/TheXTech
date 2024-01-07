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
#ifndef RENDER_PLANES_H
#define RENDER_PLANES_H

#include <array>
#include <cstdint>

#ifdef RENDER_WANTS_UNSIGNED_DEPTH
#    define RP_DEPTH_T uint16_t
#    define RP_BASE    plane_base
#else
#    define RP_DEPTH_T int16_t
#    define RP_BASE    plane_base_signed
#endif

/**
 * \brief A structure to help Z-buffered renderers keep track of the number of objects that have been rendered in each plane during a frame.
 */
struct RenderPlanes_t
{
    uint8_t m_current_plane = 0;

    std::array<RP_DEPTH_T, 256> m_plane_depth_default;
    std::array<RP_DEPTH_T, 256> m_plane_depth;

    // decompose plane into 0bxxxxxyyy (32 primary planes)
    // if yyy == 0b000, gets range starting at 0bxxxxxyyy 00000000 (1824 items)
    //       otherwise, gets range starting at 0bxxxxx111 yyy00000   (32 items)
    // add 128 to all returned values so that plane - 4, plane - 3, plane - 2, plane - 1 will have same upper 5 bits as plane.
    static constexpr uint16_t plane_base(uint8_t plane)
    {
        return ((plane & 7) ? (((plane | 7) << 8) | ((plane & 7) << 5)) : (plane << 8)) + 128;
    }

    static constexpr int16_t plane_base_signed(uint8_t plane)
    {
        return (plane_base(plane) & 0x8000) ? (plane_base(plane) & 0x7FFF) : (int16_t(plane_base(plane) & 0x7FFF) + INT16_MIN);
    }

    RenderPlanes_t()
    {
        for(int i = 0; i < 256; i++)
            m_plane_depth_default[i] = RP_BASE(i);
    }

    void reset()
    {
        m_current_plane = 0;
        m_plane_depth = m_plane_depth_default;
    }

    void set_plane(uint8_t plane)
    {
        m_current_plane = plane;
    }

    RP_DEPTH_T next()
    {
        return (m_plane_depth[m_current_plane]++);
    }
};

#endif // #ifndef RENDER_PLANES_H
