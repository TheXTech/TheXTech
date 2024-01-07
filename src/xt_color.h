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
#ifndef XT_COLOR_H
#define XT_COLOR_H

#include <cstdint>
#include <string>

/*!
 * \brief RGBA pixel color
 */
struct alignas(uint32_t) XTColor
{
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;

    // helper functions
    static inline constexpr uint8_t mul(uint8_t a, uint8_t b)
    {
        return uint8_t((uint16_t(a) * uint16_t(b)) >> 8);
    }

    static inline constexpr uint8_t from_float(float f)
    {
        return static_cast<uint8_t>(f * 255.0f);
    }

    // initializers
    inline constexpr XTColor() {}
    inline constexpr XTColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    inline constexpr XTColor(XTColor o, uint8_t alpha) : r(o.r), g(o.g), b(o.b), a(alpha) {}

    // multiply operator
    inline constexpr XTColor operator*(XTColor o) const
    {
        return XTColor(mul(r, o.r), mul(g, o.g), mul(b, o.b), mul(a, o.a));
    }

    // intensity scale
    inline constexpr XTColor operator*(float o) const
    {
        return XTColor(uint8_t(r * o), uint8_t(g * o), uint8_t(b * o), a);
    }

    inline constexpr XTColor operator*(uint8_t o) const
    {
        return XTColor(mul(r, o), mul(g, o), mul(b, o), a);
    }

    // (in)equality operators
    inline constexpr bool operator==(XTColor o) const
    {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }

    inline constexpr bool operator!=(XTColor o) const
    {
        return !(*this == o);
    }

    // alpha change operators (return new colors)
    inline constexpr XTColor with_alpha(uint8_t new_a) const
    {
        return XTColor(r, g, b, new_a);
    }

    inline constexpr XTColor with_alphaF(float new_a) const
    {
        return XTColor(r, g, b, from_float(new_a));
    }
};

static inline constexpr XTColor XTColorF(float r, float g, float b, float a = 1.0f)
{
    return XTColor(XTColor::from_float(r),
        XTColor::from_float(g),
        XTColor::from_float(b),
        XTColor::from_float(a));
}

static inline constexpr XTColor XTAlpha(uint8_t a)
{
    return XTColor().with_alpha(a);
}

static inline constexpr XTColor XTAlphaF(float a)
{
    return XTColor().with_alpha(XTColor::from_float(a));
}

/**
 * \brief converts a color string into an XTColor instance
 *
 * \param s string reference
 *
 * Currently supports hex strings only. Defined in globals.cpp.
 **/
XTColor XTColorString(const std::string& s);

#endif
