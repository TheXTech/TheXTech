/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "sdl_proxy/sdl_stdinc.h"

#include "screen.h"
#include "globals.h" // SingleCoop
#include "config.h"

#include "core/render.h" // XRender::TargetX

RangeArr<Screen_t, 0, c_screenCount - 1> Screens;
Screen_t* l_screen = &Screens[0];

RangeArr<qScreen_t, 0, c_vScreenCount> qScreenLoc;
RangeArr<vScreen_t, 0, c_vScreenCount> vScreen;

static std::array<uint8_t, maxNetplayPlayers + 1> s_screenIdxByPlayer;


int vScreen_t::TargetX() const
{
    return Screens[screen_ref].TargetX() + ScreenLeft;
}

int vScreen_t::TargetY() const
{
    return Screens[screen_ref].TargetY() + ScreenTop;
}

int vScreen_t::CameraAddX() const
{
#ifdef PGE_MIN_PORT
    return (int)(std::ceil(X / 2 - 0.5)) * 2;
#else
    return std::ceil(X - 0.5);
#endif
}

int vScreen_t::CameraAddY() const
{
#ifdef PGE_MIN_PORT
    return (int)(std::ceil(Y / 2 - 0.5)) * 2;
#else
    return std::ceil(Y - 0.5);
#endif
}

Screen_t& Screen_t::canonical_screen()
{
    if(is_canonical())
        return *this;

    return Screens[m_CanonicalScreen];
}

const Screen_t& Screen_t::canonical_screen() const
{
    if(is_canonical())
        return *this;

    return Screens[m_CanonicalScreen];
}

Screen_t& Screen_t::visible_screen()
{
    if(Visible || m_VisibleScreen >= c_screenCount)
        return *this;

    return Screens[m_VisibleScreen];
}

const Screen_t& Screen_t::visible_screen() const
{
    if(Visible || m_VisibleScreen >= c_screenCount)
        return *this;

    return Screens[m_VisibleScreen];
}

void Screen_t::set_canonical_screen(uint8_t index)
{
    m_CanonicalScreen = index;

    if(m_CanonicalScreen != 0)
    {
        Screen_t *mainScreen = &Screens[0];
        Screen_t &canScreen = canonical_screen();

        if(this >= mainScreen && this <= &Screens[c_screenCount - 1])
            canScreen.m_VisibleScreen = static_cast<uint8_t>(this - mainScreen);
        else
            canScreen.m_VisibleScreen = 0;

        canScreen.m_CanonicalScreen = 0;
        canScreen.players = players;
        canScreen.two_screen_pref = two_screen_pref;
        canScreen.four_screen_pref = four_screen_pref;
    }
}

int Screen_t::active_begin() const
{
    if(Type == ScreenTypes::SingleCoop && SingleCoop == 2)
        return 1;

    return 0;
}

int Screen_t::active_end() const
{
    if(!is_active())
        return 0;

    if(Type == ScreenTypes::SingleCoop && SingleCoop == 2)
        return 2;

    if(Type == ScreenTypes::TopBottom || Type == ScreenTypes::LeftRight)
        return 2;

    if(Type == ScreenTypes::Dynamic && vScreen(2).Visible)
        return 2;

    if(Type == ScreenTypes::Quad)
        return player_count <= maxLocalPlayers ? player_count : maxLocalPlayers;

    return 1;
}

int Screen_t::TargetX() const
{
    return XRender::TargetW / 2 - W / 2;
}

int Screen_t::TargetY() const
{
    return XRender::TargetH / 2 - H / 2;
}

void InitScreens()
{
    // assign vScreens to screens
    for(int s = 0; s < c_screenCount; s++)
    {
        for(int v = 0; v < maxLocalPlayers; v++)
        {
            Screens[s].vScreen_refs[v] = s * maxLocalPlayers + v + 1;
            Screens[s].vScreen(v + 1).screen_ref = s;
        }
    }

    // set canonical screens
    for(int s = 0; s < maxNetplayClients; s++)
    {
        Screens[s].set_canonical_screen(maxNetplayClients + s);
        Screens[maxNetplayClients + s].Visible = false;
    }

    // reset screens' players
    for(int s = 0; s < maxNetplayClients; s++)
    {
        Screens[s].players = {};
        Screens[s].player_count = 0;
    }

    UpdateScreenPlayers();
}

