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

static XMessage::Message msg_from_frame_no(XMessage::Type type, uint32_t frame_no)
{
    XMessage::Message ret;
    ret.type = type;
    ret.screen = (uint8_t)(frame_no >> 16);
    ret.player = (uint8_t)(frame_no >> 8);
    ret.message = (uint8_t)(frame_no >> 0);

    return ret;
}

static int frame_no_from_message(XMessage::Message message)
{
    return (int)(message.screen << 16) | (int)(message.player << 8) | (int)(message.message << 0);
}


namespace XMessage
{

static int s_client_thread(void* _client)
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

XMessage::Message NetworkClient::ParseMessage(const uint8_t* message)
{
    XMessage::Message got;
    got.type = (XMessage::Type)message[0];
    got.screen = message[1];
    got.player = message[2];
    got.message = message[3];

    return got;
}

NetworkClient::~NetworkClient()
{
    Disconnect(true);

    if(thread)
    {
        shutdown = true;
        SDL_WaitThread(thread, nullptr);
        thread = nullptr;
    }

    if(sdlnet_inited)
    {
        SDLNet_FreeSocketSet(socket_set);
        socket_set = nullptr;

        SDLNet_Quit();
        sdlnet_inited = false;
    }
}

void NetworkClient::EnsureThread()
{
    if(!sdlnet_inited)
    {
        SDLNet_Init();
        sdlnet_inited = true;
        socket_set = SDLNet_AllocSocketSet(3);
    }

    if(!thread)
    {
        thread = SDL_CreateThread(s_client_thread, "network thread", this);
        shutdown = false;
    }
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

    udp_socket = SDLNet_UDP_Open(0);
    if(!udp_socket || SDLNet_UDP_Bind(udp_socket, 0, &addr) != 0)
    {
        Disconnect();
        return;
    }

    // SDLNet_UDP_SetPacketLoss(udp_socket, 99);
    SDLNet_UDP_AddSocket(socket_set, udp_socket);

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

    tick = 0;
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

    // FIXME: think carefully about how to update status field
    // given that the primary status struct is owned by the game thread at this point

    if(shutdown)
        return;

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

void NetworkClient::SendAll()
{
    if(!tcp_data.socket)
        return;

    send_buffer.current_frame = tick;

    if(!message_buffer.empty())
    {
        send_buffer.messages.push_back(msg_from_frame_no(Type::frame_begin, tick));

        for(Message msg : message_buffer)
            send_buffer.messages.push_back(msg);

        message_buffer.clear();

        if(!ping_send_frame)
        {
            ping_send_frame = tick;
            ping_send_ms = SDL_GetTicks();
        }
    }

    bool fast_forward = (receive_buffer.available_frame > tick + 10);
    if(!send_buffer.messages.empty() || !fast_forward)
        SendData();

    SDL_AtomicSet(&message_buffer_state, REQUEST_PENDING);
}

void NetworkClient::SendData()
{
    if(send_buffer.current_frame < 0)
        return;

    // remove any ACKed messages from send buffer
    while(!send_buffer.messages.empty())
    {
        Message front = send_buffer.messages.front();
        if(front.type == Type::frame_begin && frame_no_from_message(front) > acked_frame)
            break;

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
    send_buffer.messages.push_back(msg_from_frame_no(Type::frame_end, send_buffer.current_frame));

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
        // send_buffer.udp_transmission.clear();
        send_buffer.messages.clear();
        for(uint8_t c : to_send)
            send_buffer.tcp_transmit_in_progress.push_back(c);
    }

    // FIXME: the above should only get called once, and then the below should get called multiple times (with UDP retransmission moved below)

    // update any TCP transmission in progress
    if(!send_buffer.tcp_transmit_in_progress.empty())
    {
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
            if(receive_buffer.tcp_frame_index > receive_buffer.available_frame)
            {
                receive_buffer.available_frame = receive_buffer.tcp_frame_index;

                for(Message msg : receive_buffer.tcp_frame_in_progress)
                {
                    receive_buffer.messages.push_back(msg);
                    // pLogDebug("TCP message get: %d %d %d %d", msg.type, msg.screen, msg.player, msg.message);
                }

                receive_buffer.tcp_frame_in_progress.clear();
            }

            if(got.type == XMessage::Type::frame_begin)
            {
                receive_buffer.tcp_frame_index = frame_no;
                receive_buffer.tcp_frame_in_progress.push_back(got);
            }
            else if(got.type == XMessage::Type::frame_end)
            {
                if(frame_no > receive_buffer.available_frame)
                    receive_buffer.available_frame = frame_no;

                receive_buffer.tcp_frame_index = -1;
            }

            break;

        default:
            if(receive_buffer.tcp_frame_index > receive_buffer.available_frame)
                receive_buffer.tcp_frame_in_progress.push_back(got);
        }
    }

    // receive UDP data
    while(SDLNet_UDP_Recv(udp_socket, udp_packet))
    {
        udp_packet_recd++;

        int udp_frame_index = -1;

        // std::string got;
        // for(int i = 0; i < udp_packet->len; i++)
        // {
        //     got += std::to_string(udp_packet->data[i]);
        //     got += ' ';
        // }

        // pLogDebug("UDP packet get: %s", got.c_str());

        for(int i = 0; i + 4 <= udp_packet->len; i += 4)
        {
            Message got = ParseMessage(&udp_packet->data[i]);

            switch(got.type)
            {
            // can recognize acks here soon
            case(XMessage::Type::transmit_start):
                acked_frame = frame_no_from_message(got);
                if(acked_frame > tick)
                {
                    // prematurely received packet
                    goto skip_packet;
                }

                if(acked_frame >= ping_send_frame && ping_send_frame != 0)
                {
                    latency_ms = SDL_GetTicks() - ping_send_ms;
                    latency_frames = tick - ping_send_frame;
                    ping_send_frame = 0;
                    pLogDebug("UDP latency: %u ms, %d ticks", latency_ms, latency_frames);
                }
                break;
            case(XMessage::Type::frame_end):
                udp_frame_index = frame_no_from_message(got);
                if(udp_frame_index > receive_buffer.available_frame)
                    receive_buffer.available_frame = udp_frame_index;
                break;
            case(XMessage::Type::frame_begin):
                udp_frame_index = frame_no_from_message(got);
            // fallthrough
            default:
                if(udp_frame_index > receive_buffer.available_frame)
                {
                    receive_buffer.messages.push_back(got);
                    // pLogDebug("UDP message get: %d %d %d %d", got.type, got.screen, got.player, got.message);
                }
                else
                {
                    // pLogDebug("UDP stale message get: %d %d %d %d [%d %d]", got.type, got.screen, got.player, got.message, udp_frame_index, receive_buffer.available_frame);
                }
                break;
            }
        }
skip_packet:
        // skip to next loop step
        (void)0;
    }
}

bool NetworkClient::WaitAndFill()
{
    // use the receive buffer to fill a frame if possible
    if(receive_buffer.available_frame >= tick)
    {
        if(!receive_buffer.messages.empty())
        {
            Message front = receive_buffer.messages.front();
            if(front.type == Type::frame_begin && frame_no_from_message(front) == tick)
            {
                receive_buffer.messages.pop_front();

                while(!receive_buffer.messages.empty() && receive_buffer.messages.front().type != Type::frame_begin)
                {
                    // pLogDebug("Message push: %d %d %d %d %d", tick, receive_buffer.messages.front().type, receive_buffer.messages.front().screen, receive_buffer.messages.front().player, receive_buffer.messages.front().message);
                    message_buffer.push_back(receive_buffer.messages.front());
                    receive_buffer.messages.pop_front();
                }
            }
        }

        // pLogDebug("UDP packets sent %d recv %d", udp_packet_sent, udp_packet_recd);

        tick++;
        return true;
    }

    return false;
}

void NetworkClient::client_loop()
{
    // currently, sleep 2ms here
    bool fast_forward = (status.client_state > CLIENT_LOBBY) && (receive_buffer.available_frame > tick + 10);
    SDLNet_CheckSockets(socket_set, (fast_forward) ? 0 : 2);

    // hang up on error
    if(tcp_control.err || tcp_data.err)
    {
        Disconnect();
        if(SDL_AtomicGet(&status_req_state) > REQUEST_IDLE)
        {
            status = ClientStatus();
            SDL_AtomicSet(&status_req_state, REQUEST_COMPLETED);
        }

        if(SDL_AtomicGet(&message_buffer_state) == REQUEST_PENDING)
            SDL_AtomicSet(&message_buffer_state, REQUEST_COMPLETED);
    }

    // section to handle connection status updates
    if(SDL_AtomicGet(&status_req_state) == REQUEST_SUBMIT)
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
            SDL_AtomicSet(&status_req_state, REQUEST_COMPLETED);
            return;
        }

