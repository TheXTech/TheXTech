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

#include "logger_sets.h"

#ifndef DISABLE_LOGGING
#include <IniProcessor/ini_processing.h>
#include <fmt_format_ne.h>
#include <fmt/fmt_printf.h>
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <stdarg.h>
#include <cstdio>
#include <mutex>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream>
#include <algorithm>

#include <AppPath/app_path.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/trace.h>
#define LOG_CHANNEL "Application"
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

static std::mutex g_lockLocker;
#define OUT_BUFFER_SIZE 10240
static char       g_outputBuffer[OUT_BUFFER_SIZE];

class MutexLocker
{
        std::mutex *m_mutex;
    public:
        MutexLocker(std::mutex *mutex)
        {
            m_mutex = mutex;
            m_mutex->lock();
        }
        ~MutexLocker()
        {
            m_mutex->unlock();
        }
};

std::string     LogWriter::m_logDirPath;
std::string     LogWriter::m_logFilePath;
PGE_LogLevel    LogWriter::m_logLevel;
int             LogWriter::m_maxFilesCount = 10;
bool            LogWriter::m_enabled = false;
bool            LogWriter::m_enabledStdOut = false;
bool            LogWriter::m_enabledVerboseLogs = false;

bool  LogWriter::m_logIsOpened = false;
SDL_RWops *LogWriter::m_logout = nullptr;

#ifndef __EMSCRIPTEN__
static std::string return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    char out[24];
    memset(out, 0, sizeof(out));
    if(0 < strftime(out, sizeof(out), "%Y_%m_%d_%H_%M_%S", std::localtime(&in_time_t)))
        return std::string(out);
    else
        return "0000_00_00_00_00_00";
}
#endif//__EMSCRIPTEN__

static void cleanUpLogs(const std::string &logsPath, int maxLogs)
{
    if(maxLogs <= 0)
        return; // No limit

    DirMan d(logsPath);
    std::vector<std::string> files, filesPre;
    d.getListOfFiles(filesPre, {".txt"});

    // Be sure that we are looking for our log files and don't touch others
    for(auto &s : filesPre)
    {
        if(SDL_strncasecmp(s.c_str(), "TheXTech_log_", 13) == 0)
            files.push_back(s);
    }

    // nothing to do, count of log files is fine
    if(static_cast<int>(files.size()) <= (maxLogs - 1))
        return;

    // Sort array
    std::sort(files.begin(), files.end());

    // Keep these files (remove from a deletion list)
    files.erase(files.end() - (maxLogs - 1), files.end());

    for(auto &s : files)
        Files::deleteFile(logsPath + "/" + s);
}

void LogWriter::LoadLogSettings(bool disableStdOut, bool verboseLogs)
{
    MutexLocker mutex(&g_lockLocker);
    (void)(mutex);
#ifdef __EMSCRIPTEN__
    m_logLevel = PGE_LogLevel::Debug;
    m_logIsOpened = false;
    m_enabled = true;
    std::fprintf(stdout, "Emscripten logs stdout.\n");
#else
    std::string logFileName = fmt::format_ne("TheXTech_log_{0}.txt", return_current_time_and_date());

#if defined(DEBUG_BUILD)
    (void)verboseLogs; // unused
    m_enabledVerboseLogs = true; // Enforce verbose log for debug builds
#else
    m_enabledVerboseLogs = verboseLogs;
#endif
    m_enabledStdOut = !disableStdOut;
    m_logLevel = PGE_LogLevel::Debug;
    std::string mainIniFile = AppPathManager::settingsFileSTD();
    IniProcessing logSettings(mainIniFile);

    std::string logPath = AppPathManager::userAppDirSTD() + "logs";
    DirMan defLogDir(logPath);

    if(!defLogDir.exists())
    {
        if(!defLogDir.mkpath())
            defLogDir.setPath(AppPathManager::userAppDirSTD());
    }

    logSettings.beginGroup("logging");
    {
        logSettings.read("log-path", m_logDirPath, defLogDir.absolutePath());
        logSettings.read("max-log-count", m_maxFilesCount, 10);
        if(!DirMan::exists(m_logDirPath))
            m_logDirPath = defLogDir.absolutePath();
        m_logLevel  = static_cast<PGE_LogLevel>(logSettings.value("log-level", static_cast<int>(PGE_LogLevel::Debug)).toInt());
        m_enabled   = (m_logLevel != PGE_LogLevel::NoLog);
    }
    logSettings.endGroup();

    m_logFilePath = m_logDirPath + "/" + logFileName;

    cleanUpLogs(m_logDirPath, m_maxFilesCount);

    if(!disableStdOut)
    {
        try
        {
            fmt::print("LogLevel {0}, log file {1}\n\n", static_cast<int>(m_logLevel), m_logFilePath);
        }
        catch(const fmt::FormatError &err)
        {
            std::fprintf(stderr, "fmt::print failed with exception: %s\n", err.what());
            abort();
        }
    }

    if(m_enabled)
    {
        m_logout = SDL_RWFromFile(m_logFilePath.c_str(), "a");
        if(m_logout)
        {
            m_logIsOpened = true;
        }
        else
        {
            std::fprintf(stderr, "Impossible to open %s for write, log printing is disabled!\n", m_logFilePath.c_str());
            std::fflush(stderr);
        }
    }
#endif
}

