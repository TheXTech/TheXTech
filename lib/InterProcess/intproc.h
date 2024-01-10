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

#ifndef INTPROC_H
#define INTPROC_H

#ifdef THEXTECH_INTERPROC_SUPPORTED

#include <mutex>
#include <deque>
#include <string>
#include "editor_pipe.h"

namespace IntProc
{
    void init();
    void quit();
    bool isEnabled();
    bool isWorking();

    void sendStarsNumber(int numStars);
    void sendTakenBlock(const LevelBlock &block);
    void sendTakenBGO(const LevelBGO &bgo);
    void sendTakenNPC(const LevelNPC &npc);
    void sendPlayerSettings(int playerId, int character, int state, int vehicleID, int vehicleState);
    void sendCloseProperties();

    std::string getState();
    void        setState(const std::string &instate);

    enum ExternalCommands
    {
        //! Show messag ebox
        MsgBox = 0,
        //! Cheat code proxy
        Cheat = 1,
        //! Place item (magic-hand only)
        PlaceItem = 2,
        //! Toggle a name of current
        SetLayer = 3,
        //! Set number of taken stars
        SetNumStars = 4
    };

    struct cmdEntry
    {
        std::string cmd;
        ExternalCommands type;
    };

    /**
     * @brief IPC interface has accepted level data
     * @return true if IPC interface keeps a complete level data buffer
     */
    bool hasLevelData();
    /**
     * @brief Is level data in process of receiving
     * @return true if receiving is in process
     */
    bool levelReceivingInProcess();

    bool sendMessage(const char *command);
    bool sendMessageS(const std::string &command);
    bool sendMessage(const std::string &command);

    void             storeCommand(const char *cmd, size_t length, ExternalCommands type);
    void             cmdLock();
    void             cmdUnLock();
    bool             hasCommand();
    ExternalCommands commandType();
    std::string getCMD();

    extern EditorPipe *editor;

}// namespace IntProc

#endif // THEXTECH_INTERPROC_SUPPORTED

#endif // INTPROC_H
