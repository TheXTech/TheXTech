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

#include <deque>

#include "controls.h"
#include "message.h"
#include "globals.h"

#include "player.h"
#include "main/cheat_code.h"
#include "main/screen_pause.h"

#ifdef THEXTECH_ENABLE_SDL_NET
#   include "main/client_methods.h"
#endif

namespace XMessage
{

static std::deque<Message> s_message_vector;

static Controls_t s_last_controls[maxNetplayPlayers + 1];

void Handle(const Message& m)
{
    if(m.screen >= maxNetplayClients)
        return;

    Screen_t& screen = Screens[m.screen];

    if(m.type == Type::press || m.type == Type::release)
    {
        if(m.player >= maxLocalPlayers || m.message >= Controls::PlayerControls::n_buttons)
            return;

        auto& controls = s_last_controls[screen.players[m.player]];
        bool& button = Controls::PlayerControls::GetButton(controls, m.message);
        bool is_press = (m.type == Type::press);

        button = is_press;
    }
    else if(m.type == Type::char_swap)
    {
        if(m.player >= screen.player_count || m.message < 1 || m.message > numCharacters || !SwapCharAllowed())
            return;

        SwapCharacter(screen.players[m.player], m.message);

        if(LevelSelect)
            SetupPlayers();
    }
    else if(m.type == Type::add_player || m.type == Type::add_player_dead)
    {
        if(screen.player_count >= maxLocalPlayers || m.message < 1 || m.message > numCharacters)
            return;

        // after AddPlayer, numPlayers is always the new player
        AddPlayer(m.message, screen);

        // set the player to be dead if needed
        if(m.type == Type::add_player_dead)
        {
            Player[numPlayers].Dead = true;

            // initialize ghost logic for player
            int living = CheckNearestLiving(numPlayers);
            if(living)
            {
                Player[numPlayers].Effect2 = -living;
                Player[numPlayers].Location.X = Player[living].Location.X;
                Player[numPlayers].Location.Y = Player[living].Location.Y;
                Player[numPlayers].Section    = Player[living].Section;
            }
            else
                Player[numPlayers].Effect2 = 0;
        }
    }
    else if(m.type == Type::drop_player)
    {
        if(m.player >= screen.player_count)
            return;

        DropPlayer(screen.players[m.player]);
    }
    else if(m.type == Type::menu_action)
    {
        PauseScreen::g_pending_action = m.message;
    }
    else if(m.type == Type::shared_controls)
    {
        if(m.message == 0)
            SharedPause = true;
        else if(m.message == 1)
            SharedPause = false;
        else if(m.message == 2)
            SharedPauseLegacy = true;
        else if(m.message == 3)
            SharedPauseLegacy = false;
    }
    else if(m.type == Type::enter_code)
        run_cheat(m);
    else if(m.type == Type::screen_w)
    {
        screen.W = m.player * 256 + m.message;
    }
    else if(m.type == Type::screen_h)
    {
        screen.H = m.player * 256 + m.message;
    }
}

void Tick()
{
#ifdef THEXTECH_ENABLE_SDL_NET
    // sync state with other clients here
    if(CurrentRoom())
        ClientFrameSync();
#endif

    // update player controls based on message queue
    Message m;
    while((m = PopMessage()))
        Handle(m);

    int numPlayers_p = numPlayers;

    // fix a bug affecting main menu dead mode
    if(GameMenu || GameOutro)
        numPlayers_p = maxLocalPlayers;

    for(int A = 1; A <= numPlayers_p && A <= maxNetplayPlayers; A++)
        Player[A].Controls = s_last_controls[A];
}

void PushMessage_Direct(Message message)
{
    s_message_vector.push_back(message);
}

void PushMessage(Message message)
{
    message.screen = l_screen - &Screens[0];
    PushMessage_Direct(message);
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
            PushMessage_Direct(m);
        }
        else if(old_pressed && !new_pressed)
        {
            m.type = Type::release;
            m.message = i;
            PushMessage_Direct(m);
        }
    }

    last_controls = controls;
}

} // namespace XMessage
