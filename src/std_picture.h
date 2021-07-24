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

#ifndef STD_PICTURE_H
#define STD_PICTURE_H

#include <vector>
#ifdef DEBUG_BUILD
#include <string>
#endif

#ifdef __3DS__
#include <citro2d.h>
#include <string>
#endif

#ifdef VITA
#include <string>
#endif

typedef unsigned int    GLenum;
typedef int             GLint;
typedef unsigned int    GLuint;

struct PGEColor
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

struct SDL_Texture;
struct StdPicture
{
#ifdef DEBUG_BUILD
    std::string origPath;
#endif
    bool inited = false;
    // Width and height
    int w = 0;
    int h = 0;
    // Frame width and height (for animation sprite textures)
    int frame_w = 0;
    int frame_h = 0;
    // Original size (if texture got scaled while loading)
    int w_orig = 0;
    int h_orig = 0;
    // Difference between original and initial size
    float w_scale = 1.0f;
    float h_scale = 1.0f;

    bool lazyLoaded = false;

#if !defined(__3DS__) && (!defined(VITA) || defined(USE_SDL_VID))
    std::vector<char> raw;
    std::vector<char> rawMask;
    bool isMaskPng = false;
    SDL_Texture *texture = nullptr;
    GLenum format = 0;
    GLint  nOfColors = 0;
    PGEColor ColorUpper;
    PGEColor ColorLower;
#endif
#ifdef __3DS__
    std::string path = "";
    uint32_t lastDrawFrame = 0;
    C2D_SpriteSheet texture = nullptr;
    C2D_Image image;
    C2D_SpriteSheet texture2 = nullptr;
    C2D_Image image2;
    C2D_SpriteSheet texture3 = nullptr;
    C2D_Image image3;
#endif
#if defined(VITA) && !defined(USE_SDL_VID)
    std::string path = "";
    uint32_t lastDrawFrame = 0;
    std::vector<char> raw;
    GLuint texture = 0;
    GLenum format = 0;
    GLint  nOfColors = 0;
    PGEColor ColorUpper;
    PGEColor ColorLower;
#endif
};

// This macro allows to get the original texture path when debug build is on,
// and safely return the blank string when the release build is
#ifdef DEBUG_BUILD
#   define StdPictureGetOrigPath(x) x.origPath
#else
#   define StdPictureGetOrigPath(x) std::string()
#endif


#endif // STD_PICTURE_H
