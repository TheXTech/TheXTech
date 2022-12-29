/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#include <cstdio>

#ifdef DEBUG_BUILD
#   include <debugnet.h>
#   ifndef NETDBG_IP_SERVER
#       define NETDBG_IP_SERVER "192.168.1.183"
#   endif
#   ifndef NETDBG_PORT_SERVER
#       define NETDBG_PORT_SERVER 18194
#   endif

#   define VITA_TEMP_BUFFER_SIZE (1024 * 1024)

static char __string_buffer[VITA_TEMP_BUFFER_SIZE];
static char __string_buffer2[VITA_TEMP_BUFFER_SIZE];
static int __vita_debug_setup = 0;
#endif

#ifndef NO_FILE_LOGGING
//! Output file
static FILE* s_logout;
#endif // #ifndef NO_FILE_LOGGING

void LogWriter::OpenLogFile()
{
#ifndef NO_FILE_LOGGING
    if(m_enabled)
        s_logout = std::fopen(m_logFilePath.c_str(), "a");
#endif // #ifndef NO_FILE_LOGGING
}

void LogWriter::CloseLog()
{
#ifndef NO_FILE_LOGGING
    if(s_logout)
        std::fclose(s_logout);
    s_logout = nullptr;
#endif // #ifndef NO_FILE_LOGGING
}

void LoggerPrivate_pLogConsole(int level, const char *label, const char *format, va_list arg)
{
#ifdef DEBUG_BUILD
    (void)level;
    (void)label;

    if(__vita_debug_setup == 0)
        debugNetInit(NETDBG_IP_SERVER, NETDBG_PORT_SERVER, DEBUG);

    // Print arg list to first string buffer.
    vsnprintf(__string_buffer, VITA_TEMP_BUFFER_SIZE - 4, format, arg);
    // Print that string buffer into second string buffer with new line & null termination.
    snprintf(__string_buffer2, VITA_TEMP_BUFFER_SIZE, "%s\n", __string_buffer);
    
    // Print to network.
    debugNetPrintf(DEBUG, __string_buffer2);
#else
    (void)level;
    (void)label;
    (void)format;
    (void)arg;
#endif
}

#ifndef NO_FILE_LOGGING
void LoggerPrivate_pLogFile(int level, const char *label, const char *format, va_list arg)
{
    if(!s_logout)
        return;

    va_list arg_in;
    (void)level;

    va_copy(arg_in, arg);
    std::fprintf(s_logout, "%s: ", label);
    std::vfprintf(s_logout, format, arg_in);
    std::fprintf(s_logout, OS_NEWLINE);
    std::fflush(s_logout);
    va_end(arg_in);
}
#endif
