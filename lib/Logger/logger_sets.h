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

#ifndef LOGGER_SETS_H
#define LOGGER_SETS_H

#include <SDL2/SDL_rwops.h>
#include <string>
#include <memory>

#include "logger.h"

class LogWriter
{
public:
    static std::string  m_logDirPath;
    static std::string  m_logFilePath;
    static PGE_LogLevel m_logLevel;
    static int          m_maxFilesCount;
    //! Is logging system is enabled
    static bool  m_enabled;
    //! Is logging system allowed to output into `stdout`
    static bool  m_enabledStdOut;
    //! Verbose logs to stdOut if possible
    static bool  m_enabledVerboseLogs;
    //! Is log file is opened
    static bool  m_logIsOpened;
    //! Output file
    static SDL_RWops *m_logout;

    static void WriteToLog(PGE_LogLevel type, const std::string &msg);
    static void LoadLogSettings(bool disableStdOut = false, bool verboseLogs = false);
};

#endif // LOGGER_SETS_H
