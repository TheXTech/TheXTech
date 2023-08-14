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

#ifndef SCREEN_HHH
#define SCREEN_HHH

#include <array>

#include "range_arr.hpp"
#include "global_constants.h"

//Public Type vScreen 'Screen controls
struct qScreen_t
{
    // previously their own arrays, vScreenX and vScreenY
    double X = 0.0;
    double Y = 0.0;

//    Left As Double
    double Left = 0.0;
//    Top As Double
    double Top = 0.0;
//    Width As Double
    double Width = 0.0;
//    Height As Double
    double Height = 0.0;
//End Type
};

struct vScreen_t : public qScreen_t
{
//    tempX As Double
    double tempX = 0.0;
//    TempY As Double
    double TempY = 0.0;
//    TempDelay As Integer
    int TempDelay = 0;
//    Visible As Boolean
    bool Visible = false;

    uint8_t Screen = 0;
};

//Public vScreen(0 To 2) As vScreen 'Sets up the players screens
extern RangeArr<vScreen_t, 0, 2> vScreen;

//Public vScreen(0 To 2) As vScreen 'Sets up the players screens
extern RangeArr<qScreen_t, 0, 2> qScreenLoc;

namespace ScreenTypes
{
    enum ScreenTypes
    {
        SinglePlayer = 0,
        TopBottom = 1,
        Average = 2,
        SharedScreen = 3,
        LeftRight = 4,
        Dynamic = 5,
        SingleCoop = 6,
        Credits = 7,
    };
}

namespace DScreenTypes
{
    enum DScreenTypes
    {
        Inactive = 0,
        LeftRight = 1,
        RightLeft = 2,
        BottomTop = 3,
        TopBottom = 4,
        Shared = 5,
        DiffSections = 6,
    };
}

struct Screen_t
{
    //! which vScreens belong to the screen
    std::array<uint8_t, maxLocalPlayers> vScreen_refs = {1, 2};

    // which players belong to the screen (at most one visible and one canonical screen may have a single player). zero-terminated.
    std::array<uint8_t, maxLocalPlayers> players = {1, 2};

    //! whether this is being rendered by any client (not necessarily the local one)
    bool Visible = true;

    //! a reference to the canonical Screen for this screen (an 800x600 screen with the same players), can be self
    uint8_t CanonicalScreen = 0;

    //! the logical width of the screen in pixels
    int W = 800;

    //! the logical height of the screen in pixels
    int H = 600;

    //! the currently active split mode for the screen
    int Type = ScreenTypes::SinglePlayer;

    //! the currently active dynamic split mode for the screen
    int DType = DScreenTypes::Inactive;

    // uses a 1-index to simplify conversion of legacy code
    inline vScreen_t& vScreen(size_t index) const
    {
        return ::vScreen[vScreen_refs[index - 1]];
    }
};

// finds the visible Screen that contains a specific player
Screen_t& ScreenByPlayer(int player);

// finds the canonical Screen that contains a specific player
Screen_t& ScreenByPlayer_canonical(int player);

// finds the visible vScreen that contains a specific player
vScreen_t& vScreenByPlayer(int player);

// finds the canonical vScreen that contains a specific player
vScreen_t& vScreenByPlayer_canonical(int player);

//! a list of all screens (local and remote, visible and virtual)
extern RangeArr<Screen_t, 0, 0> Screens;

//! the screen being rendered by the local client (or, try to make all uses of ScreenW / ScreenH occur in functions that get passed a screen?)
extern Screen_t* l_screen;

// define this macro at the top of a source file to force the file to access the screens in the modern way

#ifndef SCREEN_H_NO_HELPERS
//Public ScreenType As Integer 'The screen/view type
extern int& ScreenType;
//Public DScreenType As Integer 'The dynamic screen setup
extern int& DScreenType;

//Public ScreenType As Integer 'The screen/view type
extern int& ScreenW;
//Public DScreenType As Integer 'The dynamic screen setup
extern int& ScreenH;
#endif

#endif // #ifndef SCREEN_HHH
