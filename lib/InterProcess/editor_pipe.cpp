#include "editor_pipe.h"
#include "../Logger/logger.h"
#include "../util.h"

#include <Utils/files.h>
#include <Utils/elapsed_timer.h>
#include <Utils/strings.h>

#include <iostream>
#include <cstdio>
#include "../AppPath/app_path.h"
#include "intproc.h"
#include <fmt_format_ne.h>

#include "main/game_strings.h"

/**************************************************************************************************************/

int EditorPipe::run(void *self)
{
    EditorPipe &me = *reinterpret_cast<EditorPipe *>(self);

    while(me.m_thread_isAlive)
    {
        std::string buffer;
        std::string out;
        std::cin >> buffer;

        if(me.m_thread_isAlive)
        {
            util::base64_decode(out, buffer);
            me.icomingData(out);
        }
    }

    return 0;
}

void EditorPipe::start()
{
    std::cin.sync_with_stdio(false);
#ifndef PGE_NO_THREADING
    m_thread_isAlive = true;
    m_thread = SDL_CreateThread(&run, "EditorPipe_std", this);
    SDL_DetachThread(m_thread);
#endif
}

void EditorPipe::stop()
{
    m_thread_isAlive = false;
    //int status = 0;
    //SDL_WaitThread(m_thread, &status);
}

void EditorPipe::sendMessage(const std::string &in)
{
    std::string outO;
    util::base64_encode(outO, reinterpret_cast<const unsigned char *>(in.c_str()), in.size());
    D_pLogDebug("EditorPipe::sendMessage: sending data [%s]", outO.c_str());
    std::fprintf(stdout, "%s\n", outO.c_str());
    std::fflush(stdout);
}

void EditorPipe::sendMessage(const char *in)
{
    std::string outO;
    util::base64_encode(outO, reinterpret_cast<const unsigned char *>(in), strlen(in));
    D_pLogDebug("EditorPipe::sendMessage: sending data [%s]", outO.c_str());
    std::fprintf(stdout, "%s\n", outO.c_str());
    std::fflush(stdout);
}
/**************************************************************************************************************/


/**
 * @brief EditorPipe::LocalServer
 *  Constructor
 */

EditorPipe::EditorPipe():
    m_thread(nullptr),
    m_thread_isAlive(false),
    m_isWorking(false),
    m_doAcceptLevelData(false),
    m_doParseLevelData(false),
    m_levelAccepted(false)
{
    m_acceptedRawData.clear();
    pLogDebug("Construct interprocess pipe...");
    start();
}

/**
 * @brief EditorPipe::~LocalServer
 *  Destructor
 */
EditorPipe::~EditorPipe()
{
    pLogDebug("Destroying interprocess pipe...");
    stop();
}

void EditorPipe::sendStarsNumber(int numStars)
{
    sendMessage(fmt::sprintf_ne("CMD:NUM_STARS %d", numStars));
}

void EditorPipe::sendTakenBlock(const LevelBlock &block)
{
    std::string encoded;
    LevelData buffer;
    FileFormats::CreateLevelData(buffer);
    buffer.blocks.push_back(block);
    buffer.layers.clear();
    buffer.events.clear();
    if(FileFormats::WriteExtendedLvlFileRaw(buffer, encoded))
        sendMessage(fmt::format_ne("CMD:TAKEN_BLOCK\nTAKEN_BLOCK_END\n{0}", encoded));
}

void EditorPipe::sendTakenBGO(const LevelBGO &bgo)
{
    std::string encoded;
    LevelData buffer;
    FileFormats::CreateLevelData(buffer);
    buffer.bgo.push_back(bgo);
    buffer.layers.clear();
    buffer.events.clear();
    if(FileFormats::WriteExtendedLvlFileRaw(buffer, encoded))
        sendMessage(fmt::format_ne("CMD:TAKEN_BGO\nTAKEN_BGO_END\n{0}", encoded));
}

void EditorPipe::sendTakenNPC(const LevelNPC &npc)
{
    std::string encoded;
    LevelData buffer;
    FileFormats::CreateLevelData(buffer);
    buffer.npc.push_back(npc);
    buffer.layers.clear();
    buffer.events.clear();
    if(FileFormats::WriteExtendedLvlFileRaw(buffer, encoded))
        sendMessage(fmt::format_ne("CMD:TAKEN_NPC\nTAKEN_NPC_END\n{0}", encoded));
}

void EditorPipe::sendCloseProperties()
{
    sendMessage("CMD:CLOSE_PROPERTIES");
}

