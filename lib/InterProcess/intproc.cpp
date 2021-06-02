/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
static bool             ipc_enabled = false;
static SDL_atomic_t     has_command;

static std::string      state;
static std::mutex       state_lock;

static IntProc::ExternalCommands        cmd_recentType = IntProc::MsgBox;

static std::deque<IntProc::cmdEntry>    cmd_queue;
static std::mutex                       cmd_mutex;

void IntProc::init()
{
    SDL_AtomicSet(&has_command, 0);
    pLogDebug("IntProc constructing...");
    editor = new EditorPipe();
    editor->m_isWorking = true;
    pLogDebug("IntProc started!");
    ipc_enabled = true;
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
    ipc_enabled = false;
}

bool IntProc::isEnabled()
{
    return ipc_enabled;
}

bool IntProc::isWorking()
{
    return (editor != nullptr);
}

std::string IntProc::getState()
{
    state_lock.lock();
    std::string tmp = state;
    state_lock.unlock();
    return tmp;
}

void IntProc::setState(const std::string &instate)
{
    state_lock.lock();
    state = instate;
    state_lock.unlock();
}

void IntProc::storeCommand(const char *cmd, size_t length, IntProc::ExternalCommands type)
{
    cmd_mutex.lock();
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

    cmd_queue.push_back({in, type});
    cmd_recentType = type;
    SDL_AtomicSet(&has_command, 1);
    cmd_mutex.unlock();
}

void IntProc::cmdLock()
{
    cmd_mutex.lock();
}

void IntProc::cmdUnLock()
{
    cmd_mutex.unlock();
}

bool IntProc::hasCommand()
{
    return SDL_AtomicGet(&has_command);
}

IntProc::ExternalCommands IntProc::commandType()
{
    return cmd_recentType;
}

std::string IntProc::getCMD()
{
    cmdEntry tmp = cmd_queue.front();
    cmd_queue.pop_front();
    if(!cmd_queue.empty())
        cmd_recentType = cmd_queue.front().type;
    else
        SDL_AtomicSet(&has_command, 0);
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
