/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef GL_GEOM_H
#define GL_GEOM_H

#include <cmath>

#include "gl_inc.h"

struct PointI
{
    GLshort x;
    GLshort y;

    PointI() = default;
    constexpr PointI(GLshort x, GLshort y) :
        x(x), y(y) {}

    constexpr PointI operator-() const
    {
        return PointI(-x, -y);
    }

    inline const PointI& operator+=(const PointI& o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    inline const PointI& operator-=(const PointI& o)
    {
        return *this += -o;
    }
};

struct PointF
{
    GLfloat x;
    GLfloat y;

    constexpr PointF(GLfloat x, GLfloat y) :
        x(x), y(y) {}

    constexpr PointF(const PointI& o) :
        x(o.x), y(o.y) {}

    constexpr PointF operator-() const
    {
        return PointF(-x, -y);
    }

    inline const PointF& operator+=(const PointF& o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    inline const PointF& operator-=(const PointF& o)
    {
        return *this += -o;
    }

    inline const PointF& operator*=(const PointF& o)
    {
        x *= o.x;
        y *= o.y;
        return *this;
    }

    inline const PointF& operator/=(const PointF& o)
    {
        x /= o.x;
        y /= o.y;
        return *this;
    }
};

struct RectI
{
    PointI tl;
    PointI br;

    constexpr RectI(int l, int t, int r, int b) :
        tl(l, t), br(r, b) {}

    inline const RectI& operator+=(const PointI& o)
    {
        tl += o;
        br += o;
        return *this;
    }

    inline const RectI& operator-=(const PointI& o)
    {
        tl -= o;
        br -= o;
        return *this;
    }
};

struct RectF
{
    PointF tl;
    PointF br;

    constexpr RectF(GLfloat l, GLfloat t, GLfloat r, GLfloat b) :
        tl(l, t), br(r, b) {}

    constexpr explicit RectF(const RectI& o) :
        tl(o.tl), br(o.br) {}

    inline const RectF& operator+=(const PointF& o)
    {
        tl += o;
        br += o;
        return *this;
    }

    inline const RectF& operator-=(const PointF& o)
    {
        tl -= o;
        br -= o;
        return *this;
    }

    inline const RectF& operator*=(const PointF& o)
    {
        tl *= o;
        br *= o;
        return *this;
    }

    inline const RectF& operator/=(const PointF& o)
    {
        tl /= o;
        br /= o;
        return *this;
    }
};

struct QuadI
{
    PointI tl;
    PointI tr;
    PointI bl;
    PointI br;

    QuadI() = default;
    explicit constexpr QuadI(const RectI& r) :
        tl(r.tl.x, r.tl.y),
        tr(r.br.x, r.tl.y),
        bl(r.tl.x, r.br.y),
        br(r.br.x, r.br.y) {}

    inline const QuadI& operator+=(const PointI& o)
    {
        tl += o;
        tr += o;
        bl += o;
        br += o;
        return *this;
    }

    inline const QuadI& operator-=(const PointI& o)
    {
        tl -= o;
        tr -= o;
        bl -= o;
        br -= o;
        return *this;
    }

    // returns a Quad rotated about the origin by angle (specified in clockwise degrees)
    inline QuadI rotate(double angle) const
    {
        angle *= -2.0 * M_PI / 360.0;

        auto cosa = cos(angle);
        auto sina = sin(angle);

        QuadI ret;
        ret.tl.x = tl.x *  cosa + tl.y * sina;
        ret.tl.y = tl.x * -sina + tl.y * cosa;

        ret.tr.x = tr.x *  cosa + tr.y * sina;
        ret.tr.y = tr.x * -sina + tr.y * cosa;

        ret.bl.x = bl.x *  cosa + bl.y * sina;
        ret.bl.y = bl.x * -sina + bl.y * cosa;

        ret.br.x = br.x *  cosa + br.y * sina;
        ret.br.y = br.x * -sina + br.y * cosa;

        return ret;
    }
};

#endif // #ifndef GL_GEOM_H