void EditorPipe::sendPlayerSettings(int playerId, int character, int state, int vehicleID, int vehicleState)
{
    sendMessage(fmt::sprintf_ne("CMD:PLAYER_SETUP_UPDATE %d %d %d %d %d", playerId, character, state, vehicleID, vehicleState));
}

void EditorPipe::sendPlayerSettings2(int playerId, int health, int reservedItem)
{
    sendMessage(fmt::sprintf_ne("CMD:PLAYER_SETUP_UPDATE2 %d %d %d", playerId, health, reservedItem));
}

void EditorPipe::shut()
{
    sendMessage("CMD:ENGINE_CLOSED");
    stop();
}

bool EditorPipe::hasLevelData()
{
    m_levelAccepted_lock.lock();
    bool state = m_levelAccepted;
    m_levelAccepted = false;
    m_levelAccepted_lock.unlock();
    return state;
}

bool EditorPipe::levelReceivingInProcess()
{
    return m_doAcceptLevelData;
}

void EditorPipe::icomingData(const std::string &in)
{
    if(in.compare(0, 10, "PARSE_LVLX") == 0)
    {
        m_doAcceptLevelData = false;
        pLogDebug("LVLX accepting is done!");
        IntProc::setState(g_gameStrings.ipcStatusDataAccepted);
    }

    if(m_doAcceptLevelData)
    {
        m_acceptedRawData.append(in);

        if(m_doAcceptLevelDataParts)
            sendMessage("LVLX_NEXT\n\n");

        pLogDebug("Append LVLX data...");
    }
    else if(in.compare(0, 11, "SEND_LVLX: ") == 0 || in.compare(0, 17, "SEND_LVLX_PARTS: ") == 0)
    {
        m_doAcceptLevelDataParts = (in.compare(0, 17, "SEND_LVLX_PARTS: ") == 0);
        size_t offset = m_doAcceptLevelDataParts ? 17 : 11;
        //Delete old cached stuff
        m_acceptedRawData.clear();
        D_pLogDebug("IN: >> %s",
                    (in.size() > 30 ?
                     in.substr(0, 30).c_str() :
                     in.c_str())
                   );
        m_accepted_lvl_path   = std::string(in.c_str() + offset, (in.size() - offset));//skip "SEND_LVLX: "
        Strings::doTrim(m_accepted_lvl_path);
        m_doAcceptLevelData  = true;
        IntProc::setState(g_gameStrings.ipcStatusDataTransferStarted);
        sendMessage(m_doAcceptLevelDataParts ? "READY_PARTS\n\n" : "READY\n\n");
    }
    else if(in.compare(0, 10, "PARSE_LVLX") == 0)
    {
        pLogDebug("do Parse LVLX: PARSE_LVLX");
        m_doParseLevelData = true;
        m_doAcceptLevelDataParts = false;
        m_acceptedLevel.open(&m_acceptedRawData, m_accepted_lvl_path);
        IntProc::setState(g_gameStrings.ipcStatusDataValid);

        m_levelAccepted_lock.lock();
        m_levelAccepted = true;
        m_levelAccepted_lock.unlock();
    }
    else if(in.compare(0, 11, "PLACEITEM: ") == 0)
    {
        D_pLogDebugNA("Accepted Placing item!");
        IntProc::storeCommand(in.c_str() + 11, in.size() - 11, IntProc::PlaceItem);
    }
    else if(in.compare(0, 11, "SET_LAYER: ") == 0)
    {
        D_pLogDebugNA("Accepted layer change!");
        IntProc::storeCommand(in.c_str() + 11, in.size() - 11, IntProc::SetLayer);
    }
    else if(in.compare(0, 14, "SET_NUMSTARS: ") == 0)
    {
        D_pLogDebugNA("Accepted stars number change!");
        IntProc::storeCommand(in.c_str() + 14, in.size() - 14, IntProc::SetNumStars);
    }
    else if(in.compare(0, 8, "MSGBOX: ") == 0)
    {
        pLogDebug("Accepted Message box: %s", in.c_str());
        IntProc::storeCommand(in.c_str() + 8, in.size() - 8, IntProc::MsgBox);
    }
    else if(in.compare(0, 7, "CHEAT: ") == 0)
    {
        pLogDebug("Accepted cheat code: %s", in.c_str());
        IntProc::storeCommand(in.c_str() + 7, in.size() - 7, IntProc::Cheat);
    }
    else if(in.compare(0, 4, "PING") == 0)
    {
        D_pLogDebugNA("IN: >> PING");
        sendMessage("PONG\n\n");
        pLogDebug("Ping-Pong!");
    }
}
