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
