/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef STD_PICTURE_H
#define STD_PICTURE_H

#include <vector>
#ifdef DEBUG_BUILD
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
    std::vector<char> raw;
    std::vector<char> rawMask;
    bool isMaskPng = false;
    SDL_Texture *texture = nullptr;
    GLenum format = 0;
    GLint  nOfColors = 0;
    PGEColor ColorUpper;
    PGEColor ColorLower;
};

#endif // STD_PICTURE_H
