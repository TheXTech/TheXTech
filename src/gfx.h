/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
    void loadImage(StdPicture &img, std::string path, bool fail_okay = false);
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
    StdPicture EIcons;

#ifdef __ANDROID__
    // Android specific pictures
    enum
    {
        BUTTON_START = 0,
        BUTTON_LEFT,
        BUTTON_LEFT_CHAR,
        BUTTON_RIGHT,
        BUTTON_RIGHT_CHAR,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_UPLEFT,
        BUTTON_UPRIGHT,
        BUTTON_DOWNLEFT,
        BUTTON_DOWNRIGHT,
        BUTTON_A,
        BUTTON_A_PS,
        BUTTON_A_BLANK,
        BUTTON_A_DO,
        BUTTON_A_ENTER,
        BUTTON_A_JUMP,
        BUTTON_B,
        BUTTON_B_PS,
        BUTTON_B_BLANK,
        BUTTON_B_JUMP,
        BUTTON_B_SPINJUMP,
        BUTTON_X,
        BUTTON_X_PS,
        BUTTON_X_BACK,
        BUTTON_X_BLANK,
        BUTTON_X_BOMB,
        BUTTON_X_BUMERANG,
        BUTTON_X_FIRE,
        BUTTON_X_HAMMER,
        BUTTON_X_RUN,
        BUTTON_X_SWORD,
        BUTTON_Y,
        BUTTON_Y_PS,
        BUTTON_Y_BLANK,
        BUTTON_Y_BOMB,
        BUTTON_Y_BUMERANG,
        BUTTON_Y_FIRE,
        BUTTON_Y_HAMMER,
        BUTTON_Y_RUN,
        BUTTON_Y_STATUE,
        BUTTON_Y_SWORD,
        BUTTON_DROP,
        BUTTON_HOLD_RUN_OFF,
        BUTTON_HOLD_RUN_ON,
        BUTTON_VIEW_TOGGLE_OFF,
        BUTTON_VIEW_TOGGLE_ON,
        BUTTON_ANALOG_BORDER,
        BUTTON_ANALOG_STICK,
        BUTTONS_END
    };
    StdPicture touch[BUTTONS_END];
#endif
};

#endif // GFX_H
