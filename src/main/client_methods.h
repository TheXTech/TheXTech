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
#error "client_methods.h cannot be included in a build without SDL_net support"
#endif

#ifndef XCLIENT_METHODS_H
#define XCLIENT_METHODS_H

#include <cstdint>
#include <string>

#include "message.h"

extern std::string g_netplayServer;
extern std::string g_netplayNickname;


namespace XMessage
{

struct RoomInfo
{
    uint32_t room_key = 0;
    uint32_t engine_hash = 0;
    uint32_t asset_hash = 0;
    uint32_t content_hash = 0;
};


struct RoomName
{
    char room_name[8];
};

void Connect(const char* host = nullptr);
bool IsConnected();
void Disconnect();

void ClientFrameSync();

void RequestFillRoomInfo(RoomInfo& room_info);

void JoinNewRoom(const RoomInfo& room_info);
void JoinRoom(uint32_t room_key);
uint32_t CurrentRoom();
void LeaveRoom();

uint32_t RoomFromString(const std::string& room);
RoomName DisplayRoom(uint32_t room_key);

} // namespace XMessage

#endif // #ifndef XCLIENT_METHODS_H
