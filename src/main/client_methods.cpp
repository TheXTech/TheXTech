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

#include "main/client.h"
#include "main/client_methods.h"

namespace XMessage
{

static NetworkClient s_network_client;

void Connect(const char* host, int port)
{
    s_network_client.Connect(host, port);
}

bool IsConnected()
{
    return s_network_client.socket;
}

void Disconnect()
{
    s_network_client.Disconnect();
}

Status GetStatus()
{
    if(CurrentRoom())
    {
        if(s_network_client.tick < s_network_client.fast_forward_to)
            return Status::replay;
        else
            return Status::connected;
    }
    else
        return Status::local;
}

void ClientFrameSync()
{
    s_network_client.SendAll();
    s_network_client.WaitAndFill();
}

void RequestFillRoomInfo(RoomInfo& room_info)
{
    s_network_client.RequestFillRoomInfo(room_info);
}

void JoinNewRoom(const RoomInfo& room_info)
{
    s_network_client.JoinNewRoom(room_info);
}

void JoinRoom(uint32_t room_key)
{
    s_network_client.JoinRoom(room_key);
}

uint32_t CurrentRoom()
{
    return s_network_client.room_key;
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
