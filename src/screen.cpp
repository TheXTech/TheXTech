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

RangeArr<Screen_t, 0, c_screenCount - 1> Screens;
Screen_t* l_screen = &Screens[0];

RangeArr<qScreen_t, 0, c_vScreenCount> qScreenLoc;
RangeArr<vScreen_t, 0, c_vScreenCount> vScreen;

Screen_t& Screen_t::canonical_screen()
{
    if(is_canonical())
        return *this;

    return Screens[m_CanonicalScreen];
}

void Screen_t::set_canonical_screen(uint8_t index)
{
    m_CanonicalScreen = index;

    if(m_CanonicalScreen != 0)
    {
        canonical_screen().m_CanonicalScreen = 0;
        canonical_screen().players = players;
    }
}

void InitScreens()
{
    // assign players to main screen
    for(int p = 0; p < maxLocalPlayers; p++)
        Screens[0].players[p] = p + 1;

    // assign vScreens to screens
    for(int s = 0; s < c_screenCount; s++)
    {
        for(int v = 0; v < maxLocalPlayers; v++)
        {
            Screens[s].vScreen_refs[v] = s * maxLocalPlayers + v + 1;
            vScreen[s * maxLocalPlayers + v + 1].screen_ref = s;
        }
    }

    Screens[0].set_canonical_screen(1);
    Screens[1].Visible = false;
}

// finds the visible Screen that contains a specific player
Screen_t& ScreenByPlayer(int player)
{
    // FIXME: update this for netplay
    (void)player;
    return Screens[0];
}

// finds the canonical Screen that contains a specific player
Screen_t& ScreenByPlayer_canonical(int player)
{
    Screen_t& visible_screen = ScreenByPlayer(player);

    return visible_screen.canonical_screen();
}

// finds the visible vScreen that contains a specific player
vScreen_t& vScreenByPlayer(int player)
{
    if(player < 1 || player > maxLocalPlayers)
        return vScreen[0];

    // FIXME: update this for netplay
    return vScreen[player];
}

// finds the canonical vScreen that contains a specific player
vScreen_t& vScreenByPlayer_canonical(int player)
{
    if(player < 1 || player > maxLocalPlayers)
        return vScreen[0];

    // FIXME: update this for netplay
    return vScreen[maxLocalPlayers + player];
}

// temporary helpers while game is being converted to use Screen_t
int& ScreenType = Screens[0].Type;
int& DScreenType = Screens[0].DType;

int& ScreenW = Screens[0].W;
int& ScreenH = Screens[0].H;
