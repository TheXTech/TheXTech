#include "editor_pipe.h"
#include "../Logger/logger.h"
#include "../util.h"

#include <Utils/files.h>
#include <Utils/elapsed_timer.h>

#include <iostream>
#include <cstdio>
#include "../AppPath/app_path.h"
#include "intproc.h"
#include <fmt_format_ne.h>

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
    FileFormats::CreateLevelData(m_acceptedLevel);
    m_acceptedLevel.meta.ReadFileValid = false;
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
        IntProc::setState("LVLX Accepted, do parsing of LVLX");
    }

    if(m_doAcceptLevelData)
    {
        m_acceptedRawData.append(in);
        pLogDebug("Append LVLX data...");
    }
    else if(in.compare(0, 11, "SEND_LVLX: ") == 0)
    {
        //Delete old cached stuff
        m_acceptedRawData.clear();
        D_pLogDebug("IN: >> %s",
                    (in.size() > 30 ?
                     in.substr(0, 30).c_str() :
                     in.c_str())
                   );
        m_accepted_lvl_path   = std::string(in.c_str() + 11, (in.size() - 11));//skip "SEND_LVLX: "
        m_doAcceptLevelData  = true;
        IntProc::setState("Accepted SEND_LVLX");
        sendMessage("READY\n\n");
    }
    else if(in.compare(0, 10, "PARSE_LVLX") == 0)
    {
        pLogDebug("do Parse LVLX: PARSE_LVLX");
        m_doParseLevelData = true;
        FileFormats::ReadExtendedLvlFileRaw(m_acceptedRawData, m_accepted_lvl_path, m_acceptedLevel);
        IntProc::setState(fmt::format_ne("LVLX is valid: {0}", m_acceptedLevel.meta.ReadFileValid));
        pLogDebug("Level data parsed, Valid: %d", m_acceptedLevel.meta.ReadFileValid);

        if(!m_acceptedLevel.meta.ReadFileValid)
        {
            pLogDebug("Error reason:  %s", m_acceptedLevel.meta.ERROR_info.c_str());
            pLogDebug("line number:   %d", m_acceptedLevel.meta.ERROR_linenum);
            pLogDebug("line contents: %s", m_acceptedLevel.meta.ERROR_linedata.c_str());
            D_pLogDebug("Invalid File data BEGIN >>>>>>>>>>>\n"
                        "%s"
                        "\n<<<<<<<<<<<<INVALID File data END",
                        m_acceptedRawData.c_str());
        }

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
