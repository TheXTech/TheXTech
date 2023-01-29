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
    friend class RenderGL11;
    friend class RenderGLES;

    //! Texture instance pointer for SDL Render
    SDL_Texture *texture = nullptr;
    //! Mask texture instance pointer for SDL Render
    SDL_Texture *mask_texture = nullptr;

    //! texture ID for OpenGL and other render engines
    GLuint       texture_id = 0;
    //! mask texture ID for OpenGL and other render engines
    GLuint       mask_texture_id = 0;

    //! Texture format at OpenGL-renderer
    GLenum      format = 0;
    //! Number of colors
    GLint       nOfColors = 0;

    //! Cached color modifier
    uint8_t     modColor[4] = {255,255,255,255};

// Public API

    inline bool hasTexture()
    {
        return texture != nullptr || texture_id != 0;
    }

    inline void clear()
    {
        texture = nullptr;
    }
};

#endif // PICTURE_DATA_SDL_H
