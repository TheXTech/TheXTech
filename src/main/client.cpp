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

#include <Logger/logger.h>
#include <SDL_net.h>

#include "globals.h"
#include "player.h"
#include "graphics.h"
#include "config.h"
#include "change_res.h"
#include "sound.h"

#include "message.h"
#include "main/client.h"
#include "main/screen_progress.h"


namespace XMessage
{

NetworkClient::~NetworkClient()
{
    Disconnect(true);

    if(sdlnet_inited)
    {
        SDLNet_Quit();
        sdlnet_inited = false;
    }
}

void NetworkClient::Connect(const char* host, int port)
{
    Disconnect();

    if(!sdlnet_inited)
    {
        SDLNet_Init();
        sdlnet_inited = true;
    }

    IPaddress addr;
    if(SDLNet_ResolveHost(&addr, host, port) != 0)
        return;

    socket = SDLNet_TCP_OpenClient(&addr);
    if(!socket)
        return;

    tick = 0;
    buffer_used = 0;
    num_clients = 0;

    room_key = 0;
    requested_join_room_key = 0;
    queried_room_info = nullptr;
}

void NetworkClient::Disconnect(bool shutdown)
{
    if(!socket)
        return;

    SDLNet_TCP_Close(socket);
    socket = nullptr;
    room_key = 0;

    if(shutdown)
        return;

    UpdateConfig();
    UpdateInternalRes();
}

bool NetworkClient::FillBuffer()
{
    if(!socket)
        return false;

    // if(!SDLNet_SocketReady(socket))
    //     return false;

    if(network_client_buffer_size <= buffer_used)
        return false;

    int got = SDLNet_TCP_Recv(socket, &buffer[buffer_used], network_client_buffer_size - buffer_used);

    if(got <= 0)
    {
        Disconnect();
        return false;
    }

    buffer_used += got;
    return true;
}

bool NetworkClient::FillBufferTo(size_t fill)
{
    while(buffer_used < fill)
    {
        if(!socket)
            return false;

        FillBuffer();
    }

    return true;
}

void NetworkClient::ShiftBuffer(size_t shift)
{
    size_t to_move = buffer_used - shift;
    SDL_memmove(&buffer[0], &buffer[shift], to_move);
    buffer_used = to_move;
}

bool NetworkClient::ParseMessage(int client_no, const uint8_t* message, size_t length)
{
    UNUSED(client_no);

    if(length != 4)
        return false;

    XMessage::Message got;
    got.type = (XMessage::Type)message[0];
    got.screen = message[1];
    got.player = message[2];
    got.message = message[3];

    XMessage::PushMessage_Direct(got);
    return true;
}

void NetworkClient::LeaveRoom()
{
    if(!socket || !room_key)
        return;

    uint8_t to_send[4] = {0,0,0,0};

    SDLNet_TCP_Send(socket, to_send, 4);
}

void NetworkClient::SendAll()
{
    if(!socket)
        return;

    // send some information about the local client once sync is complete
    if(tick == fast_forward_to + 1)
    {
        UpdateInternalRes();
        XMessage::PushMessage({XMessage::Type::multiplayer_prefs, (uint8_t)g_config.two_screen_mode.m_value, (uint8_t)g_config.four_screen_mode.m_value});
    }

    // send everything
    std::vector<uint8_t> to_send;
    to_send.reserve(4096);

    uint8_t prelude_1 = (tick >> 16) & 0xff;
    uint8_t prelude_2 = (tick >>  8) & 0xff;
    uint8_t prelude_3 = (tick >>  0) & 0xff;
    uint8_t prelude_4 = 4;

    XMessage::Message m;
    while((m = XMessage::PopMessage()))
    {
        to_send.push_back(prelude_1);
        to_send.push_back(prelude_2);
        to_send.push_back(prelude_3);
        to_send.push_back(prelude_4);
        to_send.push_back((uint8_t)m.type);
        to_send.push_back(m.screen);
        to_send.push_back(m.player);
        to_send.push_back(m.message);
    }

    SDLNet_TCP_Send(socket, to_send.data(), to_send.size());
}

void NetworkClient::WaitAndFill()
{
    while(true)
    {
        if(!FillBufferTo(1))
            return;

        switch(buffer[0])
        {
        case(HEADER_CLIENT_JOIN):
            if(!FillBufferTo(2))
                return;

            pLogInfo("Added P%d on tick %d", buffer[1] + 1, tick);

            {
                Screen_t& screen = Screens[buffer[1]];
                screen.W = 800;
                screen.H = 600;
                screen.two_screen_pref = MultiplayerPrefs::Dynamic;
                screen.four_screen_pref = MultiplayerPrefs::Shared;
                screen.canonical_screen().two_screen_pref = screen.two_screen_pref;
                screen.canonical_screen().four_screen_pref = screen.four_screen_pref;

                // move last remaining player to new screen upon connection resumption
                if(screen.player_count == 0 && numPlayers == 1 && num_clients == 0)
                {
                    Screens_DropPlayer(1);
                    Screens_AssignPlayer(1, screen);
                    SwapCharacter(1, (buffer[1] % 5) + 1);
                }
                else if(Screens[buffer[1]].player_count == 0)
                    AddPlayer((buffer[1] % 5) + 1, screen);

                SetupScreens();
            }

            num_clients++;
            ShiftBuffer(2);
            break;

        case(HEADER_CLIENT_LOSS):
            if(!FillBufferTo(2))
                return;

            for(int p = Screens[buffer[1]].player_count - 1; p >= 0; p--)
            {
                if(numPlayers == 1)
                    break;

                DropPlayer(Screens[buffer[1]].players[p]);
            }

            num_clients--;
            ShiftBuffer(2);
            break;

        case(HEADER_TEXT_EVENT):
            if(!FillBufferTo(3))
                return;

            int client_no;
            client_no = buffer[1];
            int length;
            length = buffer[2];

            if(!FillBufferTo(3 + length))
                return;

            if(!ParseMessage(client_no, &buffer[3], length))
            {
                pLogWarning("Invalid message");
                Disconnect();
                return;
            }

            ShiftBuffer(3 + length);
            break;

        case(HEADER_FRAME_COMPLETE):
            if(!FillBufferTo(4))
                return;

            int frame_no;
            frame_no = ((int)buffer[1] << 16) | ((int)buffer[2] << 8) | ((int)buffer[3] << 0);

            ShiftBuffer(4);

            if(frame_no != tick)
            {
                pLogWarning("Got frame end for wrong frame (local %d, server %d)", tick, frame_no);
                Disconnect();
                return;
            }

            tick++;

            // start playing music when no longer fast forwarding
            if(tick == fast_forward_to)
                UpdateMusicVolume();
            else if(tick < fast_forward_to)
                IndicateProgress(start_fast_forward, (double)tick / fast_forward_to, "Loading game history...");

            return;

        case(HEADER_YOU_ARE):
            if(!FillBufferTo(2))
                return;

            l_screen = &Screens[buffer[1]];
            ShiftBuffer(2);

            break;

        case(HEADER_RAND_SEED):
            if(!FillBufferTo(4))
                return;

            int rand_seed;
            rand_seed = ((int)buffer[1] << 16) | ((int)buffer[2] << 8) | ((int)buffer[3] << 0);

            seedRandom(rand_seed);

            ShiftBuffer(4);

            break;

        case(HEADER_TIME_IS):
            if(!FillBufferTo(4))
                return;

            frame_no = ((int)buffer[1] << 16) | ((int)buffer[2] << 8) | ((int)buffer[3] << 0);

            ShiftBuffer(4);

            fast_forward_to = frame_no;
            start_fast_forward = SDL_GetTicks();
            break;

        case(HEADER_LEFT_ROOM):
            ShiftBuffer(1);

            room_key = 0;

            return;

        default:
            pLogWarning("Got unexpected header byte %d", (int)buffer[0]);
            Disconnect();
            return;
        }
    }
}

void NetworkClient::_FinishRequestFillRoomInfo()
{
    if(!queried_room_info)
        return;

    if(!FillBufferTo(1))
        return;

    if(buffer[0] != HEADER_ROOM_INFO)
    {
        Disconnect();
        return;
    }

    if(!FillBufferTo(17))
        return;

    uint32_t got_room_key = ((uint32_t)buffer[1] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 8) | ((uint32_t)buffer[4] << 0);

    if(got_room_key != queried_room_info->room_key && got_room_key != 0)
    {
        pLogWarning("Got room info for unexpected room");
        Disconnect();
    }

    uint32_t engine_hash = ((uint32_t)buffer[5] << 24) | ((uint32_t)buffer[6] << 16) | ((uint32_t)buffer[7] << 8) | ((uint32_t)buffer[8] << 0);
    uint32_t asset_hash = ((uint32_t)buffer[9] << 24) | ((uint32_t)buffer[10] << 16) | ((uint32_t)buffer[11] << 8) | ((uint32_t)buffer[12] << 0);
    uint32_t content_hash = ((uint32_t)buffer[13] << 24) | ((uint32_t)buffer[14] << 16) | ((uint32_t)buffer[15] << 8) | ((uint32_t)buffer[16] << 0);

    ShiftBuffer(17);

    queried_room_info->engine_hash = engine_hash;
    queried_room_info->asset_hash = asset_hash;
    queried_room_info->content_hash = content_hash;
    queried_room_info->room_key = got_room_key;
}

void NetworkClient::RequestFillRoomInfo(RoomInfo& room_info)
{
    if(!socket)
        return;

    std::array<uint8_t, 5> to_send =
    {
        HEADER_ROOM_INFO,
        uint8_t(room_info.room_key >> 24), uint8_t(room_info.room_key >> 16), uint8_t(room_info.room_key >> 8), uint8_t(room_info.room_key >> 0),
    };

    SDLNet_TCP_Send(socket, to_send.data(), to_send.size());
    queried_room_info = &room_info;

    _FinishRequestFillRoomInfo();
}

void NetworkClient::_FinishJoinRoom()
{
    if(!FillBufferTo(1))
        return;

    if(buffer[0] != HEADER_ROOM_KEY)
    {
        Disconnect();
        return;
    }

    if(!FillBufferTo(5))
        return;

    room_key = ((uint32_t)buffer[1] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 8) | ((uint32_t)buffer[4] << 0);

    ShiftBuffer(5);

    if(requested_join_room_key && room_key != requested_join_room_key)
    {
        pLogWarning("Server placed client in unexpected room");
        Disconnect();
    }

    pLogInfo("NetPlay: joined room %s", DisplayRoom(room_key).room_name);
}

void NetworkClient::JoinNewRoom(const RoomInfo& room_info)
{
    if(!socket)
        return;

    std::array<uint8_t, 13> to_send =
    {
        HEADER_CREATE_ROOM,
        uint8_t(room_info.engine_hash  >> 24), uint8_t(room_info.engine_hash  >> 16), uint8_t(room_info.engine_hash  >> 8), uint8_t(room_info.engine_hash  >> 0),
        uint8_t(room_info.asset_hash   >> 24), uint8_t(room_info.asset_hash   >> 16), uint8_t(room_info.asset_hash   >> 8), uint8_t(room_info.asset_hash   >> 0),
        uint8_t(room_info.content_hash >> 24), uint8_t(room_info.content_hash >> 16), uint8_t(room_info.content_hash >> 8), uint8_t(room_info.content_hash >> 0),
    };

    SDLNet_TCP_Send(socket, to_send.data(), to_send.size());
    requested_join_room_key = 0;

    _FinishJoinRoom();
}

void NetworkClient::JoinRoom(uint32_t room_key)
{
    if(!socket)
        return;

    std::array<uint8_t, 5> to_send =
    {
        HEADER_JOIN_ROOM,
        uint8_t(room_key >> 24), uint8_t(room_key >> 16), uint8_t(room_key >> 8), uint8_t(room_key >> 0),
    };

    SDLNet_TCP_Send(socket, to_send.data(), to_send.size());
    requested_join_room_key = room_key;

    _FinishJoinRoom();
}

} // namespace XMessage
