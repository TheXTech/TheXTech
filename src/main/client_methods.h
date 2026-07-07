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

enum ClientState
{
    CLIENT_OFF = 0,
    CLIENT_SESSION_CONFIG, // joined room but haven't yet sent/received session
    CLIENT_LOBBY,
    CLIENT_HOST_IDLE,
    // first active state
    CLIENT_MIN_ACTIVE_STATE,
    CLIENT_HOST_SPECTATED = CLIENT_MIN_ACTIVE_STATE,
    CLIENT_HOST,
    CLIENT_GUEST,
    CLIENT_SPECTATOR,
};

struct ClientStatus
{
    ClientState client_state = CLIENT_OFF;
    std::string server_address;
    int server_port = 4305;
    RoomInfo room_info;
    int client_index = 0;
    bool knock_knock = false;
};

struct GameThread
{
    ClientStatus status_req;
    ClientStatus status;

    // track new replies
    int status_req_reply_seen = 0;

    // track new status updates
    int status_alarm_id_seen = 0;

    void push_status_req();
    bool pull_status();
    bool status_req_completed();
};

inline XMessage::Message msg_from_frame_no(XMessage::Type type, uint32_t frame_no)
{
    XMessage::Message ret;
    ret.type = type;
    ret.screen = (uint8_t)(frame_no >> 16);
    ret.player = (uint8_t)(frame_no >> 8);
    ret.message = (uint8_t)(frame_no >> 0);

    return ret;
}

void Connect(const char* host = nullptr);
void Disconnect();
void NetStartup();
void NetShutdown();
const ClientStatus* GetClientStatus();
bool CompleteRequest();

void ClientFrameSync(std::vector<Message>& submit_queue, std::vector<Message>& message_vector);

bool RequestFillRoomInfo(uint32_t room_key);
const RoomInfo* GetRoomInfo();

void JoinNewRoom(const RoomInfo& room_info);
void JoinRoom(uint32_t room_key);
void ActivateHost();
uint32_t CurrentRoom();
void LeaveRoom();

uint32_t RoomFromString(const std::string& room);
RoomName DisplayRoom(uint32_t room_key);

} // namespace XMessage

#endif // #ifndef XCLIENT_METHODS_H