        if(status_req.client_state != status.client_state)
        {
            // always shift through lobby (in the future...)
            if(status.client_state != CLIENT_LOBBY)
            {
                // request leave room, mark as pending
                return;
            }

            if(status_req.client_state == CLIENT_GUEST)
            {
                auto room_key = status_req.room_info.room_key;
                std::array<uint8_t, 5> to_send =
                {
                    HEADER_JOIN_ROOM,
                    uint8_t(room_key >> 24), uint8_t(room_key >> 16), uint8_t(room_key >> 8), uint8_t(room_key >> 0),
                };

                SDLNet_TCP_Send(tcp_control.socket, to_send.data(), to_send.size());
            }
            else if(status_req.client_state == CLIENT_HOST)
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

            SDL_AtomicSet(&status_req_state, REQUEST_PENDING);
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

                SDL_AtomicSet(&status_req_state, REQUEST_PENDING);
                return;
            }
        }

        // all done with the request
        SDL_AtomicSet(&status_req_state, REQUEST_COMPLETED);
        return;

        // // if currently in a room, try to leave it
        // if(status.client_state != CLIENT_LOBBY)
        // {
        //     // ...
        //     SDL_AtomicSet(&status_req_state, REQUEST_PENDING);
        //     return;
        // }

        // // try to join a room as a guest
        // if(status_req.client_state == CLIENT_GUEST)
        // {
        //     // try to leave the room
        //     // ...
        //     SDL_AtomicSet(&status_req_state, REQUEST_PENDING);
        // }
    }

    // section to send pending messages
    if(SDL_AtomicGet(&message_buffer_state) == REQUEST_SUBMIT)
        SendAll();

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
            if(SDL_AtomicGet(&status_req_state) == REQUEST_PENDING && got_room_key == status_req.room_info.room_key)
            {
                status.room_info.room_key = got_room_key;
                status.room_info.engine_hash = engine_hash;
                status.room_info.asset_hash = asset_hash;
                status.room_info.content_hash = content_hash;

                SDL_AtomicSet(&status_req_state, REQUEST_COMPLETED);
            }
        }
        else if(tcp_control.buffer[0] == HEADER_ROOM_KEY)
        {
            if(!tcp_control.FillBufferTo_NB(16))
                return;

            const auto& buffer = tcp_control.buffer;

            status.room_info.room_key = ((uint32_t)buffer[1] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 8) | ((uint32_t)buffer[4] << 0);

            status.client_index = buffer[5];

            status.rand_seed = ((uint32_t)buffer[6] << 16) | ((uint32_t)buffer[7] << 8) | ((uint32_t)buffer[8] << 0);

            fast_forward_to = ((int)buffer[9] << 16) | ((int)buffer[10] << 8) | ((int)buffer[11] << 0);

            session_key = ((uint32_t)buffer[12] << 24) | ((uint32_t)buffer[13] << 16) | ((uint32_t)buffer[14] << 8) | ((uint32_t)buffer[15] << 0);

            tcp_control.ShiftBuffer(16);

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

            pLogInfo("NetPlay: joined room %s", DisplayRoom(status.room_info.room_key).room_name);

            status.client_state = status_req.client_state;

            if(SDL_AtomicGet(&status_req_state) == REQUEST_PENDING)
            {
                // FIXME: maybe do SUBMIT if more actions need to be taken?
                SDL_AtomicSet(&status_req_state, REQUEST_COMPLETED);
            }
        }
        else if(tcp_control.buffer[0] == HEADER_ACK)
        {
            if(!tcp_control.FillBufferTo_NB(4))
                return;

            acked_frame = ((int)tcp_control.buffer[1] << 16) | ((int)tcp_control.buffer[2] << 8) | ((int)tcp_control.buffer[3] << 0);

            if(acked_frame >= ping_send_frame && ping_send_frame != 0)
            {
                latency_ms = SDL_GetTicks() - ping_send_ms;
                latency_frames = tick - ping_send_frame;
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

            // fixme: consider how to handle
            fast_forward_to = ((int)tcp_control.buffer[1] << 16) | ((int)tcp_control.buffer[2] << 8) | ((int)tcp_control.buffer[3] << 0);

            tcp_control.ShiftBuffer(4);
        }
    }

    // handle incoming data messages from TCP data channel
    if(status.client_state > CLIENT_LOBBY)
    {
        // if(!fast_forward)
        //     SendData();

        ReceiveData();

        if(SDL_AtomicGet(&message_buffer_state) == REQUEST_PENDING && WaitAndFill())
            SDL_AtomicSet(&message_buffer_state, REQUEST_COMPLETED);
    }
}

} // namespace XMessage
