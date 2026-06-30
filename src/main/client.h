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
    std::vector<Message> tcp_frame_in_progress;
    int tcp_frame_index = -1;
};

struct SendBuffer
{
    std::deque<Message> messages;

    std::vector<uint8_t> tcp_transmit_in_progress;
    size_t tcp_transmit_pos = 0;
};

struct MutexSent final
{
    SDL_mutex* mutex = nullptr;

    MutexSent(const MutexSent&) = delete;
    MutexSent& operator=(const MutexSent&) = delete;
    MutexSent(MutexSent&& o)
    {
        release();
        mutex = o.mutex;
        o.mutex = nullptr;
    }

    MutexSent(SDL_mutex* _mutex)
    {
        mutex = _mutex;

        if(mutex)
            SDL_LockMutex(mutex);
    }

    ~MutexSent()
    {
        release();
    }

    void release()
    {
        if(mutex)
            SDL_UnlockMutex(mutex);

        mutex = nullptr;
    }

    operator bool() const
    {
        return this->mutex;
    }
};

int client_thread(void* _client);

struct NetworkClientState
{
    int current_frame = 0;
    int available_frame = -1;
    int remote_frame = -1;

    std::vector<Message> new_history;
};

struct NetworkClient
{
    friend int client_thread(void* _client);

    static int frame_no_from_message(XMessage::Message message);

    // synchronization state
public:
    SDL_Thread* thread = nullptr;
    SDL_sem* client_wakeup = nullptr;
    SDL_sem* game_wakeup = nullptr;
    bool shutdown = false;

    // both to be called from main thread
    void Startup();
    void Shutdown();

    // status requests -- sent by main thread, used by network thread
public:
    SDL_mutex*   status_req_sync_lock; // locked by main thread when altering status_req and by network thread when reading status_req (try_lock) or when altering gameplay state
    ClientStatus status_req_sync;      // copy of status_req to synchronize from main thread to network thread
    SDL_atomic_t status_req_id;        // incremented by main thread to indicate new status change request
    SDL_atomic_t status_req_completed; // set by network thread to indicate completed status changes
private:
    ClientStatus status_req;                     // network thread's copy of status_req
    int          status_req_id_seen = 0;         // set by network thread to indicate seen requests
    int          status_req_in_progress = false; // tracks whether the network thread is currently responding to a status change request

    void pull_status_req();

    // status change notifications -- sent by network thread, used by main thread
public:
    SDL_mutex*   status_sync_lock;     // try_locked by main thread when copying status, locked by network thread when altering status
    ClientStatus status_sync;          // copy of status to synchronize from network thread to main thread
    SDL_atomic_t status_alarm_id;      // incremented by network thread to indicate new status change
private:
    ClientStatus status;               // network thread's copy of status

    void push_status();
    void push_completed_request();

    MutexSent get_session_access();

private:
    TCPWrapper tcp_control;
    TCPWrapper tcp_data;
    UDPsocket  udp_socket = nullptr;
    UDPpacket* udp_packet = nullptr;

    int udp_packet_sent = 0;
    int udp_packet_recd = 0;

    SDLNet_SocketSet socket_set = nullptr;
    uint32_t session_key = 0;

    RecvBuffer receive_buffer;
    SendBuffer send_buffer;

private:
    // network thread's copy of critical game state, to avoid needing to synchronize prematurely.
    NetworkClientState temp_state;

    // ping info
    int ping_send_frame = -1;
    uint32_t ping_send_ms = 0;
    int acked_frame = 0;

    uint32_t latency_ms = 0;
    int latency_frames = 0;

    bool sdlnet_inited = false;

    void Connect(const char* host, int port);
    void Disconnect(bool shutdown = false);

    XMessage::Message ParseMessage(const uint8_t* message);

    // misc in-game calls
    void LeaveRoom();

    // load messages into send buffer, and store history into g_session
    void SyncData();

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
