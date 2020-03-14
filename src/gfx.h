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

#ifndef GFX_H
#define GFX_H

#include "range_arr.hpp"
#include "std_picture.h"
#include <vector>
#include <string>

class GFX_t
{
    std::vector<StdPicture*> m_loadedImages;
    void loadImage(StdPicture &img, std::string path);
    int m_loadErrors = 0;
public:
    GFX_t();
    bool load();
    void unLoad();

    StdPicture BMVs;
    StdPicture BMWin;
    RangeArr<StdPicture, 1, 3> Boot;
    RangeArr<StdPicture, 1, 5> CharacterName;
    StdPicture Chat;
    RangeArr<StdPicture, 0, 2> Container;
    RangeArr<StdPicture, 1, 3> ECursor;
    RangeArr<StdPicture, 0, 9> Font1;
    RangeArr<StdPicture, 1, 3> Font2;
    StdPicture Font2S;
    RangeArr<StdPicture, 1, 2> Heart;
    RangeArr<StdPicture, 0, 8> Interface;
    StdPicture LoadCoin;
    StdPicture Loader;
    RangeArr<StdPicture, 0, 3> MCursor;
    RangeArr<StdPicture, 1, 4> MenuGFX;
    RangeArr<StdPicture, 2, 2> Mount;
    RangeArr<StdPicture, 0, 7> nCursor;
    StdPicture TextBox;
    RangeArr<StdPicture, 1, 2> Tongue;
    StdPicture Warp;
    StdPicture YoshiWings;
};

#endif // GFX_H
