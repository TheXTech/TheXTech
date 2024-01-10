/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#include "intproc.h"
#include <SDL2/SDL_atomic.h>
#include "../Logger/logger.h"

EditorPipe              *IntProc::editor = nullptr;
static bool             s_ipc_enabled = false;
static SDL_atomic_t     s_has_command;

static std::string      s_state;
static std::mutex       s_state_lock;

static IntProc::ExternalCommands        s_cmd_recentType = IntProc::MsgBox;

static std::deque<IntProc::cmdEntry>    s_cmd_queue;
static std::mutex                       s_cmd_mutex;

void IntProc::init()
{
    SDL_AtomicSet(&s_has_command, 0);
    pLogDebug("IntProc constructing...");
    editor = new EditorPipe();
    editor->m_isWorking = true;
    pLogDebug("IntProc started!");
    s_ipc_enabled = true;
}

void IntProc::quit()
{
    if(editor != nullptr)
    {
        editor->m_isWorking = false;
        editor->shut();
        delete editor;
    }

    editor  = nullptr;
    s_ipc_enabled = false;
}

bool IntProc::isEnabled()
{
    return s_ipc_enabled;
}

bool IntProc::isWorking()
{
    return (editor != nullptr);
}

std::string IntProc::getState()
{
    s_state_lock.lock();
    std::string tmp = s_state;
    s_state_lock.unlock();
    return tmp;
}

void IntProc::setState(const std::string &instate)
{
    s_state_lock.lock();
    s_state = instate;
    s_state_lock.unlock();
}

void IntProc::storeCommand(const char *cmd, size_t length, IntProc::ExternalCommands type)
{
    s_cmd_mutex.lock();
    std::string in;
    in.reserve(length);
    bool escape = false;

    for(size_t i = 0; i < length; i++)
    {
        const char &c = cmd[i];

        if(escape)
        {
            if(c == 'n')
                in.push_back('\n');

            escape = false;
        }
        else if(c == '\\')
        {
            escape = true;
            continue;
        }
        else
            in.push_back(c);
    }

    s_cmd_queue.push_back({in, type});
    s_cmd_recentType = type;
    SDL_AtomicSet(&s_has_command, 1);
    s_cmd_mutex.unlock();
}

void IntProc::cmdLock()
{
    s_cmd_mutex.lock();
}

void IntProc::cmdUnLock()
{
    s_cmd_mutex.unlock();
}

bool IntProc::hasCommand()
{
    return SDL_AtomicGet(&s_has_command);
}

IntProc::ExternalCommands IntProc::commandType()
{
    return s_cmd_recentType;
}

std::string IntProc::getCMD()
{
    cmdEntry tmp = s_cmd_queue.front();
    s_cmd_queue.pop_front();
    if(!s_cmd_queue.empty())
        s_cmd_recentType = s_cmd_queue.front().type;
    else
        SDL_AtomicSet(&s_has_command, 0);
    return tmp.cmd;
}

bool IntProc::sendMessage(const std::string &command)
{
    if(!editor)
    {
        pLogWarning("IntProc::sendMessage: `editor` is not initialized!");
        return false;
    }

    editor->sendMessage(command);
    return true;
}

bool IntProc::sendMessageS(const std::string &command)
{
    if(!editor)
    {
        pLogWarning("IntProc::sendMessageS: `editor` is not initialized!");
        return false;
    }

    editor->sendMessage(command);
    return true;
}

bool IntProc::sendMessage(const char *command)
{
    if(!editor)
    {
        pLogWarning("IntProc::sendMessage: `editor` is not initialized!");
        return false;
    }

    editor->sendMessage(command);
    return true;
}

bool IntProc::hasLevelData()
{
    if(!editor)
        return false;
    return editor->hasLevelData();
}

bool IntProc::levelReceivingInProcess()
{
    if(!editor)
        return false;
    return editor->levelReceivingInProcess();
}

void IntProc::sendStarsNumber(int numStars)
{
    if(!editor)
        return;
    editor->sendStarsNumber(numStars);
}

void IntProc::sendTakenBlock(const LevelBlock &block)
{
    if(!editor)
        return;
    editor->sendTakenBlock(block);
}

void IntProc::sendTakenBGO(const LevelBGO &bgo)
{
    if(!editor)
        return;
    editor->sendTakenBGO(bgo);
}

void IntProc::sendTakenNPC(const LevelNPC &npc)
{
    if(!editor)
        return;
    editor->sendTakenNPC(npc);
}

void IntProc::sendPlayerSettings(int playerId, int character, int state, int vehicleID, int vehicleState)
{
    if(!editor)
        return;
    editor->sendPlayerSettings(playerId, character, state, vehicleID, vehicleState);
}

void IntProc::sendCloseProperties()
{
    if(!editor)
        return;
    editor->sendCloseProperties();
}
