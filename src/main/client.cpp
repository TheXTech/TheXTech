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

#include <array>
#include <vector>

#include <Logger/logger.h>
#include <SDL_net.h>

#include "message.h"
#include "main/client.h"

static constexpr bool max_debug = false;

namespace XMessage
{

int client_thread(void* _client)
{
    if(!_client)
        return 1;

    NetworkClient& client = *static_cast<NetworkClient*>(_client);

    while(!client.shutdown)
    {
        client.client_loop();
        // eventually, some way of sleeping
    }

    return 0;
}

bool TCPWrapper::FillBuffer(bool nb)
{
    if(!socket)
        return false;

    if(nb)
    {
        if(!SDLNet_SocketReady(socket))
            return false;
    }

    if(network_client_buffer_size <= buffer_used)
        return false;

    int got = SDLNet_TCP_Recv(socket, &buffer[buffer_used], network_client_buffer_size - buffer_used);

    if(got <= 0)
    {
        err = true;
        return false;
    }

    buffer_used += got;
    return true;
}

bool TCPWrapper::FillBufferTo(size_t fill)
{
    while(buffer_used < fill)
    {
        if(!socket || err)
            return false;

        FillBuffer();
    }

    return true;
}

bool TCPWrapper::FillBufferTo_NB(size_t fill)
{
    while(buffer_used < fill)
    {
        if(!socket || err)
            return false;

        if(!FillBuffer(true))
            return false;
    }

    return true;
}

void TCPWrapper::ShiftBuffer(size_t shift)
{
    size_t to_move = buffer_used - shift;
    SDL_memmove(&buffer[0], &buffer[shift], to_move);
    buffer_used = to_move;
}

int NetworkClient::frame_no_from_message(XMessage::Message message)
{
    return (int)(message.screen << 16) | (int)(message.player << 8) | (int)(message.message << 0);
}

XMessage::Message NetworkClient::ParseMessage(const uint8_t* message)
{
    XMessage::Message got;
    got.type = (XMessage::Type)message[0];
    got.screen = message[1];
    got.player = message[2];
    got.message = message[3];

    return got;
}

void NetworkClient::Shutdown()
{
    if(thread)
    {
        shutdown = true;
        SDL_WaitThread(thread, nullptr);
        thread = nullptr;
    }

    Disconnect(true);

    if(client_wakeup)
    {
        SDL_DestroySemaphore(client_wakeup);
        client_wakeup = nullptr;
    }

    if(game_wakeup)
    {
        SDL_DestroySemaphore(game_wakeup);
        game_wakeup = nullptr;
    }

    if(status_req_sync_lock)
    {
        SDL_DestroyMutex(status_req_sync_lock);
        status_req_sync_lock = nullptr;
    }

    if(status_sync_lock)
    {
        SDL_DestroyMutex(status_sync_lock);
        status_sync_lock = nullptr;
    }

    if(sdlnet_inited)
    {
        SDLNet_FreeSocketSet(socket_set);
        socket_set = nullptr;

        SDLNet_Quit();
        sdlnet_inited = false;
    }
}

void NetworkClient::Startup()
{
    if(thread)
        return;

    if(!sdlnet_inited)
    {
        SDLNet_Init();
        sdlnet_inited = true;
        socket_set = SDLNet_AllocSocketSet(3);
    }

    if(!client_wakeup)
        client_wakeup = SDL_CreateSemaphore(0);

    if(!game_wakeup)
        game_wakeup = SDL_CreateSemaphore(0);

    if(!status_req_sync_lock)
        status_req_sync_lock = SDL_CreateMutex();

    if(!status_sync_lock)
        status_sync_lock = SDL_CreateMutex();

    SDL_AtomicSet(&status_req_id, 0);
    SDL_AtomicSet(&status_req_completed, 0);
    SDL_AtomicSet(&status_alarm_id, 0);

    if(!thread)
    {
        shutdown = false;
        thread = SDL_CreateThread(client_thread, "network thread", this);
    }
}

void NetworkClient::pull_status_req()
{
    if(SDL_AtomicGet(&status_req_id) == status_req_id_seen)
        return;

    if(SDL_TryLockMutex(status_req_sync_lock) == SDL_MUTEX_TIMEDOUT)
        return;

    // don't interrupt a status request unless the new request is to disconnect
    if(!status_req_in_progress || status_req_sync.client_state == CLIENT_OFF)
    {
        try
        {
            status_req = status_req_sync;
            status_req_id_seen = SDL_AtomicGet(&status_req_id);
            status_req_in_progress = REQUEST_SUBMIT;
        }
        catch(...)
        {
            // ignore exception on string copy, make sure to unlock mutex
        }
    }

    SDL_UnlockMutex(status_req_sync_lock);
}

void NetworkClient::push_status()
{
    SDL_LockMutex(status_sync_lock);
    try
    {
        status_sync = status;
    }
    catch(...)
    {
        // ignore exception on string copy, make sure to unlock mutex
    }
    SDL_AtomicAdd(&status_alarm_id, 1);
    SDL_UnlockMutex(status_sync_lock);
}

void NetworkClient::push_completed_request()
{
    push_status();
    SDL_AtomicSet(&status_req_completed, status_req_id_seen);
    status_req_in_progress = 0;
}

MutexSent NetworkClient::get_session_access()
{
    // cached copy - don't even try if there isn't a request we're trying to handle
    if(status_req.client_state < CLIENT_MIN_ACTIVE_STATE)
        return MutexSent(nullptr);

    MutexSent ret(status_req_sync_lock);

    // don't return a valid mutex (and modify the session) if the client doesn't want active NetPlay right now
    if(status_req_sync.client_state < CLIENT_MIN_ACTIVE_STATE)
        ret.release();

    return ret;
}

void NetworkClient::Connect(const char* host, int port)
{
    Disconnect();

    IPaddress addr;
    if(SDLNet_ResolveHost(&addr, host, port) != 0)
        return;

    tcp_control.socket = SDLNet_TCP_OpenClient(&addr);
    if(!tcp_control.socket)
        return;

    SDLNet_TCP_AddSocket(socket_set, tcp_control.socket);

    tcp_data.socket = SDLNet_TCP_OpenClient(&addr);
    if(!tcp_data.socket)
    {
        Disconnect();
        return;
    }

    SDLNet_TCP_AddSocket(socket_set, tcp_data.socket);

#ifndef __WII__
    udp_socket = SDLNet_UDP_Open(0);
    if(!udp_socket || SDLNet_UDP_Bind(udp_socket, 0, &addr) != 0)
    {
        Disconnect();
        return;
    }

    // SDLNet_UDP_SetPacketLoss(udp_socket, 99);
    SDLNet_UDP_AddSocket(socket_set, udp_socket);
#endif

    udp_packet = SDLNet_AllocPacket(256);
    if(!udp_packet)
    {
        Disconnect();
        return;
    }

    udp_packet->channel = 0;
    udp_packet->address = addr;

    status = ClientStatus();
    status.client_state = CLIENT_LOBBY;
    status.server_address = host;
    status.server_port = port;

    push_status();
}

void NetworkClient::Disconnect(bool shutdown)
{
    if(tcp_control.socket)
    {
        SDLNet_TCP_DelSocket(socket_set, tcp_control.socket);
        SDLNet_TCP_Close(tcp_control.socket);
        tcp_control.socket = nullptr;
    }

    if(tcp_data.socket)
    {
        SDLNet_TCP_DelSocket(socket_set, tcp_data.socket);
        SDLNet_TCP_Close(tcp_data.socket);
        tcp_data.socket = nullptr;
    }

    if(udp_socket)
    {
        SDLNet_UDP_DelSocket(socket_set, udp_socket);
        SDLNet_UDP_Close(udp_socket);
        udp_socket = nullptr;
    }

    if(udp_packet)
    {
        SDLNet_FreePacket(udp_packet);
        udp_packet = nullptr;
    }

    tcp_control.buffer_used = 0;
    tcp_control.err = false;

    tcp_data.buffer_used = 0;
    tcp_data.err = false;

    session_key = 0;

    if(shutdown)
        return;

    status = ClientStatus();

    if(status_req_in_progress)
        push_completed_request();
    else
        push_status();

    // UpdateConfig();
    // UpdateInternalRes();
}

void NetworkClient::LeaveRoom()
{
    if(!tcp_control.socket || status.client_state == CLIENT_LOBBY)
        return;

    uint8_t to_send[4] = {0,0,0,0};

    SDLNet_TCP_Send(tcp_control.socket, to_send, 4);
}

void NetworkClient::SyncData()
{
    if(!tcp_data.socket || status.client_state < CLIENT_MIN_ACTIVE_STATE)
        return;

    auto session_access = get_session_access();
    if(!session_access)
        return;

    // checks if there was a new frame on the main thread's side
    bool new_frame = (g_session.current_frame > temp_state.current_frame);
    bool is_waiting = (g_session.current_frame > g_session.available_frame);

    temp_state.current_frame = g_session.current_frame;

    if(new_frame && !g_session.submit_buffer.empty())
    {
        // maybe use remote_frame here?? prevents killing some un-acked messages
        send_buffer.messages.push_back(msg_from_frame_no(Type::frame_begin, temp_state.current_frame));

        for(Message msg : g_session.submit_buffer)
        {
            send_buffer.messages.push_back(msg);
            if(max_debug)
                pLogDebug("Message send %d %d %d %d %d", temp_state.current_frame, (int)msg.type, msg.screen, msg.player, msg.message);
        }

        g_session.submit_buffer.clear();

        if(!ping_send_frame)
        {
            if(udp_packet_recd > 0 || temp_state.current_frame - acked_frame > 60)
            {
                ping_send_frame = temp_state.current_frame;
                ping_send_ms = SDL_GetTicks();
            }
        }
    }

    // remote_frame is explicitly specified, available_frame is implicit
    if(temp_state.available_frame > temp_state.remote_frame)
        temp_state.remote_frame = temp_state.available_frame;

    // sync values from temp_state to g_session
    g_session.available_frame = temp_state.available_frame;
    g_session.remote_frame = temp_state.remote_frame;

    // FIXME: can throw
    for(Message msg : temp_state.new_history)
        g_session.history.push_back(msg);

    session_access.release();

    // no longer waiting? wake up main thread!
    if(is_waiting && temp_state.available_frame >= temp_state.current_frame)
        SDL_SemPost(game_wakeup);

    temp_state.new_history.clear();

    bool fast_forward = (temp_state.remote_frame > temp_state.current_frame + 10);
    if(!send_buffer.messages.empty() || (new_frame && !fast_forward))
        SendData();
}

void NetworkClient::SendData()
{
    if(temp_state.current_frame < 0)
        return;

    // remove any ACKed messages from send buffer
    while(!send_buffer.messages.empty())
    {
        Message front = send_buffer.messages.front();
        if(front.type == Type::frame_begin && frame_no_from_message(front) > acked_frame)
            break;
        else if(max_debug && front.type == Type::frame_begin)
            pLogDebug("Killed acked message (%d <= %d)", frame_no_from_message(front), acked_frame);

        send_buffer.messages.pop_front();
    }

    // FIXME: something about having received things
    bool has_udp = (udp_socket != nullptr && udp_packet_recd != 0);

    // 1ms timer goes here
    // if there's nothing that hasn't been ACKed, put a 15ms timer here
    if(!has_udp && send_buffer.messages.empty())
        return;

    // serialize messages
    std::vector<uint8_t> to_send;
    to_send.reserve(4096);

    // temporary marker to let server know how far the client has ACKed
    send_buffer.messages.push_back(msg_from_frame_no(Type::frame_end, temp_state.current_frame));

    for(XMessage::Message m : send_buffer.messages)
    {
        to_send.push_back((uint8_t)m.type);
        to_send.push_back(m.screen);
        to_send.push_back(m.player);
        to_send.push_back(m.message);
    }

    // remove temporary marker
    send_buffer.messages.pop_back();

    size_t MTU_size = 250;

    // ideally, send with UDP
    if(has_udp && to_send.size() <= MTU_size)
    {
        udp_packet->len = to_send.size();
        SDL_memcpy(udp_packet->data, to_send.data(), to_send.size());
        udp_packet_sent++;
        SDLNet_UDP_Send(udp_socket, 0, udp_packet);
    }
    // fallback to TCP if UDP is unavailable or unfeasible
    else
    {
        // WARNING: at this point, we don't currently have a safe transition back to UDP.
        // Eventually, add a special header for the UDP packet following a TCP transmission.
        // send_buffer.udp_transmission.clear();
        send_buffer.messages.clear();
        for(uint8_t c : to_send)
            send_buffer.tcp_transmit_in_progress.push_back(c);
    }

    // FIXME: the above should only get called once, and then the below should get called multiple times (with UDP retransmission moved below)

    // update any TCP transmission in progress
    if(!send_buffer.tcp_transmit_in_progress.empty())
    {
        if(max_debug)
        {
            std::string out;
            for(int i = send_buffer.tcp_transmit_pos; i < (int)send_buffer.tcp_transmit_in_progress.size(); i++)
            {
                out += std::to_string((int)send_buffer.tcp_transmit_in_progress[i]);
                out += ' ';
            }

            pLogDebug("TCP transmit %s", out.c_str());
        }

        send_buffer.tcp_transmit_pos += SDLNet_TCP_Send(tcp_data.socket,
            send_buffer.tcp_transmit_in_progress.data() + send_buffer.tcp_transmit_pos,
            send_buffer.tcp_transmit_in_progress.size() - send_buffer.tcp_transmit_pos);

        if(send_buffer.tcp_transmit_pos >= send_buffer.tcp_transmit_in_progress.size())
        {
            send_buffer.tcp_transmit_in_progress.clear();
            send_buffer.tcp_transmit_pos = 0;
        }
    }
}

void NetworkClient::ReceiveData()
{
    while(tcp_data.FillBufferTo_NB(4))
    {
        Message got = ParseMessage(&tcp_data.buffer[0]);
        tcp_data.ShiftBuffer(4);

        switch(got.type)
        {
        case(XMessage::Type::frame_begin):
        case(XMessage::Type::frame_end):
            int frame_no;
            frame_no = frame_no_from_message(got);

            // previous TCP frame is finished!
            if(receive_buffer.tcp_frame_index > temp_state.available_frame)
            {
                for(Message msg : receive_buffer.tcp_frame_in_progress)
                {
                    temp_state.new_history.push_back(msg);
                    if(max_debug)
                        pLogDebug("TCP message get: %d %d %d %d", (int)msg.type, msg.screen, msg.player, msg.message);
                }

                temp_state.available_frame = receive_buffer.tcp_frame_index;
            }

            receive_buffer.tcp_frame_in_progress.clear();

            if(got.type == XMessage::Type::frame_begin)
            {
                receive_buffer.tcp_frame_index = frame_no;
            }
            else if(got.type == XMessage::Type::frame_end)
            {
                if(frame_no > temp_state.available_frame)
                    temp_state.available_frame = frame_no;

                receive_buffer.tcp_frame_index = -1;
                break;
            }

            // fallthrough
        default:
            if(receive_buffer.tcp_frame_index > temp_state.available_frame)
                receive_buffer.tcp_frame_in_progress.push_back(got);
        }
    }

    // receive UDP data
    while(SDLNet_UDP_Recv(udp_socket, udp_packet))
    {
        udp_packet_recd++;

        if(max_debug)
        {
            std::string got;
            for(int i = 0; i < udp_packet->len; i++)
            {
                got += std::to_string(udp_packet->data[i]);
                got += ' ';
            }

            pLogDebug("UDP packet get: %s", got.c_str());
        }

        int udp_frame_index = -1;

        for(int i = 0; i + 4 <= udp_packet->len; i += 4)
        {
            Message got = ParseMessage(&udp_packet->data[i]);

            switch(got.type)
            {
            // can recognize acks here soon
            case(XMessage::Type::transmit_start):
                acked_frame = frame_no_from_message(got);
                if(acked_frame > temp_state.available_frame + 1)
                {
                    // prematurely received packet
                    goto skip_packet;
                }

                if(acked_frame >= ping_send_frame && ping_send_frame != 0)
                {
                    latency_ms = SDL_GetTicks() - ping_send_ms;
                    latency_frames = temp_state.current_frame - ping_send_frame;
                    ping_send_frame = 0;
                    pLogDebug("UDP latency: %u ms, %d ticks", latency_ms, latency_frames);
                }
                break;
            case(XMessage::Type::frame_end):
                udp_frame_index = frame_no_from_message(got);
                if(udp_frame_index > temp_state.available_frame)
                    temp_state.available_frame = udp_frame_index;
                break;
            case(XMessage::Type::frame_begin):
                udp_frame_index = frame_no_from_message(got);
            // fallthrough
            default:
                if(udp_frame_index > temp_state.available_frame)
                {
                    temp_state.new_history.push_back(got);

                    if(max_debug)
                        pLogDebug("UDP message get: %d %d %d %d", (int)got.type, got.screen, got.player, got.message);
                }
                else
                {
                    // pLogDebug("UDP stale message get: %d %d %d %d [%d %d]", got.type, got.screen, got.player, got.message, udp_frame_index, temp_state.available_frame);
                }
                break;
            }
        }
skip_packet:
        // skip to next loop step
        (void)0;
    }
}

void NetworkClient::client_loop()
{
    if(!tcp_control.socket || !SDLNet_CheckSockets(socket_set, 0))
    {
        // currently, sleep 2ms here (waiting on messages from the main thread), then check again
        SDL_SemWaitTimeout(client_wakeup, 2);

        if(tcp_control.socket)
            SDLNet_CheckSockets(socket_set, 0);
    }

    // hang up on error
    if(tcp_control.err || tcp_data.err)
        Disconnect();

    pull_status_req();

    // section to handle connection status updates
    if(status_req_in_progress == REQUEST_SUBMIT)
    {
        // if we shouldn't be connected, disconnect!
        if(status_req.client_state == CLIENT_OFF)
        {
            Disconnect();
        }
        // otherwise, we should be connected.
        else if(status_req.server_address != status.server_address || !tcp_control.socket)
        {
            // this will update status if it succeeds
            Connect(status_req.server_address.c_str(), status_req.server_port);
        }

        // if not currently connected, ensure status is cleared and return early
        if(!tcp_control.socket)
        {
            status = ClientStatus();
            push_completed_request();
            return;
        }

        if(status_req.client_state != status.client_state)
        {
            // always shift through lobby (in the future...)
            if(status.client_state != CLIENT_LOBBY && status.client_state != CLIENT_SESSION_CONFIG && status.client_state != CLIENT_HOST_IDLE)
            {
                // request leave room, mark as pending
                return;
            }

            if(status.client_state == CLIENT_SESSION_CONFIG || status.client_state == CLIENT_HOST_IDLE)
            {
                if(status_req.client_state == CLIENT_GUEST && status.client_state == CLIENT_SESSION_CONFIG)
                {
                    std::array<uint8_t, 1> to_send =
                    {
                        HEADER_GET_SESSION,
                    };

                    SDLNet_TCP_Send(tcp_control.socket, to_send.data(), to_send.size());
                }
                else if(status_req.client_state == CLIENT_HOST && status.client_state == CLIENT_HOST_IDLE)
                {
                    MutexSent session_access = get_session_access();

                    if(session_access)
                    {
                        uint32_t session_size = 9 + g_session.save_data.size();
                        uint32_t current_frame = g_session.current_frame;
                        uint32_t history_size = g_session.history.size() * 4;

                        temp_state.remote_frame = current_frame;

                        // encode session here
                        std::array<uint8_t, 21> to_send_a =
                        {
                            HEADER_PUT_SESSION,
                            uint8_t(current_frame >> 16), uint8_t(current_frame >> 8), uint8_t(current_frame >> 0),
                            uint8_t(session_size >> 24), uint8_t(session_size >> 16), uint8_t(session_size >> 8), uint8_t(session_size >> 0),
                            uint8_t(history_size >> 24), uint8_t(history_size >> 16), uint8_t(history_size >> 8), uint8_t(history_size >> 0),
                            uint8_t(g_session.random_seed  >> 24), uint8_t(g_session.random_seed  >> 16), uint8_t(g_session.random_seed  >> 8), uint8_t(g_session.random_seed  >> 0),
                            g_session.init_char_select[0], g_session.init_char_select[1], g_session.init_char_select[2], g_session.init_char_select[3],
                            g_session.save_present
                        };

                        // FIXME: blocking while lock is held
                        SDLNet_TCP_Send(tcp_control.socket, to_send_a.data(), to_send_a.size());
                        SDLNet_TCP_Send(tcp_control.socket, g_session.save_data.data(), g_session.save_data.size());
                        SDLNet_TCP_Send(tcp_control.socket, g_session.history.data(), history_size);

                        pLogDebug("Activating hosting, sending session to server (frame %d, save size %d, history size %d)...", (int)current_frame, (int)g_session.save_data.size(), (int)(g_session.history.size() * 4));
                    }
                    else
                        Disconnect();
                }
            }
            else if(status_req.client_state == CLIENT_GUEST)
            {
                auto room_key = status_req.room_info.room_key;
                std::array<uint8_t, 5> to_send =
                {
                    HEADER_JOIN_ROOM,
                    uint8_t(room_key >> 24), uint8_t(room_key >> 16), uint8_t(room_key >> 8), uint8_t(room_key >> 0),
                };

                SDLNet_TCP_Send(tcp_control.socket, to_send.data(), to_send.size());
            }
            else if(status_req.client_state == CLIENT_HOST || status_req.client_state == CLIENT_HOST_IDLE)
            {
                const auto& room_info = status_req.room_info;

                std::array<uint8_t, 13> to_send =
                {
                    HEADER_CREATE_ROOM,
                    uint8_t(room_info.engine_hash  >> 24), uint8_t(room_info.engine_hash  >> 16), uint8_t(room_info.engine_hash  >> 8), uint8_t(room_info.engine_hash  >> 0),
                    uint8_t(room_info.asset_hash   >> 24), uint8_t(room_info.asset_hash   >> 16), uint8_t(room_info.asset_hash   >> 8), uint8_t(room_info.asset_hash   >> 0),
                    uint8_t(room_info.content_hash >> 24), uint8_t(room_info.content_hash >> 16), uint8_t(room_info.content_hash >> 8), uint8_t(room_info.content_hash >> 0),
                };

                SDLNet_TCP_Send(tcp_control.socket, to_send.data(), to_send.size());
            }

            status_req_in_progress = REQUEST_PENDING;
            return;
        }

        if(status_req.room_info.room_key != status.room_info.room_key)
        {
            if(status_req.client_state == CLIENT_LOBBY)
            {
                // send room info request
                auto room_key = status_req.room_info.room_key;
                std::array<uint8_t, 5> to_send =
                {
                    HEADER_ROOM_INFO,
                    uint8_t(room_key >> 24), uint8_t(room_key >> 16), uint8_t(room_key >> 8), uint8_t(room_key >> 0),
                };

                SDLNet_TCP_Send(tcp_control.socket, to_send.data(), to_send.size());

                status_req_in_progress = REQUEST_PENDING;
                return;
            }
        }

        // all done with the request
        push_completed_request();
        return;
    }

    // section to send pending messages, and update the main thread if needed
    SyncData();

    // section to handle incoming TCP control messages
    if(tcp_control.FillBufferTo_NB(1))
    {
        if(tcp_control.buffer[0] == HEADER_ROOM_INFO)
        {
            if(!tcp_control.FillBufferTo_NB(17))
                return;

            const auto& buffer = tcp_control.buffer;

            uint32_t got_room_key = ((uint32_t)buffer[1] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 8) | ((uint32_t)buffer[4] << 0);
            uint32_t engine_hash = ((uint32_t)buffer[5] << 24) | ((uint32_t)buffer[6] << 16) | ((uint32_t)buffer[7] << 8) | ((uint32_t)buffer[8] << 0);
            uint32_t asset_hash = ((uint32_t)buffer[9] << 24) | ((uint32_t)buffer[10] << 16) | ((uint32_t)buffer[11] << 8) | ((uint32_t)buffer[12] << 0);
            uint32_t content_hash = ((uint32_t)buffer[13] << 24) | ((uint32_t)buffer[14] << 16) | ((uint32_t)buffer[15] << 8) | ((uint32_t)buffer[16] << 0);

            tcp_control.ShiftBuffer(17);

            // ignore unexpected (stale?) rooom key responses
            if(status_req_in_progress == REQUEST_PENDING && got_room_key == status_req.room_info.room_key)
            {
                status.room_info.room_key = got_room_key;
                status.room_info.engine_hash = engine_hash;
                status.room_info.asset_hash = asset_hash;
                status.room_info.content_hash = content_hash;

                push_completed_request();
            }
        }
        else if(tcp_control.buffer[0] == HEADER_ROOM_KEY)
        {
            if(!tcp_control.FillBufferTo_NB(10))
                return;

            const auto& buffer = tcp_control.buffer;

            status.room_info.room_key = ((uint32_t)buffer[1] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 8) | ((uint32_t)buffer[4] << 0);

            status.client_index = buffer[5];

            session_key = ((uint32_t)buffer[6] << 24) | ((uint32_t)buffer[7] << 16) | ((uint32_t)buffer[8] << 8) | ((uint32_t)buffer[9] << 0);

            tcp_control.ShiftBuffer(10);

            if(status_req.room_info.room_key && status.room_info.room_key != status_req.room_info.room_key)
            {
                pLogWarning("Server placed client in unexpected room");
                Disconnect();
            }

            // register data TCP channel (would actually be ideal to CONNECT it here)
            {
                std::array<uint8_t, 5> to_send =
                {
                    HEADER_DATA_CHANNEL,
                    uint8_t(session_key >> 24), uint8_t(session_key >> 16), uint8_t(session_key >> 8), uint8_t(session_key >> 0),
                };

                SDLNet_TCP_Send(tcp_data.socket, to_send.data(), to_send.size());
            }

            // register data UDP channel
            if(udp_socket)
            {
                udp_packet->data[0] = uint8_t(session_key >> 24);
                udp_packet->data[1] = uint8_t(session_key >> 16);
                udp_packet->data[2] = uint8_t(session_key >> 8);
                udp_packet->data[3] = uint8_t(session_key >> 0);

                udp_packet->len = 4;

                for(int i = 0; i < 4; i++)
                {
                    SDLNet_UDP_Send(udp_socket, 0, udp_packet);
                    SDL_Delay(1);
                }
            }

            udp_packet_sent = 0;
            udp_packet_recd = 0;

            // reset send and receive buffers
            receive_buffer = RecvBuffer();
            send_buffer = SendBuffer();

            // copying previously-existing logic from receive_buffer reinitialization, but these should probably happen elsewhere
            temp_state.remote_frame = 0;
            temp_state.current_frame = 0;
            temp_state.available_frame = -1;
            temp_state.new_history.clear();

            pLogInfo("NetPlay: joining room %s", DisplayRoom(status.room_info.room_key).room_name);

            if(status_req.client_state == CLIENT_HOST || status_req.client_state == CLIENT_HOST_IDLE)
                status.client_state = CLIENT_HOST_IDLE;
            else
                status.client_state = CLIENT_SESSION_CONFIG;

            if(status_req.client_state == CLIENT_HOST_IDLE)
            {
                status_req_in_progress = REQUEST_COMPLETED;
                push_completed_request();
            }
            else if(status_req_in_progress == REQUEST_PENDING)
                status_req_in_progress = REQUEST_SUBMIT;
        }
        else if(tcp_control.buffer[0] == HEADER_GET_SESSION)
        {
            if(!tcp_control.FillBufferTo_NB(1))
                return;

            tcp_control.ShiftBuffer(1);

            if(status_req_in_progress == REQUEST_PENDING && (status.client_state == CLIENT_SESSION_CONFIG || status.client_state == CLIENT_HOST_IDLE))
            {
                temp_state.available_frame = temp_state.remote_frame;

                status.client_state = CLIENT_HOST;
                pLogDebug("Hosting begun. The random seed is %d.", (int)g_session.random_seed);
                push_completed_request();
            }
            else
                Disconnect();
        }
        else if(tcp_control.buffer[0] == HEADER_PUT_SESSION)
        {
            if(!tcp_control.FillBufferTo_NB(8))
                return;

            temp_state.remote_frame = ((int)tcp_control.buffer[1] << 16) | ((int)tcp_control.buffer[2] << 8) | ((int)tcp_control.buffer[3] << 0);

            uint32_t session_size = ((uint32_t)tcp_control.buffer[4] << 24) | ((uint32_t)tcp_control.buffer[5] << 16) | ((uint32_t)tcp_control.buffer[6] << 8) | ((uint32_t)tcp_control.buffer[7] << 0);

            tcp_control.ShiftBuffer(8);

            // decode session here
            if(session_size < 9 || session_size > 2048000)
                Disconnect();

            // FIXME: not NB, should be checking for cancel
            if(!tcp_control.FillBufferTo(9))
                Disconnect();

            if(status_req_in_progress == REQUEST_PENDING && status.client_state == CLIENT_SESSION_CONFIG)
            {
                auto session_access = get_session_access();
                if(session_access)
                {
                    g_session.random_seed = ((uint32_t)tcp_control.buffer[0] << 24) | ((uint32_t)tcp_control.buffer[1] << 16) | ((uint32_t)tcp_control.buffer[2] << 8) | ((uint32_t)tcp_control.buffer[3] << 0);
                    g_session.init_char_select[0] = tcp_control.buffer[4];
                    g_session.init_char_select[1] = tcp_control.buffer[5];
                    g_session.init_char_select[2] = tcp_control.buffer[6];
                    g_session.init_char_select[3] = tcp_control.buffer[7];
                    g_session.save_present = tcp_control.buffer[8];

                    session_size -= 9;

                    // FIXME: may throw
                    g_session.save_data.resize(session_size);
                }
                else
                {
                    session_size = 0;
                    Disconnect();
                }

                session_access.release();

                tcp_control.ShiftBuffer(9);

                // yes, we're accessing this memory directly, because we know exactly where this gets called from
                char* dest = &g_session.save_data[0];
                while(session_size > 0)
                {
                    // FIXME: not NB, should be checking for cancel
                    size_t want_bytes = SDL_min(network_client_buffer_size, session_size);
                    if(!tcp_control.FillBufferTo(want_bytes))
                    {
                        Disconnect();
                        break;
                    }

                    // note: direct access
                    SDL_memcpy(dest, tcp_control.buffer, want_bytes);

                    dest += want_bytes;
                    session_size -= want_bytes;

                    tcp_control.ShiftBuffer(want_bytes);
                }

                if(session_size == 0)
                {
                    status.client_state = CLIENT_GUEST;
                    pLogDebug("The random seed is %d", (int)g_session.random_seed);
                    push_completed_request();
                }
            }
            else
                Disconnect();
        }
        else if(tcp_control.buffer[0] == HEADER_ACK)
        {
            if(!tcp_control.FillBufferTo_NB(4))
                return;

            acked_frame = ((int)tcp_control.buffer[1] << 16) | ((int)tcp_control.buffer[2] << 8) | ((int)tcp_control.buffer[3] << 0);

            if(max_debug)
                pLogDebug("TCP ack %d", acked_frame);

            if(acked_frame >= ping_send_frame && ping_send_frame != 0)
            {
                latency_ms = SDL_GetTicks() - ping_send_ms;
                latency_frames = temp_state.current_frame - ping_send_frame;
                ping_send_frame = 0;
                pLogDebug("TCP latency: %u ms, %d ticks", latency_ms, latency_frames);
            }

            tcp_control.ShiftBuffer(4);
        }
        // notice of current frame (for fast-forward)
        else if(tcp_control.buffer[0] == HEADER_FRAME_COMPLETE)
        {
            if(!tcp_control.FillBufferTo_NB(4))
                return;

            temp_state.remote_frame = ((int)tcp_control.buffer[1] << 16) | ((int)tcp_control.buffer[2] << 8) | ((int)tcp_control.buffer[3] << 0);

            tcp_control.ShiftBuffer(4);
        }
        // request to start the session
        else if(tcp_control.buffer[0] == HEADER_KNOCK_KNOCK)
        {
            tcp_control.ShiftBuffer(1);

            if(status.client_state != CLIENT_HOST_IDLE)
                Disconnect();
            else
            {
                pLogDebug("Received request to initiate hosting.");
                status.knock_knock = true;
                push_status();
            }
        }
    }

    // handle incoming data messages from TCP/UDP data channels
    if(status.client_state >= CLIENT_MIN_ACTIVE_STATE)
    {
        bool is_waiting = (temp_state.available_frame < temp_state.current_frame);
        ReceiveData();
        if(is_waiting && temp_state.available_frame >= temp_state.current_frame)
            SyncData();
    }
}

} // namespace XMessage
