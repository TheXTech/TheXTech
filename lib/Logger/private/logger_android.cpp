/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#define LOGGER_INTERNAL
#include "logger_sets.h"
#include "logger_private.h"

#include <mutex>
#include "sdl_proxy/sdl_rwops.h"

#include <android/log.h>

#ifndef NO_FILE_LOGGING
static std::mutex g_lockLocker;
#   define OUT_BUFFER_SIZE 10240
static char       g_outputBuffer[OUT_BUFFER_SIZE];
#   define OUT_BUFFER_STRING_SIZE 10239
//! Output file
static SDL_RWops *s_logout = nullptr;
#endif // NO_FILE_LOGGING

void LogWriter::OpenLogFile()
{
#ifndef NO_FILE_LOGGING
    MUTEXLOCK(mutex);

    if(LogWriter::m_enabledStdOut)
        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "LogLevel %d, log file %s\n\n", m_logLevel, m_logFilePath.c_str());

    if(m_enabled)
    {
        s_logout = SDL_RWFromFile(m_logFilePath.c_str(), "a");
        if(!s_logout)
            __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "Impossible to open %s for write, log printing into the file is disabled!\n", m_logFilePath.c_str());
    }
#endif // NO_FILE_LOGGING
}

void LogWriter::CloseLog()
{
#ifndef NO_FILE_LOGGING
    MUTEXLOCK(mutex);
    if(s_logout)
        SDL_RWclose(s_logout);
    s_logout = nullptr;
#endif // NO_FILE_LOGGING
}

static int pgeToAndroidLL(int level)
{
    switch(level)
    {
    case PGE_LogLevel::NoLog:
        return ANDROID_LOG_INFO;
    case PGE_LogLevel::Fatal:
        return ANDROID_LOG_FATAL;
    default:
    case PGE_LogLevel::Info:
        return ANDROID_LOG_INFO;
    case PGE_LogLevel::Critical:
        return ANDROID_LOG_ERROR;
    case PGE_LogLevel::Warning:
        return ANDROID_LOG_WARN;
    case PGE_LogLevel::Debug:
        return ANDROID_LOG_DEBUG;
    }
}

void LoggerPrivate_pLogConsole(int level, const char *label, const char *format, va_list arg)
{
    (void)label;
    va_list arg_in;
    va_copy(arg_in, arg);
    __android_log_vprint(pgeToAndroidLL(level), "TRACKERS", format, arg_in);
    va_end(arg_in);
}

#ifndef NO_FILE_LOGGING
void LoggerPrivate_pLogFile(int level, const char *label, const char *format, va_list arg)
{
    va_list arg_in;
    (void)level;

    if(!s_logout)
        return;

    MUTEXLOCK(mutex);

    va_copy(arg_in, arg);

    int len = SDL_snprintf(g_outputBuffer, OUT_BUFFER_SIZE, "%s: ", label);
    if(len > 0)
        SDL_RWwrite(s_logout, g_outputBuffer, 1, (size_t)(len < OUT_BUFFER_STRING_SIZE ? len : OUT_BUFFER_STRING_SIZE));

    len = SDL_vsnprintf(g_outputBuffer, OUT_BUFFER_SIZE, format, arg_in);
    if(len > 0)
        SDL_RWwrite(s_logout, g_outputBuffer, 1, (size_t)(len < OUT_BUFFER_STRING_SIZE ? len : OUT_BUFFER_STRING_SIZE));

    SDL_RWwrite(s_logout, reinterpret_cast<const void *>(OS_NEWLINE), 1, OS_NEWLINE_LEN);
    va_end(arg_in);

/* WORKAROUNDS: flush the output of SDL RWops */
#ifdef HAVE_STDIO_H
    if(s_logout->hidden.stdio.fp)
        fflush(s_logout->hidden.stdio.fp);
#endif
}
#endif // NO_FILE_LOGGING
