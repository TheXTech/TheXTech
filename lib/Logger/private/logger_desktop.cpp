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
#include "sdl_proxy/sdl_rwops.h"

#include "logger_sets.h"
#include "logger_private.h"

#ifndef NO_FILE_LOGGING

#ifndef PGE_NO_THREADING
static std::mutex g_lockLocker;
#endif

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
    {
        try
        {
            fmt::print(stderr, "LogLevel {0}, log file {1}\n\n", static_cast<int>(m_logLevel), m_logFilePath);
            std::fflush(stderr);
        }
        catch(const fmt::FormatError &err)
        {
            std::fprintf(stderr, "fmt::print failed with exception: %s\n", err.what());
            std::fflush(stderr);
            abort();
        }
    }

    if(m_enabled)
    {
        s_logout = SDL_RWFromFile(m_logFilePath.c_str(), "a");
        if(!s_logout)
        {
            std::fprintf(stderr, "Impossible to open %s for write, log printing into the file is disabled!\n", m_logFilePath.c_str());
            std::fflush(stderr);
        }
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

void LoggerPrivate_pLogConsole(int level, const char *label, const char *format, va_list arg)
{
    va_list arg_in;
    (void)level;

    va_copy(arg_in, arg);
    std::fprintf(stderr, "%s: ", label);
    std::vfprintf(stderr, format, arg_in);
    std::fprintf(stderr, OS_NEWLINE);
    std::fflush(stderr);
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
}
#endif // NO_FILE_LOGGING