void UpdateScreenPlayers()
{
    // update canonical screens
    for(int s = 0; s < maxNetplayClients; s++)
    {
        Screens[s].canonical_screen().players = Screens[s].players;
        Screens[s].canonical_screen().player_count = Screens[s].player_count;
    }

    // assign players to vScreens
    for(int s = 0; s < c_screenCount; s++)
    {
        for(int v = 0; v < maxLocalPlayers; v++)
        {
            Screens[s].vScreen(v + 1).player = Screens[s].players[v];
        }
    }

    // update player screen indexes
    s_screenIdxByPlayer = {};
    for(int s = 0; s < maxNetplayClients; s++)
    {
        for(int p = 0; p < Screens[s].player_count; p++)
        {
            if(Screens[s].players[p] <= maxNetplayPlayers)
                s_screenIdxByPlayer[Screens[s].players[p]] = s;
        }
    }
}

void Screens_AssignPlayer(int player, Screen_t& screen)
{
    if(screen.player_count >= maxLocalPlayers)
        return;

    screen.players[screen.player_count] = player;
    screen.player_count++;

    UpdateScreenPlayers();
}

void Screens_DropPlayer(int player)
{
    // drop player itself
    for(int s = 0; s < maxNetplayClients; s++)
    {
        for(int i = 0; i < Screens[s].player_count; i++)
        {
            if(Screens[s].players[i] == player)
            {
                // shift other players left
                for(int j = i; j + 1 < Screens[s].player_count; j++)
                    Screens[s].players[j] = Screens[s].players[j + 1];

                // reduce player count
                Screens[s].player_count--;
            }
        }
    }

    // shift all player indices down
    for(int s = 0; s < maxNetplayClients; s++)
    {
        for(int i = 0; i < Screens[s].player_count; i++)
        {
            if(Screens[s].players[i] > player)
                Screens[s].players[i]--;
        }
    }
}

// finds the visible Screen that contains a specific player
int ScreenIdxByPlayer(int player)
{
    if(player > maxNetplayPlayers)
        return 0;

    return s_screenIdxByPlayer[player];
}

// finds the canonical Screen that contains a specific player
Screen_t& ScreenByPlayer_canonical(int player)
{
    Screen_t& visible_screen = ScreenByPlayer(player);

    return visible_screen.canonical_screen();
}

// finds the visible vScreen that contains a specific player
int vScreenIdxByPlayer(int player)
{
    if(player < 1 || player > maxLocalPlayers)
        return 0;

    Screen_t& screen = ScreenByPlayer(player);

    bool is_splitscreen = (screen.Type == 1 || screen.Type == 4 || (screen.Type == 5 && screen.vScreen(2).Visible) || screen.Type == 6 || screen.Type == ScreenTypes::Quad);

    if(is_splitscreen)
    {
        for(int i = 0; i < screen.player_count; i++)
        {
            if(player == screen.players[i])
                return screen.vScreen_refs[i];
        }
    }

    return screen.vScreen_refs[0];
}

// finds the canonical vScreen that contains a specific player
int vScreenIdxByPlayer_canonical(int player)
{
    if(player < 1 || player > maxLocalPlayers)
        return 0;

    Screen_t& screen = ScreenByPlayer_canonical(player);

    bool is_splitscreen = (screen.Type == 1 || screen.Type == 4 || (screen.Type == 5 && screen.vScreen(2).Visible) || screen.Type == 6 || screen.Type == ScreenTypes::Quad);

    if(is_splitscreen)
    {
        for(int i = 0; i < screen.player_count; i++)
        {
            if(player == screen.players[i])
                return screen.vScreen_refs[i];
        }
    }

    return screen.vScreen_refs[0];
}
