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

#include <deque>

#include "controls.h"
#include "message.h"
#include "globals.h"
#include "sound.h"

#include "main/client.h"
#include "main/client_methods.h"
#include "main/screen_progress.h"

std::string g_netplayServer = "thextech.link";
std::string g_netplayNickname;

namespace XMessage
{

static NetworkClient s_network_client;

bool in_fast_forward = false;
uint32_t fast_forward_begin_ms = 0;
uint32_t fast_forward_begin_frame = 0;

void Connect(const char* host)
{
    s_network_client.EnsureThread();

    if(SDL_AtomicGet(&s_network_client.status_req_state) > REQUEST_IDLE)
        return;

    s_network_client.status_req = ClientStatus();
    s_network_client.status_req.client_state = CLIENT_LOBBY;
    s_network_client.status_req.server_address = (host) ? host : g_netplayServer;

    SDL_AtomicSet(&s_network_client.status_req_state, REQUEST_SUBMIT);
}

void Disconnect()
{
    if(SDL_AtomicGet(&s_network_client.status_req_state) > REQUEST_IDLE)
        return;

    s_network_client.status_req = ClientStatus();

    SDL_AtomicSet(&s_network_client.status_req_state, REQUEST_SUBMIT);
}

const ClientStatus* GetClientStatus()
{
    if(SDL_AtomicGet(&s_network_client.status_req_state) > REQUEST_IDLE)
        return nullptr;

    return &s_network_client.status;
}

bool CompleteRequest()
{
    if(SDL_AtomicGet(&s_network_client.status_req_state) == REQUEST_COMPLETED)
    {
        SDL_AtomicSet(&s_network_client.status_req_state, REQUEST_IDLE);
        return true;
    }

    return false;
}

Status GetStatus()
{
    if(CurrentRoom())
    {
        if(in_fast_forward)
            return Status::replay;
        else
            return Status::connected;
    }
    else
        return Status::local;
}

void ClientFrameSync(std::deque<Message>& buffer)
{
    bool start_fast_forward = (s_network_client.tick < s_network_client.fast_forward_to - 8);
    bool end_fast_forward = (s_network_client.tick >= s_network_client.fast_forward_to);
    if(!in_fast_forward && start_fast_forward)
    {
        in_fast_forward = true;
        fast_forward_begin_ms = SDL_GetTicks();
        fast_forward_begin_frame = s_network_client.tick;
    }
    else if(in_fast_forward)
    {
        IndicateProgress(fast_forward_begin_ms, num_t(s_network_client.tick - fast_forward_begin_frame) / (s_network_client.fast_forward_to - fast_forward_begin_frame), "Loading game history...");

        if(end_fast_forward)
        {
            in_fast_forward = false;
            // start playing music when no longer fast forwarding
            UpdateMusicVolume();
        }
    }

    while(SDL_AtomicGet(&s_network_client.message_buffer_state) != REQUEST_IDLE)
    {
        // fixme: wait on some cond here
    }

    std::swap(s_network_client.message_buffer, buffer);

    SDL_AtomicSet(&s_network_client.message_buffer_state, REQUEST_SUBMIT);

    while(SDL_AtomicGet(&s_network_client.message_buffer_state) != REQUEST_COMPLETED)
    {
        // fixme: wait on some cond here
    }

    std::swap(s_network_client.message_buffer, buffer);

    SDL_AtomicSet(&s_network_client.message_buffer_state, REQUEST_IDLE);
}

bool RequestFillRoomInfo(uint32_t room_key)
{
    if(SDL_AtomicGet(&s_network_client.status_req_state) > REQUEST_IDLE || s_network_client.status_req.client_state != CLIENT_LOBBY)
        return false;

    s_network_client.status_req.room_info.room_key = room_key;

    SDL_AtomicSet(&s_network_client.status_req_state, REQUEST_SUBMIT);
    return true;
}

const RoomInfo* GetRoomInfo()
{
    if(SDL_AtomicGet(&s_network_client.status_req_state) > REQUEST_IDLE)
        return nullptr;

    return &s_network_client.status.room_info;
}


void JoinNewRoom(const RoomInfo& room_info)
{
    if(SDL_AtomicGet(&s_network_client.status_req_state) > REQUEST_IDLE || s_network_client.status.client_state != CLIENT_LOBBY)
        return;

    s_network_client.status_req.client_state = CLIENT_HOST;
    s_network_client.status_req.room_info = room_info;

    SDL_AtomicSet(&s_network_client.status_req_state, REQUEST_SUBMIT);
}

void JoinRoom(uint32_t room_key)
{
    if(SDL_AtomicGet(&s_network_client.status_req_state) > REQUEST_IDLE || s_network_client.status.client_state != CLIENT_LOBBY)
        return;

    s_network_client.status_req.client_state = CLIENT_GUEST;
    s_network_client.status_req.room_info.room_key = room_key;

    SDL_AtomicSet(&s_network_client.status_req_state, REQUEST_SUBMIT);
}

uint32_t CurrentRoom()
{
    if(SDL_AtomicGet(&s_network_client.status_req_state) > REQUEST_IDLE || s_network_client.status.client_state == CLIENT_LOBBY)
        return 0;

    return s_network_client.status.room_info.room_key;
}

void LeaveRoom()
{
    s_network_client.LeaveRoom();
}

uint32_t RoomFromString(const std::string& room)
{
    if(room.size() != 6 && !(room.size() == 7 && room[3] == '-'))
        return 0;

    uint32_t sum = 0;
    uint32_t shift = 30;
    for(size_t i = 0; i < room.size(); i++)
    {
        if(i == 3 && room.size() == 7)
            continue;

        shift -= 5;

        char c = room[i];
        if(c < 48 || (c > 57 && c < 65) || (c > 90 && c < 97) || c > 122)
            return 0;

        if(c >= 97)
            c -= 32;

        const uint8_t digit_codes[10] = {14, 8, 25, 26, 27, 18, 28, 29, 30, 31};

        if(c >= 65)
            sum += (uint32_t)(c - 65) << shift;
        else
            sum += (uint32_t)digit_codes[c - 48] << shift;
    }

    return sum;
}

RoomName DisplayRoom(uint32_t room_key)
{
    RoomName ret;
    ret.room_name[0] = '\0';

    if(room_key == 0 || room_key & (3 << 30))
        return ret;

    const char letters[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ346789";

    ret.room_name[3] = '-';
    ret.room_name[7] = '\0';
    uint32_t shift = 30;
    for(int i = 0; i < 6; i++)
    {
        shift -= 5;
        int index = (i >= 3) ? i + 1 : i;
        ret.room_name[index] = letters[(room_key >> shift) & 31];
    }

    return ret;
}

} // namespace XMessage
