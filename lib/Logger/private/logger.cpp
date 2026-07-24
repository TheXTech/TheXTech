/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <stdarg.h>
#include "../logger.h"
#include "logger_sets.h"

#ifndef NO_FILE_LOGGING
#   include <chrono>  // chrono::system_clock
#   include <ctime>   // localtime
#   include <DirManager/dirman.h>
#   include <Utils/files.h>
#endif

#include <fmt_format_ne.h>
#include <fmt/fmt_printf.h>

#include "sdl_proxy/sdl_stdinc.h"   /* IWYU pragma: keep */
#include "sdl_proxy/sdl_timer.h"    /* IWYU pragma: keep */
#ifndef THEXTECH_NO_SDL_CORE
#   include <SDL2/SDL_log.h>
#endif

#ifndef MOONDUST_LOGGER_FILENAME_PREFIX
#   error Please define the "-DMOONDUST_LOGGER_FILENAME_PREFIX=<name-of-application>" to specify the log filename prefix
#endif

PGE_LogSetup    g_pLogGlobalSetup;

std::string     LogWriter::m_logDirPath;
std::string     LogWriter::m_logFilePath;
PGE_LogLevel::Level LogWriter::m_logLevel = PGE_LogLevel::NoLog;
int             LogWriter::m_maxFilesCount = 10;
bool            LogWriter::m_enabled = false;
bool            LogWriter::m_enabledStdOut = false;
bool            LogWriter::m_enabledVerboseLogs = false;
uint64_t        LogWriter::m_appStartTicks = 0;

static std::string getRunTimeString()
{
    uint64_t ticks_fms = SDL_GetTicks64() - LogWriter::m_appStartTicks;
    uint64_t ticks_fs = ticks_fms / 1000;
    uint64_t ticks_fm = ticks_fs / 60;
    uint64_t ticks_h = ticks_fm / 60;

    uint64_t ticks_ms = ticks_fms % 1000;
    uint64_t ticks_s = ticks_fs % 60;
    uint64_t ticks_m = ticks_fm % 60;

    if(ticks_h > 0)
        return fmt::sprintf_ne("%02d:%02d:%02d.%03d",
                               (int)ticks_h, (int)ticks_m, (int)ticks_s, (int)ticks_ms);
    else
        return fmt::sprintf_ne("%02d:%02d.%03d",
                               (int)ticks_m, (int)ticks_s, (int)ticks_ms);
}

#if !defined(NO_FILE_LOGGING)
static std::string return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

#ifdef PGE_MIN_PORT
    // chrono doesn't work reliably here
    time(&in_time_t);
#endif

    char out[24];
    SDL_memset(out, 0, sizeof(out));
    if(0 < strftime(out, sizeof(out), "%Y_%m_%d_%H_%M_%S", std::localtime(&in_time_t)))
        return std::string(out);
    else
        return "0000_00_00_00_00_00";
}

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
        if(SDL_strncasecmp(s.c_str(), MOONDUST_LOGGER_FILENAME_PREFIX "_log_", 13) == 0)
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
#endif // !NO_FILE_LOGGING


#ifndef THEXTECH_NO_SDL_CORE
static void sdl_logger_callback(void *, int category, SDL_LogPriority priority, const char *message)
{
    const char *cat_str;

    switch(category)
    {
    case SDL_LOG_CATEGORY_APPLICATION:
        cat_str = "App";
        break;
    case SDL_LOG_CATEGORY_ERROR:
        cat_str = "Error";
        break;
    case SDL_LOG_CATEGORY_ASSERT:
        cat_str = "Assert";
        break;
    case SDL_LOG_CATEGORY_SYSTEM:
        cat_str = "System";
        break;
    case SDL_LOG_CATEGORY_AUDIO:
        cat_str = "Audio";
        break;
    case SDL_LOG_CATEGORY_VIDEO:
        cat_str = "Video";
        break;
    case SDL_LOG_CATEGORY_RENDER:
        cat_str = "Render";
        break;
    case SDL_LOG_CATEGORY_INPUT:
        cat_str = "Input";
        break;
    case SDL_LOG_CATEGORY_TEST:
        cat_str = "Test";
        break;

    default:
        cat_str = "Custom";
        break;
    }

    switch(priority)
    {
    case SDL_LOG_PRIORITY_VERBOSE:
        pLogDebug("V-SDL2 (%d:%s): %s", category, cat_str, message);
        break;

    case SDL_LOG_PRIORITY_DEBUG:
        pLogDebug("D-SDL2 (%d:%s): %s", category, cat_str, message);
        break;

    case SDL_LOG_PRIORITY_INFO:
        pLogInfo("I-SDL2 (%d:%s): %s", category, cat_str, message);
        break;

    case SDL_LOG_PRIORITY_WARN:
        pLogWarning("W-SDL2 (%d:%s): %s", category, cat_str, message);
        break;

    case SDL_LOG_PRIORITY_ERROR:
        pLogCritical("E-SDL2 (%d:%s): %s", category, cat_str, message);
        break;

    default:
    case SDL_LOG_PRIORITY_CRITICAL:
        pLogFatal("C-SDL2 (%d:%s): %s", category, cat_str, message);
        break;
    }
}

