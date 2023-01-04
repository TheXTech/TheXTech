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

#include "../controls.h"
#include "../globals.h"
#include "../graphics.h"

#include "menu_main.h"

namespace QuickReconnectScreen
{

bool g_active;

static int s_toast_duration[maxLocalPlayers] = {0};

void Render()
{
    if(GameMenu || LevelEditor || GameOutro)
    {
        g_active = false;
        return;
    }

    const int draw_X = 20;
    const int press_button_Y = ScreenH - 40;
    const int last_player_Y = press_button_Y - 20;

    int drawn = 0;
    bool none_missing = true;

    char message[] = "P_ DISCONNECTED";

    for(int i = maxLocalPlayers - 1; i >= 0; i--)
    {
        if(i >= numPlayers)
            continue;

        message[1] = '1' + i;

        if(i >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[i])
        {
            int draw_Y = last_player_Y - 20 * drawn;
            message[3] = 'D';
            SuperPrint(message, 3, draw_X, draw_Y);
            drawn++;
            none_missing = false;
        }
        else if(s_toast_duration[i])
        {
            int draw_Y = last_player_Y - 20 * drawn;
            message[3] = '\0';
            std::string p = (Controls::g_InputMethods[i]->Profile ? Controls::g_InputMethods[i]->Profile->Name : "<NONE>");
            std::string&& s = message + g_mainMenu.controlsConnected + " " + Controls::g_InputMethods[i]->Name + ", " + g_mainMenu.wordProfile + ": " + p;
            SuperPrint(s, 3, draw_X, draw_Y);
            drawn++;
        }
    }

    if(!none_missing)
        SuperPrint(g_mainMenu.phrasePressAButton, 3, draw_X + 20, press_button_Y);
}

void Logic()
{
    if(GameMenu || LevelEditor || GameOutro)
    {
        g_active = false;
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
