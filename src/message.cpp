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

#include <deque>

#include "controls.h"
#include "message.h"
#include "globals.h"


namespace XMessage
{

static std::deque<Message> s_message_vector;

void Tick()
{
    // eventually sync state with other clients here
}

void PushMessage(Message message)
{
    s_message_vector.push_back(message);
}

Message PopMessage()
{
    Message ret;
    if(s_message_vector.empty())
        return ret;

    ret = s_message_vector.front();
    s_message_vector.pop_front();
    return ret;
}

void PushControls(int l_player_i, const Controls_t& controls)
{
    Controls_t& last_controls = Controls::g_RawControls[l_player_i];

    Message m;
    m.screen = l_screen - &Screens[0];
    m.player = l_player_i;

    for(uint8_t i = 0; i < Controls::PlayerControls::n_buttons; i++)
    {
        bool new_pressed = Controls::PlayerControls::GetButton(controls, i);
        bool old_pressed = Controls::PlayerControls::GetButton(last_controls, i);

        if(new_pressed && !old_pressed)
        {
            m.type = Type::press;
            m.message = i;
            PushMessage(m);
        }
        else if(old_pressed && !new_pressed)
        {
            m.type = Type::release;
            m.message = i;
            PushMessage(m);
        }
    }

    last_controls = controls;
}

} // namespace XMessage
