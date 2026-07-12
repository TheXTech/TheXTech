/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef XMESSAGE_H
#define XMESSAGE_H

#include <cstdint>
#include <array>
#include <vector>
#include <string>

#include "global_constants.h"

struct Controls_t;


// The purpose of these methods is to track the set of player-initiated events that may affect gameplay
namespace XMessage
{

enum class Status : uint8_t
{
    local,
    replay,
    connected,
};

enum class Type : uint8_t
{
    empty,
    press,
    release,
    menu_action,
    char_swap,
    add_player,
    add_player_dead,
    drop_player,
    shared_controls,
    enter_code,
    screen_w,
    screen_h,
    multiplayer_prefs,
    camera_overscan_x,
    episode_option_change,
    compat_session_tweak_change,
    frame_begin = 32, // meta-message: the following messages belong to the named frame
#ifdef THEXTECH_ENABLE_SDL_NET
    // special server control messages
    add_client = 33,
    drop_client = 34,
    frame_end = 35, // meta-message: closes the previously named frame (only used by TCP)
    transmit_start = 36, // meta-message: acknowledges client messages up to named frame (only used by UDP)
#endif // #ifdef THEXTECH_ENABLE_SDL_NET
};

struct Message
{
    Type type = Type::empty;
    uint8_t screen;
    uint8_t player;
    uint8_t message;

    inline Message() : screen(0), player(0) {}
    inline Message(Type type, uint8_t player, uint8_t message) : type(type), screen(0), player(player), message(message) {}
    operator bool() const
    {
        return type != Type::empty;
    }
};

struct Session
{
    // initial state
    uint32_t random_seed = 0;
    std::array<uint8_t, maxLocalPlayers> init_char_select{};
#ifdef THEXTECH_ENABLE_SDL_NET
    uint8_t save_present = 0;
    std::string save_data;

    // current state
    int current_frame = 0;
    int available_frame = -1;
    int remote_frame = -1;
    std::vector<Message> history;
    size_t next_message = 0; // index into history

    // messages to submit to the network client
    std::vector<Message> submit_buffer;
#endif
};

extern Session g_session;

void InitSession();
void Tick();

void PushMessage_Direct(Message message);
void PushMessage(Message message);

void PushControls(int l_player_i, const Controls_t& controls);

#ifdef THEXTECH_ENABLE_SDL_NET

// defined in client_methods.cpp
Status GetStatus();

#else // #ifdef THEXTECH_ENABLE_SDL_NET

static inline Status GetStatus()
{
    return Status::local;
}

#endif // #ifdef THEXTECH_ENABLE_SDL_NET

} // namespace XMessage

#endif // #ifndef XMESSAGE_H
