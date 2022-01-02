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
#ifndef STD_PICTURE_H
#define STD_PICTURE_H

#include <vector>
#ifdef DEBUG_BUILD
#   include <string>
#endif

typedef unsigned int    GLenum;
typedef int             GLint;
typedef unsigned int    GLuint;

/*!
 * \brief RGB pixel color
 */
struct PGEColor
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

struct SDL_Texture;

/**
 * @brief Handler of a graphical texture
 */
struct StdPicture
{
#ifdef DEBUG_BUILD
    //! Debug-only file path to the original picture
    std::string origPath;
#endif
    //! Was this texture initialized by graphical engine or not?
    bool inited = false;

    //! Width of texture
    int w = 0;
    //! Height of texture
    int h = 0;

    // Frame width and height (for animation sprite textures)
    //! Animation frame width
    int frame_w = 0;
    //1 Animation frame height
    int frame_h = 0;

    // Original size (if texture got scaled while loading)
    //! Original width
    int w_orig = 0;
    //! Original height
    int h_orig = 0;

    // Difference between original and initial size
    //! Width scale factor
    float w_scale = 1.0f;
    //! Height scale factor
    float h_scale = 1.0f;

    //! Is this a lazy-loaded texture?
    bool lazyLoaded = false;
    //! Original compressed data of the front image
    std::vector<char> raw;
    //! Original compressed data of the mask image (if presented)
    std::vector<char> rawMask;

    //! Was mask restored from the PNG at default graphics?
    bool isMaskPng = false;
    //! Texture instance pointer for SDL Render
    SDL_Texture *texture = nullptr;
    //! texture ID for OpenGL and other render engines
    GLint        texture_id = 0;

    //! Texture format at OpenGL-renderer
    GLenum format = 0;
    //! Number of colors
    GLint  nOfColors = 0;
    //! Left-top pixel color
    PGEColor ColorUpper;
    //! Left-bottom pixel color
    PGEColor ColorLower;
    //! Cached color modifier
    uint8_t modColor[4] = {255,255,255,255};
};

// This macro allows to get the original texture path when debug build is on,
// and safely return the blank string when the release build is
#ifdef DEBUG_BUILD
#   define StdPictureGetOrigPath(x) x.origPath
#else
#   define StdPictureGetOrigPath(x) std::string()
#endif


#endif // STD_PICTURE_H