void LoadLogSettings(bool disableStdOut, bool verboseLogs)
{
    LogWriter::LoadLogSettings(disableStdOut, verboseLogs);
}

void CloseLog()
{
#ifndef __EMSCRIPTEN__
    MutexLocker mutex(&g_lockLocker);
    (void)(mutex);
    if(LogWriter::m_logout)
        SDL_RWclose(LogWriter::m_logout);
    LogWriter::m_logout = nullptr;
    //LogWriter::m_out_stream.reset();
    LogWriter::m_logIsOpened = false;
#endif
}

#ifdef _WIN32
#define OS_NEWLINE "\r\n"
#define OS_NEWLINE_LEN 2
#else
#define OS_NEWLINE "\n"
#define OS_NEWLINE_LEN 1
#endif

#if defined(__ANDROID__)
static int pgeToAndroidLL(PGE_LogLevel level)
{
    switch(level)
    {
    case PGE_LogLevel::NoLog:
        return ANDROID_LOG_INFO;
    case PGE_LogLevel::Fatal:
        return ANDROID_LOG_FATAL;
    case PGE_LogLevel::Info:
        return ANDROID_LOG_INFO;
    case PGE_LogLevel::Critical:
        return ANDROID_LOG_ERROR;
    case PGE_LogLevel::Warning:
        return ANDROID_LOG_WARN;
    case PGE_LogLevel::Debug:
        return ANDROID_LOG_DEBUG;
    }
    // For just in a case
    return ANDROID_LOG_INFO;
}
#endif

static void pLogGeneric(PGE_LogLevel level, const char *label, const char *format, va_list arg)
{
    va_list arg_in;

#if !defined(__ANDROID__)
    if(LogWriter::m_enabledStdOut && LogWriter::m_enabledVerboseLogs)
    {
        va_copy(arg_in, arg);
        std::fprintf(stdout, "%s: ", label);
        std::vfprintf(stdout, format, arg_in);
        std::fprintf(stdout, OS_NEWLINE);
        std::fflush(stdout);
        va_end(arg_in);
    }
#endif

#if defined(__ANDROID__)
    va_copy(arg_in, arg);
    __android_log_vprint(pgeToAndroidLL(level), "TRACKERS", format, arg_in);
    va_end(arg_in);
#endif

#ifndef __EMSCRIPTEN__
    if(LogWriter::m_logout == nullptr)
        return;
#endif

    if(LogWriter::m_logLevel < level)
        return;

    MutexLocker mutex(&g_lockLocker);
    ((void)mutex);

#ifdef __EMSCRIPTEN__
    va_copy(arg_in, arg);
    int len = std::vsnprintf(g_outputBuffer, OUT_BUFFER_SIZE, format, arg_in);
    std::fprintf(stdout, "%s: %s\n", label, g_outputBuffer);
    std::fflush(stdout);
    va_end(arg_in);
#else
    va_copy(arg_in, arg);
    int len = SDL_snprintf(g_outputBuffer, OUT_BUFFER_SIZE, "%s: ", label);
    SDL_RWwrite(LogWriter::m_logout, g_outputBuffer, 1, (size_t)len);
    len = SDL_vsnprintf(g_outputBuffer, OUT_BUFFER_SIZE, format, arg_in);
    SDL_RWwrite(LogWriter::m_logout, g_outputBuffer, 1, (size_t)len);
    SDL_RWwrite(LogWriter::m_logout, reinterpret_cast<const void *>(OS_NEWLINE), 1, OS_NEWLINE_LEN);
    va_end(arg_in);
#endif

}