static SDL_LogPriority log_pge2sdl(PGE_LogLevel::Level l)
{
    switch(l)
    {
    case PGE_LogLevel::Debug:
        return SDL_LOG_PRIORITY_VERBOSE;

    case PGE_LogLevel::Info:
        return SDL_LOG_PRIORITY_INFO;

    case PGE_LogLevel::Warning:
        return SDL_LOG_PRIORITY_WARN;

    case PGE_LogLevel::Critical:
        return SDL_LOG_PRIORITY_ERROR;

    case PGE_LogLevel::Fatal:
    case PGE_LogLevel::NoLog:
    default:
        return SDL_LOG_PRIORITY_CRITICAL;
    }
}
#endif


void LoadLogSettings(bool disableStdOut, bool verboseLogs)
{
#if defined(DEBUG_BUILD) || defined(__WIIU__) || defined(THEXTECH_IOS)
    (void)verboseLogs; // unused
    LogWriter::m_enabledVerboseLogs = true; // Enforce verbose log for debug builds or on some platforms like WiiU
#else
    LogWriter::m_enabledVerboseLogs = verboseLogs;
#endif

    LogWriter::m_enabledStdOut = !disableStdOut;
    LogWriter::m_logLevel = g_pLogGlobalSetup.level;

#if !defined(NO_FILE_LOGGING)
    std::string logFileName = fmt::format_ne(MOONDUST_LOGGER_FILENAME_PREFIX "_log_{0}.txt", return_current_time_and_date());
#endif

    LogWriter::m_appStartTicks = SDL_GetTicks64();

#if !defined(NO_FILE_LOGGING)
    const std::string logPath = g_pLogGlobalSetup.logPathDefault;
    DirMan defLogDir(logPath);

    if(!defLogDir.exists())
    {
        if(!defLogDir.mkpath())
            defLogDir.setPath(g_pLogGlobalSetup.logPathFallBack);
    }
#endif // !NO_FILE_LOGGING

#if !defined(NO_FILE_LOGGING)
    LogWriter::m_logDirPath = g_pLogGlobalSetup.logPathCustom.empty() ? defLogDir.absolutePath() : g_pLogGlobalSetup.logPathCustom;
    LogWriter::m_maxFilesCount = g_pLogGlobalSetup.maxFilesCount;

    if(!DirMan::exists(LogWriter::m_logDirPath))
        LogWriter::m_logDirPath = defLogDir.absolutePath();
#endif // !NO_FILE_LOGGING

    LogWriter::m_logLevel = g_pLogGlobalSetup.level;
    LogWriter::m_enabled   = (LogWriter::m_logLevel != PGE_LogLevel::NoLog);

#if !defined(NO_FILE_LOGGING)
    LogWriter::m_logFilePath = LogWriter::m_logDirPath + "/" + logFileName;
    cleanUpLogs(LogWriter::m_logDirPath, LogWriter::m_maxFilesCount);
#else
    LogWriter::m_logFilePath.clear();
#endif // !NO_FILE_LOGGING

#ifndef THEXTECH_NO_SDL_CORE
    // Forward SDL's log into our logger
    SDL_LogSetOutputFunction(sdl_logger_callback, NULL);
    SDL_LogSetAllPriority(log_pge2sdl(LogWriter::m_logLevel));
#endif

    LogWriter::OpenLogFile();
}

void UpdateLogLevel(PGE_LogLevel::Level logLevel)
{
    if(LogWriter::m_logLevel != logLevel)
        LogWriter::m_logLevel = logLevel;

#ifndef THEXTECH_NO_SDL_CORE
    SDL_LogSetAllPriority(log_pge2sdl(logLevel));
#endif
}

void CloseLog()
{
#ifndef THEXTECH_NO_SDL_CORE
    // Remove SDL's log callbacks
    SDL_LogResetPriorities();
#endif

    LogWriter::CloseLog();
}

std::string getLogFilePath()
{
#ifdef NO_FILE_LOGGING
    return "<No log file: Logging was been disabled in this build>";
#else
    return LogWriter::m_logFilePath;
#endif
}

static inline void pLogGeneric(int level, const char *label, const char *format, va_list arg)
{
    if(LogWriter::m_enabledStdOut && LogWriter::m_enabledVerboseLogs)
        LoggerPrivate_pLogConsole(level, label, format, arg);

#ifndef NO_FILE_LOGGING
    if(LogWriter::m_logLevel >= level)
    {
        auto t = getRunTimeString();
        LoggerPrivate_pLogFile(level, label, t.c_str(), format, arg);
    }
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

void pLogDebug(const std::string &line)
{
    pLogDebug("%s", line.c_str());
}

void pLogWarning(const std::string &line)
{
    pLogWarning("%s", line.c_str());
}

void pLogCritical(const std::string &line)
{
    pLogCritical("%s", line.c_str());
}

void pLogInfo(const std::string &line)
{
    pLogInfo("%s", line.c_str());
}

void pLogFatal(const std::string &line)
{
    pLogFatal("%s", line.c_str());
}
