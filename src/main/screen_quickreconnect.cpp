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

#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "controls.h"
#include "globals.h"
#include "graphics.h"
#include "game_main.h"

#include "main/menu_main.h"
#include "main/game_strings.h"

#include "main/screen_quickreconnect.h"

namespace QuickReconnectScreen
{

bool g_active;
int g_toast_duration[maxLocalPlayers] = {0};

void Deactivate()
{
    g_active = false;

    for(int i = 0; i < maxLocalPlayers; ++i)
        g_toast_duration[i] = 0;
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
        else if(g_toast_duration[i])
        {
            g_toast_duration[i] --;
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
                g_toast_duration[i] = MAX_TOAST_DURATION;
        }
    }

    if(!has_missing && !has_toast)
        g_active = false;
}

} // namespace QuickReconnectScreen
