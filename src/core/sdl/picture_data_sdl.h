/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef PICTURE_DATA_SDL_H
#define PICTURE_DATA_SDL_H

#include <stdint.h>

typedef unsigned int    GLenum;
typedef int             GLint;
typedef unsigned int    GLuint;

struct SDL_Texture;

/*!
 * \brief Platform specific picture data. Fields should not be used directly
 */
struct StdPictureData
{
// Compatible backend is only can use these internals
    friend class RenderSDL;

    //! Texture instance pointer for SDL Render
    SDL_Texture *texture = nullptr;
    //! texture ID for OpenGL and other render engines
    GLint        texture_id = 0;

    //! Texture format at OpenGL-renderer
    GLenum      format = 0;
    //! Number of colors
    GLint       nOfColors = 0;

    //! Enable the software simulation of the bitmask renderer to support some graphical tricks
    bool        bitmask = false;

    uint8_t     *mask_back = nullptr;
    int         mask_back_pitch = 0;
    int         mask_back_w = 0;
    int         mask_back_h = 0;

    uint8_t     *mask_front = nullptr;
    int         mask_front_pitch = 0;
    int         mask_front_w = 0;
    int         mask_front_h = 0;

    uint8_t     *mask_render_buffer = nullptr;
    uint8_t     mask_render_buffer_pitch = 0;
    uint8_t     mask_render_buffer_w = 0;
    uint8_t     mask_render_buffer_h = 0;
    SDL_Texture *mask_render = nullptr;

    //! Cached color modifier
    uint8_t     modColor[4] = {255,255,255,255};

// Public API

    inline bool hasTexture()
    {
        return texture != nullptr || bitmask;
    }

    inline void clear()
    {
        texture = nullptr;
        bitmask = false;

        mask_back = nullptr;
        mask_back_pitch = 0;
        mask_back_w = 0;
        mask_back_h = 0;

        mask_front = nullptr;
        mask_front_pitch = 0;
        mask_front_w = 0;
        mask_front_h = 0;

        mask_render_buffer = nullptr;
        mask_render_buffer_pitch = 0;
        mask_render_buffer_w = 0;
        mask_render_buffer_h = 0;
    }
};

#endif // PICTURE_DATA_SDL_H
