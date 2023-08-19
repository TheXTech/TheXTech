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

#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "controls.h"
#include "globals.h"
#include "graphics.h"

#include "main/menu_main.h"
#include "main/game_strings.h"

namespace QuickReconnectScreen
{

bool g_active;

static int s_toast_duration[maxLocalPlayers] = {0};

void Deactivate()
{
    g_active = false;

    for(int i = 0; i < maxLocalPlayers; ++i)
        s_toast_duration[i] = 0;
}

void Render()
{
    if(GameMenu || LevelEditor || GameOutro)
    {
        Deactivate();
        return;
    }

    const int draw_X = 20;
    const int draw_X_right = ScreenW - 20;
    const int press_button_Y = ScreenH - 40;
    const int last_player_Y = press_button_Y - 20;

    int long_drawn = 0;
    int left_drawn = 0;
    int right_drawn = 0;
    bool left_missing = false;
    bool right_missing = false;

    std::string message;

    for(int i = maxLocalPlayers - 1; i >= 0; i--)
    {
        if(i >= numPlayers)
            continue;

        if(i >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[i])
        {
            message = fmt::format_ne(g_gameStrings.controlsPhrasePlayerDisconnected, i + 1);

            // P2 is special, gets right-aligned
            if(i == 1)
            {
                int draw_Y = last_player_Y - 20 * (right_drawn + long_drawn);
                SuperPrintRightAlign(message, 3, draw_X_right, draw_Y);
                right_missing = true;
                right_drawn++;
            }
            else
            {
                int draw_Y = last_player_Y - 20 * (left_drawn + long_drawn);
                SuperPrint(message, 3, draw_X, draw_Y);
                left_missing = true;
                left_drawn++;
            }
        }
        else if(s_toast_duration[i])
        {
            int draw_Y = last_player_Y - 20 * (SDL_max(left_drawn, right_drawn) + long_drawn);
            const std::string& p = (Controls::g_InputMethods[i]->Profile ? Controls::g_InputMethods[i]->Profile->Name : g_mainMenu.caseNone);
            message = fmt::format_ne(g_gameStrings.controlsPhrasePlayerConnected, i + 1, Controls::g_InputMethods[i]->Name, p);

            // P2 is special, gets right-aligned
            if(i == 1)
            {
                SuperPrintRightAlign(message, 3, draw_X_right, draw_Y);
            }
            else
                SuperPrint(message, 3, draw_X, draw_Y);

            long_drawn++;
        }
    }

    if(left_missing)
        SuperPrint(g_gameStrings.connectPressAButton, 3, draw_X + 20, press_button_Y);

    if(right_missing)
        SuperPrintRightAlign(g_gameStrings.connectPressAButton, 3, draw_X_right - 20, press_button_Y);
}

void Logic()
{
    if(GameMenu || LevelEditor || GameOutro)
    {
        Deactivate();
        return;
    }

    bool has_missing = false;
    bool has_toast = false;
    bool was_missing[maxLocalPlayers] = {false};

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        if(i >= numPlayers)
            continue;

        if(i >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[i])
        {
            has_missing = true;
            was_missing[i] = true;
        }
        else if(s_toast_duration[i])
        {
            s_toast_duration[i] --;
            has_toast = true;
        }
    }

    if(has_missing)
    {
        Controls::PollInputMethod();

        // add toasts for new players
        for(int i = 0; i < maxLocalPlayers; i++)
        {
            if(was_missing[i] && i < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[i])
                s_toast_duration[i] = 66 * 3; // 3 seconds
        }
    }

    if(!has_missing && !has_toast)
        g_active = false;
}

} // namespace QuickReconnectScreen
