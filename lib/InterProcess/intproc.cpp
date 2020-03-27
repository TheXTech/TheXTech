/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2020 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "../Logger/logger.h"

EditorPipe              *IntProc::editor = nullptr;
static bool             ipc_enabled = false;

static std::string      state;
static std::mutex       state_lock;

static IntProc::ExternalCommands        cmd_recentType = IntProc::MsgBox;

static std::deque<IntProc::cmdEntry>    cmd_queue;
static std::mutex                       cmd_mutex;

void IntProc::init()
{
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
    return !cmd_queue.empty();
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