void pLogDebug(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    pLogGeneric(PGE_LogLevel::Debug, "Debug", format, arg);
    va_end(arg);
}

void pLogWarning(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    pLogGeneric(PGE_LogLevel::Warning, "Warning", format, arg);
    va_end(arg);
}

void pLogCritical(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    pLogGeneric(PGE_LogLevel::Critical, "Critical", format, arg);
    va_end(arg);
}

void pLogFatal(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    pLogGeneric(PGE_LogLevel::Fatal, "Fatal", format, arg);
    va_end(arg);
}

void pLogInfo(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    pLogGeneric(PGE_LogLevel::Info, "Info", format, arg);
    va_end(arg);
}

void WriteToLog(PGE_LogLevel type, const std::string &msg)
{
    LogWriter::WriteToLog(type, msg);
}

std::string getLogFilePath()
{
    return LogWriter::m_logFilePath;
}

void LogWriter::WriteToLog(PGE_LogLevel type, const std::string &msg)
{
    if(!m_enabled)
        return; //if logging disabled

    if(!m_logIsOpened)
    {
        switch(type)
        {
        case PGE_LogLevel::Debug:
            std::fprintf(stderr, "DEBUG: %s\n", msg.c_str());
            break;
        case PGE_LogLevel::Warning:
            std::fprintf(stderr, "WARNING: %s\n", msg.c_str());
            break;
        case PGE_LogLevel::Critical:
            std::fprintf(stderr, "CRITICAL ERROR: %s\n", msg.c_str());
            break;
        case PGE_LogLevel::Info:
            std::fprintf(stderr, "INFO: %s\n", msg.c_str());
            break;
        case PGE_LogLevel::Fatal:
            std::fprintf(stderr, "FATAL ERROR: %s\n", msg.c_str());
            break;
        case PGE_LogLevel::NoLog:
            break;
        }
        std::fflush(stderr);
        return;
    }

    switch(type)
    {
    case PGE_LogLevel::Debug:
        if(m_logLevel < PGE_LogLevel::Debug)
            return;
        pLogDebug("%s", msg.c_str());
        break;
    case PGE_LogLevel::Warning:
        if(m_logLevel < PGE_LogLevel::Warning)
            return;
        pLogWarning("%s", msg.c_str());
        break;
    case PGE_LogLevel::Critical:
        if(m_logLevel < PGE_LogLevel::Critical)
            return;
        pLogCritical("%s", msg.c_str());
        break;
    case PGE_LogLevel::Info:
        if(m_logLevel < PGE_LogLevel::Info)
            return;
        pLogInfo("%s", msg.c_str());
        break;
    case PGE_LogLevel::Fatal:
        if(m_logLevel < PGE_LogLevel::Fatal)
            return;
        pLogFatal("%s", msg.c_str());
        break;
    case PGE_LogLevel::NoLog:
        return;
    }
}

#else // DISABLE_LOGGING

/* Dummies  */

std::string  LogWriter::m_logFilePath;
PGE_LogLevel LogWriter::m_logLevel = PGE_LogLevel::NoLog;
bool  LogWriter::m_enabled = false;
bool  LogWriter::m_enabledStdOut = false;
bool  LogWriter::m_logIsOpened = false;
SDL_RWops *LogWriter::m_logout = nullptr;


void LogWriter::LoadLogSettings(bool)
{}

void LoadLogSettings(bool)
{}

void CloseLog()
{}

void pLogDebug(const char *, ...)
{}

void pLogWarning(const char *, ...)
{}

void pLogCritical(const char *, ...)
{}

void pLogFatal(const char *, ...)
{}

void pLogInfo(const char *, ...)
{}

std::string getLogFilePath()
{
    return "<No log file: Logging was been disabled in this build>";
}

void LogWriter::WriteToLog(PGE_LogLevel, const std::string &)
{}

#endif // DISABLE_LOGGING
