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
#include "game_main.h"

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

    // prevent collision with HUD at normal resolutions
    const int start_Y = ScreenH >= 640 ? 8 : 80;

    int drawn = 0;

    std::string message;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        if(i >= numPlayers)
            continue;

        if(i >= (int)Controls::g_InputMethods.size())
            continue;

        const Controls::InputMethod* input_method = Controls::g_InputMethods[i];

        if(!input_method)
            continue;

        if(s_toast_duration[i])
        {
            int draw_Y = start_Y + 20 * drawn;

            const std::string& profile_name = (input_method->Profile ? input_method->Profile->Name : g_mainMenu.caseNone);

            message = fmt::format_ne(g_gameStrings.controlsPhrasePlayerConnected, i + 1, input_method->Name, profile_name);

            SuperPrintScreenCenter(message, 3, draw_Y);

            drawn++;
        }
    }
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

    if(has_missing && GamePaused != PauseCode::DropAdd)
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
