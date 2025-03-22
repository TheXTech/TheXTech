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

#ifndef THEXTECH_ENABLE_SDL_NET
#error "client.h cannot be included in a build without SDL_net support"
#endif

#ifndef XCLIENT_H
#define XCLIENT_H

#include <cstdint>
#include <cstddef>
#include <climits>
#include <SDL_net.h>

#include "message.h"
#include "client_methods.h"

namespace XMessage
{

static constexpr size_t network_client_buffer_size = 2048;

enum NetworkHeader
{
    HEADER_CLIENT_JOIN = 1,
    HEADER_CLIENT_LOSS = 2,
    HEADER_TEXT_EVENT = 3,
    HEADER_FRAME_COMPLETE = 4,
    HEADER_YOU_ARE = 5,
    HEADER_RAND_SEED = 6,
    HEADER_TIME_IS = 7,
    HEADER_LEFT_ROOM = 8,

    HEADER_ROOM_KEY = 9,
    HEADER_ROOM_INFO = 10,

    HEADER_CREATE_ROOM = 11,
    HEADER_JOIN_ROOM = 12,
};

struct NetworkClient
{
    TCPsocket socket;
    int tick = 0;
    int num_clients = 0;
    uint8_t buffer[network_client_buffer_size];
    size_t buffer_used = 0;
    int fast_forward_to = INT_MAX;
    uint32_t start_fast_forward = 0;

    uint32_t room_key = 0;
    uint32_t requested_join_room_key = 0;

    RoomInfo* queried_room_info = nullptr;

    bool sdlnet_inited = false;

    ~NetworkClient();

    void Connect(const char* host, int port);
    void Disconnect(bool shutdown = false);

    // returns true if something has been read from the socket
    bool FillBuffer();

    // returns true if something has been read from the socket, false if connection is broken
    bool FillBufferTo(size_t fill);

    // shift the buffer after reading n bytes
    void ShiftBuffer(size_t shift);

    bool ParseMessage(int client_no, const uint8_t* message, size_t length);

    // misc in-game calls
    void LeaveRoom();
    void SendAll();
    void WaitAndFill();

    // misc lobby calls
    void _FinishRequestFillRoomInfo();
    void RequestFillRoomInfo(RoomInfo& room_info);

    void _FinishJoinRoom();
    void JoinNewRoom(const RoomInfo& room_info);
    void JoinRoom(uint32_t room_key);
};

} // namespace XMessage

#endif // #ifndef XCLIENT_H
