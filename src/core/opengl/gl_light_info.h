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
#ifndef GL_LIGHT_INFO_H

#include <cstdint>

#include "xt_color.h"


typedef unsigned int    GLenum;
typedef int             GLint;
typedef unsigned int    GLuint;
typedef float           GLfloat;

enum class GLLightType : uint32_t
{
    none = 0,
    point,
    arc,
    bar,
    box,
    duplicate,
};

#ifdef THEXTECH_BIG_ENDIAN
struct GLLightColor
{
    constexpr GLLightColor() = default;
    constexpr GLLightColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)
        : a(a), r(r), g(g), b(b) {}
    constexpr GLLightColor(XTColor color)
        : a(color.a), r(color.r), g(color.g), b(color.b) {}

    uint8_t a = 0x00;
    uint8_t r = 0x00;
    uint8_t g = 0x00;
    uint8_t b = 0x00;
};
#else // #ifdef THEXTECH_BIG_ENDIAN
struct GLLightColor
{
    constexpr GLLightColor() = default;
    constexpr GLLightColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)
        : b(b), g(g), r(r), a(a) {}
    constexpr GLLightColor(XTColor color)
        : b(color.b), g(color.g), r(color.r), a(color.a) {}

    uint8_t b = 0x00;
    uint8_t g = 0x00;
    uint8_t r = 0x00;
    uint8_t a = 0x00;
};
#endif // #else (from #ifdef THEXTECH_BIG_ENDIAN)

using GLLightPos = std::array<GLfloat, 4>;


struct GLLight
{
    constexpr GLLight() = default;
    constexpr GLLight(GLLightType type, GLLightColor color, GLfloat radius, GLfloat depth, GLLightPos pos)
        : type(type), color(color), radius(radius), depth(depth), pos(pos) {}

    static constexpr GLLight Point(GLfloat x, GLfloat y, GLfloat depth, GLLightColor color, GLfloat radius)
    {
        return GLLight(GLLightType::point, color, radius, depth, {x, y, 0.0f, 0.0f});
    }

    static constexpr GLLight Box(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat depth, GLLightColor color, GLfloat radius)
    {
        return GLLight(GLLightType::box, color, radius, depth, {x1, y1, x2, y2});
    }

    static constexpr GLLight Bar(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat depth, GLLightColor color, GLfloat radius)
    {
        return GLLight(GLLightType::bar, color, radius, depth, {x1, y1, x2, y2});
    }

    static constexpr GLLight Arc(GLfloat x1, GLfloat y1, GLfloat angle, GLfloat width, GLfloat depth, GLLightColor color, GLfloat radius)
    {
        return GLLight(GLLightType::arc, color, radius, depth, {x1, y1, angle, width});
    }

    GLLightType type = GLLightType::none;
    GLLightColor color = GLLightColor{0, 0, 0, 0};
    GLfloat radius = 0.0f;
    GLfloat depth = 0.0f;
    GLLightPos pos = GLLightPos{0.0, 0.0, 0.0, 0.0};

    // frustrating, but at least it's here. observe that pos is not sorted; that's intentional because calling code may sort it in different ways.
    inline bool operator<(const GLLight& o) const
    {
        return type  < o.type    || (type == o.type &&
            (color.b < o.color.b || (color.b == o.color.b &&
            (color.r < o.color.r || (color.r == o.color.r &&
            (color.g < o.color.g || (color.g == o.color.g &&
            (color.a < o.color.a || (color.a == o.color.a &&
            (radius  < o.radius))))))))));
    }

    // frustrating, but at least it's here. observe that pos is not sorted; that's intentional because calling code may sort it in different ways.
    inline bool operator==(const GLLight& o) const
    {
        return (type == o.type &&
            color.b == o.color.b &&
            color.r == o.color.r &&
            color.g == o.color.g &&
            color.a == o.color.a &&
            radius == o.radius);
    }
};

enum class GLLightSystemType : uint32_t
{
    disabled = 0,
    shadow_none,
    shadow_rays,
    shadow_drop,
};

/*!
 * \brief Information parameterizing the overall lighting system
 */
struct GLLightSystem
{
    GLLightSystemType system_type = GLLightSystemType::disabled;
    GLfloat shadow_strength = 0.05f;
    GLLightColor ambient = GLLightColor(0, 0, 0);
    uint32_t padding_2 = 0;

    inline operator bool() const
    {
        return system_type != GLLightSystemType::disabled;
    }
};

struct GLPictureLightInfo;

struct GLPictureLightInfoPtr
{
private:
    GLPictureLightInfo* info = nullptr;

public:
    operator bool() const
    {
        return info;
    }

    inline GLPictureLightInfo& operator*() const { return *info; }
    inline GLPictureLightInfo* operator->() const { return info; }

    inline GLPictureLightInfoPtr() = default;
    inline GLPictureLightInfoPtr(const GLPictureLightInfoPtr& o)
    {
        *this = o;
    }

    inline GLPictureLightInfoPtr(GLPictureLightInfoPtr&& o)
    {
        *this = o;
    }

    inline GLPictureLightInfoPtr& operator=(const GLPictureLightInfoPtr& o);
    inline GLPictureLightInfoPtr& operator=(GLPictureLightInfoPtr&& o);

    inline void init();

    inline ~GLPictureLightInfoPtr();
};

struct GLPictureLightInfo
{
    GLLight light;
    GLPictureLightInfoPtr next;
};

inline GLPictureLightInfoPtr& GLPictureLightInfoPtr::operator=(const GLPictureLightInfoPtr& o)
{
    if(info)
    {
        delete info;
        info = nullptr;
    }

    if(o.info)
    {
        init();
        info->light = o.info->light;
        info->next = o.info->next;
    }

    return *this;
}

inline GLPictureLightInfoPtr& GLPictureLightInfoPtr::operator=(GLPictureLightInfoPtr&& o)
{
    if(info)
    {
        delete info;
        info = nullptr;
    }

    if(o.info)
    {
        info = o.info;
        o.info = nullptr;
    }

    return *this;
}

inline void GLPictureLightInfoPtr::init()
{
    if(!info)
        info = new GLPictureLightInfo;
}

inline GLPictureLightInfoPtr::~GLPictureLightInfoPtr()
{
    if(info)
    {
        delete info;
        info = nullptr;
    }
}


#endif // #ifndef GL_LIGHT_INFO_H
