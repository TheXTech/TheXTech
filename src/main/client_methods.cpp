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

#include "Logger/logger.h"
#include "controls.h"
#include "message.h"
#include "globals.h"
#include "sound.h"
#include "change_res.h"

#include "core/events.h"

#include "main/client.h"
#include "main/client_methods.h"
#include "main/screen_progress.h"

std::string g_netplayServer = "thextech.link";
std::string g_netplayNickname;

namespace XMessage
{

static GameThread s_game_thread;
static NetworkClient s_network_client;

static bool s_in_fast_forward = false;
static uint32_t s_fast_forward_begin_ms = 0;
static uint32_t s_fast_forward_begin_frame = 0;

void GameThread::push_status_req()
{
    SDL_LockMutex(s_network_client.status_req_sync_lock);
    try
    {
        s_network_client.status_req_sync = status_req;
    }
    catch(...)
    {
        // ignore exception on string copy, make sure to unlock mutex
    }
    SDL_AtomicAdd(&s_network_client.status_req_id, 1);
    SDL_UnlockMutex(s_network_client.status_req_sync_lock);
}

bool GameThread::pull_status()
{
    if(SDL_AtomicGet(&s_network_client.status_alarm_id) == status_alarm_id_seen)
        return false;

    if(SDL_TryLockMutex(s_network_client.status_sync_lock) == SDL_MUTEX_TIMEDOUT)
        return false;

    bool ret = false;

    try
    {
        status = s_network_client.status_sync;
        status_alarm_id_seen = SDL_AtomicGet(&s_network_client.status_alarm_id);
        ret = true;
    }
    catch(...)
    {
        // ignore exception on string copy, make sure to unlock mutex
    }

    SDL_UnlockMutex(s_network_client.status_sync_lock);
    return ret;
}

bool GameThread::status_req_completed()
{
    int req_id = SDL_AtomicGet(&s_network_client.status_req_id);
    if(status_req_reply_seen == req_id)
        return false;

    if(SDL_AtomicGet(&s_network_client.status_req_completed) != req_id)
        return false;

    // client thread reports that it has completed the request, we just need to be sure that we have access to its reported state
    pull_status();

    // we have a stale state
    if(status_alarm_id_seen != SDL_AtomicGet(&s_network_client.status_alarm_id))
        return false;

    status_req_reply_seen = req_id;
    return true;
}

void Connect(const char* host)
{
    s_game_thread.status_req = ClientStatus();
    s_game_thread.status_req.client_state = CLIENT_LOBBY;
    s_game_thread.status_req.server_address = (host) ? host : g_netplayServer;

    s_game_thread.push_status_req();
}

void Disconnect()
{
    s_game_thread.status_req = ClientStatus();
    s_game_thread.push_status_req();
}

void NetStartup()
{
    s_network_client.Startup();
}

void NetShutdown()
{
    s_network_client.Shutdown();
}

const ClientStatus* GetClientStatus()
{
    s_game_thread.pull_status();

    return &s_game_thread.status;
}

bool CompleteRequest()
{
    return s_game_thread.status_req_completed();
}

Status GetStatus()
{
    if(s_game_thread.status_req.client_state >= CLIENT_MIN_ACTIVE_STATE)
    {
        if(s_in_fast_forward)
            return Status::replay;
        else
            return Status::connected;
    }
    else
        return Status::local;
}

void ClientFrameSync(std::vector<Message>& submit_buffer, std::vector<Message>& message_vector)
{
    SDL_LockMutex(s_network_client.status_req_sync_lock);

    g_session.current_frame++;

    int remote_frame = g_session.remote_frame;
    int current_frame = g_session.current_frame;

    bool start_fast_forward = (current_frame < remote_frame - 8);
    bool end_fast_forward = (current_frame >= remote_frame);

    // submit requested messages
    // warning: can throw
    for(Message i : submit_buffer)
        g_session.submit_buffer.push_back(i);

    submit_buffer.clear();

    // wait for network thread to prepare current frame
    while(g_session.available_frame < current_frame && GameIsActive)
    {
        SDL_UnlockMutex(s_network_client.status_req_sync_lock);
        SDL_SemPost(s_network_client.client_wakeup);

        // wait for a wakeup call from the network thread, refreshing events every 5ms
        SDL_SemWaitTimeout(s_network_client.game_wakeup, 5);

        // this is the frame loop for waiting
        XEvents::doEvents();

        SDL_LockMutex(s_network_client.status_req_sync_lock);
    }

    // fill buffer of events that happened
    while(g_session.next_message < g_session.history.size())
    {
        Message front = g_session.history[g_session.next_message];
        if(front.type == Type::frame_begin && NetworkClient::frame_no_from_message(front) > current_frame)
            break;

        g_session.next_message++;
        // if(max_debug)
        //     pLogDebug("Message get: %d %d %d %d %d", current_frame, (int)front.type, front.screen, front.player, front.message);

        if(front.type != Type::frame_begin)
            message_vector.push_back(front);
    }

    // update current frame and unlock mutex
    SDL_UnlockMutex(s_network_client.status_req_sync_lock);

    // local logic: handle fast-forward
    if(!s_in_fast_forward && start_fast_forward)
    {
        s_in_fast_forward = true;
        s_fast_forward_begin_ms = SDL_GetTicks();
        s_fast_forward_begin_frame = current_frame;
    }
    else if(s_in_fast_forward)
    {
        IndicateProgress(s_fast_forward_begin_ms, num_t(current_frame - s_fast_forward_begin_frame) / (remote_frame - s_fast_forward_begin_frame), "Loading game history...");

        if(end_fast_forward)
        {
            s_in_fast_forward = false;
            // start playing music when no longer fast forwarding
            UpdateMusicVolume();
            // update current resolution (may need to resync screen size)
            UpdateInternalRes();
        }
    }
}

bool RequestFillRoomInfo(uint32_t room_key)
{
    s_game_thread.status_req.client_state = CLIENT_LOBBY;
    s_game_thread.status_req.room_info.room_key = room_key;

    s_game_thread.push_status_req();

    return true;
}

const RoomInfo* GetRoomInfo()
{
    s_game_thread.pull_status();
    return &s_game_thread.status.room_info;
}


void JoinNewRoom(const RoomInfo& room_info)
{
    XMessage::g_session.random_seed = iRand(2147483647);

    XMessage::g_session.current_frame = 0;
    XMessage::g_session.available_frame = -1;
    XMessage::g_session.remote_frame = -1;

    XMessage::g_session.submit_buffer.clear();
    XMessage::g_session.history.clear();
    XMessage::g_session.next_message = 0;

    s_game_thread.status_req.client_state = CLIENT_HOST;
    s_game_thread.status_req.room_info = room_info;

    s_game_thread.push_status_req();
}

void JoinRoom(uint32_t room_key)
{
    XMessage::g_session.current_frame = 0;
    XMessage::g_session.available_frame = -1;
    XMessage::g_session.remote_frame = -1;

    XMessage::g_session.submit_buffer.clear();
    XMessage::g_session.history.clear();
    XMessage::g_session.next_message = 0;

    s_game_thread.status_req.client_state = CLIENT_GUEST;
    s_game_thread.status_req.room_info.room_key = room_key;

    s_game_thread.push_status_req();
}

uint32_t CurrentRoom()
{
    s_game_thread.pull_status();
    return s_game_thread.status.room_info.room_key;
}

void LeaveRoom()
{
    s_game_thread.status_req.client_state = CLIENT_LOBBY;
    s_game_thread.status_req.room_info = RoomInfo();
    s_game_thread.push_status_req();
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
