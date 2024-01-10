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

#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_timer.h"

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


void LoadLogSettings(bool disableStdOut, bool verboseLogs)
{
#if defined(DEBUG_BUILD) || defined(__WIIU__)
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

    LogWriter::OpenLogFile();
}

void CloseLog()
{
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
