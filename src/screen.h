/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "numeric_types.h"

//Public Type vScreen 'Screen controls
struct qScreen_t
{
    // previously their own arrays, vScreenX and vScreenY
    num_t X = 0_n;
    num_t Y = 0_n;

//    Left As Double
    int Left = 0;
//    Top As Double
    int Top = 0;
//    Width As Double
    int Width = 0;
//    Height As Double
    int Height = 0;
//End Type

//    NEW: location on screen when vScreens are smaller due to level size
    int ScreenTop = 0;
    int ScreenLeft = 0;
};

struct vScreen_t : public qScreen_t
{
    struct SmallScreenFeatures_t
    {
        int16_t offset_x = 0;
        int16_t offset_y = 0;
        int16_t offset_y_hold = 0;
        int8_t last_buttons_held = 0;
    };

    SmallScreenFeatures_t small_screen_features;

//    tempX As Double
    num_t tempX = 0_n;
//    TempY As Double
    num_t TempY = 0_n;
//    TempDelay As Integer
    int TempDelay = 0;
//    Visible As Boolean
    bool Visible = false;

    // NEW: which screen the vScreen belongs to.
    uint8_t screen_ref = 0;

    // NEW: which player is associated with the vScreen. IMPORTANT: unused when shared screen mode is active
    uint8_t player = 0;

    //! left x-coordinate for drawing on the currently active render target
    int TargetX() const;

    //! top y-coordinate for start drawing on the currently active render target
    int TargetY() const;

    //! x-offset to add for render calls (based on X, but may be rounded)
    int CameraAddX() const;

    //! Y-offset to add for render calls (based on Y, but may be rounded)
    int CameraAddY() const;
};

constexpr int maxNetplayClients = 8;
constexpr int maxNetplayPlayers = maxNetplayClients * maxLocalPlayers;
constexpr int c_screenCount = 2 * maxNetplayClients;
constexpr int c_vScreenCount = c_screenCount * maxLocalPlayers;
constexpr int c_vScreenCount_visible = maxNetplayClients * maxLocalPlayers;

//Public vScreen(0 To 2) As vScreen 'Sets up the players screens
extern RangeArr<vScreen_t, 0, c_vScreenCount> vScreen;

//Public vScreen(0 To 2) As vScreen 'Sets up the players screens
extern RangeArr<qScreen_t, 0, c_vScreenCount> qScreenLoc;

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
        Quad = 9,
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

// player-specified per-screen preferences for multiplayer mode
// (overridden by cheats SingleCoop and g_ClonedPlayerMode)
namespace MultiplayerPrefs
{
    enum MultiplayerPrefs
    {
        Dynamic = 0,
        Split = 1,     // means LeftRight in 2P mode
        Shared = 2,
        Max_4P = 2,
        TopBottom = 3, // 2P mode only
        Max_2P = 3,
    };
}

struct Screen_t
{
private:
    //! a reference to the canonical Screen for this screen (an 800x600 screen with the same players); 0 indicates that the screen itself is canonical
    uint8_t m_CanonicalScreen = 0;
    //! a reference to the visible Screen for a canonical screen
    uint8_t m_VisibleScreen = 0;

public:
    using localarr_t = std::array<uint8_t, maxLocalPlayers>;

    //! which vScreens belong to the screen
    localarr_t vScreen_refs;

    //! which players belong to the screen (no player may belong to multiple visible screens).
    localarr_t players;

    //! which characters have been requested
    localarr_t charSelect;

    //! how many active players in this screen (excluding cloned-player mode)
    int player_count = 0;

    //! whether this is being rendered by any client (not necessarily the local one)
    bool Visible = true;

    //! the logical width of the screen in pixels
    int W = 800;

    //! the logical height of the screen in pixels
    int H = 600;

    //! the currently active split mode for the screen
    int Type = ScreenTypes::SinglePlayer;

    //! the currently active dynamic split mode for the screen
    int DType = DScreenTypes::Inactive;

    //! the currently requested multiplayer modes for the screen
    int two_screen_pref = MultiplayerPrefs::Dynamic;
    int four_screen_pref = MultiplayerPrefs::Shared;

    inline bool is_active() const
    {
        return player_count != 0;
    }

    inline bool is_canonical() const
    {
        return m_CanonicalScreen == 0;
    }

    Screen_t& canonical_screen();
    const Screen_t& canonical_screen() const;

    Screen_t& visible_screen();
    const Screen_t& visible_screen() const;

    void set_canonical_screen(uint8_t index);

    // uses a 1-index to simplify conversion of legacy code
    inline vScreen_t& vScreen(size_t index) const
    {
        return ::vScreen[vScreen_refs[index - 1]];
    }

    //! First active vScreen index (0-indexed). Use to replace numScreens logic.
    int active_begin() const;

    //! Bound on active vScreen indexes (0-indexed). Use to replace numScreens logic.
    int active_end() const;

    //! left x-coordinate for drawing on the currently active render target
    int TargetX() const;

    //! top y-coordinate for start drawing on the currently active render target
    int TargetY() const;
};

//! a list of all screens (local and remote, visible and virtual)
extern RangeArr<Screen_t, 0, c_screenCount - 1> Screens;

void InitScreens();

// ensure that the vScreen players and canonical screen players are consistent with the primary screens' players
void UpdateScreenPlayers();

// assigns a player to a screen
void Screens_AssignPlayer(int player, Screen_t& screen);

// drop a player from the screens system, lowering the indices of all higher players (but does no other player drop logic)
void Screens_DropPlayer(int player);

// finds the visible Screen that contains a specific player
int ScreenIdxByPlayer(int player);

// finds the visible Screen that contains a specific player
inline Screen_t& ScreenByPlayer(int player)
{
    return Screens[ScreenIdxByPlayer(player)];
}

// finds the canonical Screen that contains a specific player
Screen_t& ScreenByPlayer_canonical(int player);

// finds the visible vScreen that contains a specific player
int vScreenIdxByPlayer(int player);

// finds the visible vScreen that contains a specific player
inline vScreen_t& vScreenByPlayer(int player)
{
    return vScreen[vScreenIdxByPlayer(player)];
}

// finds the canonical vScreen that contains a specific player
int vScreenIdxByPlayer_canonical(int player);

// finds the visible vScreen that contains a specific player
inline vScreen_t& vScreenByPlayer_canonical(int player)
{
    return vScreen[vScreenIdxByPlayer_canonical(player)];
}

//! the screen being rendered by the local client (or, try to make all uses of ScreenW / ScreenH occur in functions that get passed a screen?)
extern Screen_t* l_screen;

// define this macro at the top of a source file to force the file to access the screens in the modern way

#ifndef SCREEN_H_NO_HELPERS
//Public ScreenType As Integer 'The screen/view type
// extern int& ScreenType;
//Public DScreenType As Integer 'The dynamic screen setup
// extern int& DScreenType;

//Public ScreenType As Integer 'The screen/view type
// extern int& ScreenW;
//Public DScreenType As Integer 'The dynamic screen setup
// extern int& ScreenH;
#endif

#endif // #ifndef SCREEN_HHH
