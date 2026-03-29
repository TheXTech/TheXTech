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

#ifndef THEXTECH_ENABLE_SDL_NET
#error "client.h cannot be included in a build without SDL_net support"
#endif

#ifndef XCLIENT_H
#define XCLIENT_H

#include <cstdint>
#include <cstddef>
#include <climits>
#include <deque>

#include <SDL_net.h>
#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_thread.h>

#include "message.h"
#include "client_methods.h"

namespace XMessage
{

static constexpr size_t network_client_buffer_size = 2048;

enum NetworkHeader
{
    // HEADER_CLIENT_JOIN = 1,
    // HEADER_CLIENT_LOSS = 2,
    // HEADER_TEXT_EVENT = 3,
    HEADER_FRAME_COMPLETE = 4,
    // HEADER_YOU_ARE = 5,
    // HEADER_RAND_SEED = 6,
    // HEADER_TIME_IS = 7,
    HEADER_LEFT_ROOM = 8,

    HEADER_ROOM_KEY = 9,
    HEADER_ROOM_INFO = 10,

    HEADER_CREATE_ROOM = 11,
    HEADER_JOIN_ROOM = 12,

    HEADER_DATA_CHANNEL = 13,
    HEADER_ACK = 14,

    HEADER_PUT_SESSION = 15,
    HEADER_GET_SESSION = 16,
};

// enum BufferState
// {
//     OWNER_GAME = 0,
//     OWNER_CLIENT = 1,
//     OWNER_CLIENT_RECV = 2,
// };

enum RequestState
{
    // game is owner of request fields
    REQUEST_COMPLETED = -1,
    REQUEST_IDLE = 0,
    // client thread is owner of request fields
    REQUEST_SUBMIT = 1,
    REQUEST_PENDING = 2,
};

struct TCPWrapper
{
    TCPsocket socket = nullptr;
    uint8_t buffer[network_client_buffer_size];
    size_t buffer_used = 0;
    bool err = false;

    // returns true if something has been read from the socket
    bool FillBuffer(bool nb = false);

    // returns true if something has been read from the socket, false if connection is broken
    bool FillBufferTo(size_t fill);

    // returns true if the TCP buffer has been filled to this point, false if not ready yet
    bool FillBufferTo_NB(size_t fill);

    // shift the buffer after reading n bytes
    void ShiftBuffer(size_t shift);
};

struct RecvBuffer
{
    std::deque<Message> messages;
    std::vector<Message> tcp_frame_in_progress;
    int tcp_frame_index = -1;

    int available_frame = -1;
};

struct SendBuffer
{
    std::deque<Message> messages;
    int current_frame = -1;

    std::vector<uint8_t> tcp_transmit_in_progress;
    size_t tcp_transmit_pos = 0;
};

struct NetworkClient
{
    SDL_Thread* thread = nullptr;
    SDL_sem* client_wakeup = nullptr;
    SDL_sem* game_wakeup = nullptr;
    bool shutdown = false;

    TCPWrapper tcp_control;
    TCPWrapper tcp_data;
    UDPsocket  udp_socket = nullptr;
    UDPpacket* udp_packet = nullptr;

    int udp_packet_sent = 0;
    int udp_packet_recd = 0;

    SDLNet_SocketSet socket_set = nullptr;
    int tick = 0;
    uint32_t session_key = 0;

    int fast_forward_to = INT_MAX;

    SDL_atomic_t status_req_state;
    ClientStatus status_req;
    ClientStatus status;

    RecvBuffer receive_buffer;
    SendBuffer send_buffer;

    SDL_atomic_t message_buffer_state;
    std::deque<Message> message_buffer;

    // ping info
    int ping_send_frame = -1;
    uint32_t ping_send_ms = 0;
    int acked_frame = 0;

    uint32_t latency_ms = 0;
    int latency_frames = 0;

    bool sdlnet_inited = false;

    void EnsureThread();

    void Connect(const char* host, int port);
    void Disconnect(bool shutdown = false);

    void Shutdown();

    XMessage::Message ParseMessage(const uint8_t* message);

    // misc in-game calls
    void LeaveRoom();

    // load messages into send buffer
    void SendAll();

    // transmit messages
    void SendData();

    // receive messages
    void ReceiveData();

    bool WaitAndFill();

    // misc lobby calls
    void JoinNewRoom(const RoomInfo& room_info);

    // this is the primary loop for the client thread
    void client_loop();
};

} // namespace XMessage

#endif // #ifndef XCLIENT_H
