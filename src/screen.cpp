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

#include "screen.h"

RangeArr<Screen_t, 0, 0> Screens;
Screen_t* l_screen = &Screens[0];

RangeArr<qScreen_t, 0, 2> qScreenLoc;
RangeArr<vScreen_t, 0, 2> vScreen;

// finds the visible Screen that contains a specific player
Screen_t& ScreenByPlayer(int player)
{
    (void)player; // FIXME: Remove this line once multiple screens will be supported
    return Screens[0];
}

// finds the visible vScreen that contains a specific player
vScreen_t& vScreenByPlayer(int player)
{
    if(player < 1 || player > maxLocalPlayers)
        return vScreen[0];

    return vScreen[player];
}

// temporary helpers while game is being converted to use Screen_t
int& ScreenType = Screens[0].Type;
int& DScreenType = Screens[0].DType;

int& ScreenW = Screens[0].W;
int& ScreenH = Screens[0].H;
