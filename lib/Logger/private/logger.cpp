/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2022 Vitaly Novichkov <admin@wohlnet.ru>
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
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <DirManager/dirman.h>
#include <Utils/files.h>
#endif
#include <IniProcessor/ini_processing.h>
#include <AppPath/app_path.h>

#include <fmt_format_ne.h>
#include <fmt/fmt_printf.h>

#include "core/std.h"


std::string     LogWriter::m_logDirPath;
std::string     LogWriter::m_logFilePath;
PGE_LogLevel::Level LogWriter::m_logLevel = PGE_LogLevel::NoLog;
int             LogWriter::m_maxFilesCount = 10;
bool            LogWriter::m_enabled = false;
bool            LogWriter::m_enabledStdOut = false;
bool            LogWriter::m_enabledVerboseLogs = false;


#if !defined(NO_FILE_LOGGING)
static std::string return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

#ifdef PGE_MIN_PORT
    // chrono doesn't work reliably here
    time( &in_time_t );
#endif

    char out[24];
    XStd::memset(out, 0, sizeof(out));
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
        if(XStd::strncasecmp(s.c_str(), "TheXTech_log_", 13) == 0)
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
#endif


void LoadLogSettings(bool disableStdOut, bool verboseLogs)
{
#   if defined(DEBUG_BUILD)
    (void)verboseLogs; // unused
    LogWriter::m_enabledVerboseLogs = true; // Enforce verbose log for debug builds
#   else
    LogWriter::m_enabledVerboseLogs = verboseLogs;
#endif

    LogWriter::m_enabledStdOut = !disableStdOut;
    LogWriter::m_logLevel = PGE_LogLevel::Debug;

#if !defined(NO_FILE_LOGGING)
    std::string logFileName = fmt::format_ne("TheXTech_log_{0}.txt", return_current_time_and_date());
#endif

    std::string mainIniFile = AppPathManager::settingsFileSTD();
    IniProcessing logSettings(mainIniFile);

#if !defined(NO_FILE_LOGGING)
    std::string logPath = AppPathManager::logsDir();
    DirMan defLogDir(logPath);

    if(!defLogDir.exists())
    {
        if(!defLogDir.mkpath())
            defLogDir.setPath(AppPathManager::userAppDirSTD());
    }
#endif

    logSettings.beginGroup("logging");
    {
#if !defined(NO_FILE_LOGGING)
        logSettings.read("log-path", LogWriter::m_logDirPath, defLogDir.absolutePath());
        logSettings.read("max-log-count", LogWriter::m_maxFilesCount, 10);

        if(!DirMan::exists(LogWriter::m_logDirPath))
            LogWriter::m_logDirPath = defLogDir.absolutePath();
#endif

        IniProcessing::StrEnumMap logLevelEnum =
        {
            {"0", PGE_LogLevel::NoLog},
            {"1", PGE_LogLevel::Fatal},
            {"2", PGE_LogLevel::Info},
            {"3", PGE_LogLevel::Critical},
            {"4", PGE_LogLevel::Warning},
            {"5", PGE_LogLevel::Debug},
            {"disabled", PGE_LogLevel::NoLog},
            {"fatal",    PGE_LogLevel::Fatal},
            {"info",     PGE_LogLevel::Info},
            {"critical", PGE_LogLevel::Critical},
            {"warning",  PGE_LogLevel::Warning},
            {"debug",    PGE_LogLevel::Debug}
        };

        logSettings.readEnum("log-level", LogWriter::m_logLevel, PGE_LogLevel::Debug, logLevelEnum);
        LogWriter::m_enabled   = (LogWriter::m_logLevel != PGE_LogLevel::NoLog);
    }
    logSettings.endGroup();

#if !defined(NO_FILE_LOGGING)
    LogWriter::m_logFilePath = LogWriter::m_logDirPath + "/" + logFileName;
    cleanUpLogs(LogWriter::m_logDirPath, LogWriter::m_maxFilesCount);
#else
    LogWriter::m_logFilePath.clear();
#endif

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
        LoggerPrivate_pLogFile(level, label, format, arg);
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
