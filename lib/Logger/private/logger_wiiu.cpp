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

#define LOGGER_INTERNAL
#include "logger_sets.h"
#include "logger_private.h"
#include <fmt/fmt_printf.h>
#include <coreinit/debug.h>

#define VITA_TEMP_BUFFER_SIZE (1024 * 1024)

static char s_string_buffer[VITA_TEMP_BUFFER_SIZE - 3];

#ifdef DEBUG_BUILD
#include <stdio.h>
#include <string>

// #define ENABLE_NET_LOG

#   ifdef ENABLE_NET_LOG
#       include <unistd.h>
#       include <sys/socket.h>
#       include <arpa/inet.h>

#       ifndef NETDBG_IP_SERVER
#           define NETDBG_IP_SERVER "172.19.9.141"
#       endif
#       ifndef NETDBG_PORT_SERVER
#           define NETDBG_PORT_SERVER 18194
#       endif

static char s_string_buffer2[VITA_TEMP_BUFFER_SIZE];
static int s_wut_debug_setup = 0;
static int s_socket_desc = 0;
static struct sockaddr_in s_server;
#   endif // ENABLE_NET_LOG
#endif // DEBUG_BUILD

#ifndef NO_FILE_LOGGING
//! Output file
static FILE* s_logout;
#endif // #ifndef NO_FILE_LOGGING

#ifndef PGE_NO_THREADING
#   ifdef PGE_SDL_MUTEX
    static SDL_mutex *g_lockLocker = nullptr;
#   else
    static std::mutex g_lockLocker;
#   endif
#endif

void LogWriter::OpenLogFile()
{
#if !defined(PGE_NO_THREADING) && defined(PGE_SDL_MUTEX)
    OSReport("Debug: Creating the SDL Mutex");
    g_lockLocker = SDL_CreateMutex();
#endif

    if(m_enabled)
    {
#ifndef NO_FILE_LOGGING
        OSReport("Debug: Opening a log file for APPEND: %s\n", m_logFilePath.c_str());
        s_logout = std::fopen(m_logFilePath.c_str(), "a");
#endif // #ifndef NO_FILE_LOGGING
    }
}

void LogWriter::CloseLog()
{
#if !defined(PGE_NO_THREADING) && defined(PGE_SDL_MUTEX)
    OSReport("Debug: Destroying the SDL Mutex");
    SDL_DestroyMutex(g_lockLocker);
    g_lockLocker = nullptr;
#endif

#ifndef NO_FILE_LOGGING
    if(s_logout)
        std::fclose(s_logout);
    s_logout = nullptr;
#endif // #ifndef NO_FILE_LOGGING


#ifdef ENABLE_NET_LOG
    if(s_wut_debug_setup)
    {
        const char *msg = "---- Game disconnected ----\n";
        send(s_socket_desc, msg, strlen(msg) + 1, 0);
        usleep(2000);
        shutdown(s_socket_desc, 2);
        s_socket_desc = 0;
        s_wut_debug_setup = 0;
    }
#endif
}

void LoggerPrivate_pLogConsole(int level, const char *label, const char *format, va_list arg)
{
    va_list arg_in;
    (void)level;
    MUTEXLOCK(mutex);

#ifdef ENABLE_NET_LOG

    // Try to connect the netcat server (run as `nc -nklv 18194`)
    if(!s_wut_debug_setup && s_socket_desc == 0)
    {
        s_socket_desc = socket(AF_INET , SOCK_STREAM , 0);
        if(s_socket_desc != -1)
        {
            s_server.sin_addr.s_addr = inet_addr(NETDBG_IP_SERVER);
            s_server.sin_family = AF_INET;
            s_server.sin_port = htons(NETDBG_PORT_SERVER);

            if(connect(s_socket_desc , (struct sockaddr *)&s_server , sizeof(s_server)) >= 0)
                s_wut_debug_setup = 1;
            else
            {
                shutdown(s_socket_desc, 2);
                s_socket_desc = -1;
            }
        }
    }

    // If success, send log lines to it
    if(s_socket_desc > 0)
    {
        va_copy(arg_in, arg);
        // Print arg list to first string buffer.
        std::vsnprintf(s_string_buffer, VITA_TEMP_BUFFER_SIZE - 4, format, arg_in);
        va_end(arg_in);

        // Print that string buffer into second string buffer with new line & null termination.
        std::snprintf(s_string_buffer2, VITA_TEMP_BUFFER_SIZE, "%s: %s\n", label, s_string_buffer);

        send(s_socket_desc, s_string_buffer2, strlen(s_string_buffer2) + 1, 0);
    }
#else
    va_copy(arg_in, arg);
    // Print arg list to first string buffer.
    std::vsnprintf(s_string_buffer, VITA_TEMP_BUFFER_SIZE - 4, format, arg_in);
    va_end(arg_in);

    // Print that string buffer into the output with new line & null termination.
    OSReport("%s: %s\n", label, s_string_buffer);
#endif
}

#ifndef NO_FILE_LOGGING
void LoggerPrivate_pLogFile(int level, const char *label, const char *in_time, const char *format, va_list arg)
{
    MUTEXLOCK(mutex);
    if(!s_logout)
        return;

    va_list arg_in;
    (void)level;

    va_copy(arg_in, arg);
    std::fprintf(s_logout, "%s [%s]: ", in_time, label);
    std::vfprintf(s_logout, format, arg_in);
    std::fprintf(s_logout, OS_NEWLINE);
    std::fflush(s_logout);
    va_end(arg_in);

}
#endif
