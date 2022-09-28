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

#ifndef NO_FILE_LOGGING
//! Output file
static FILE* s_logout;
#endif // #ifndef NO_FILE_LOGGING

void LogWriter::OpenLogFile()
{
    if(m_enabled)
    {
#ifndef NO_FILE_LOGGING
        s_logout = std::fopen(m_logFilePath.c_str(), "a");
#endif // #ifndef NO_FILE_LOGGING
    }
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
    (void)level;
    (void)label;
    (void)format;
    (void)arg;
}

void LoggerPrivate_pLogFile(int level, const char *label, const char *format, va_list arg)
{
#ifndef NO_FILE_LOGGING
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
#endif
}
