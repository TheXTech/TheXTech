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

#pragma once
#ifndef APP_PATH_PRIVATE_H
#define APP_PATH_PRIVATE_H

#include <string>

namespace AppPathP
{
/*!
 * \brief Initialize all default internals
 */
extern void initDefaultPaths(const std::string &userDirName);

/*!
 * \brief Get the path to the executable's directory (On some platforms may be empty, so, use user directory instead)
 * \return Path to the executable's directory
 */
extern std::string appDirectory();

/*!
 * \brief Get the default path to the writable user directory
 * \return Path to the user directory
 */
extern std::string userDirectory();

/*!
 * \brief Get the default path to the read-only assets root
 * \return Path to the assets root directory
 */
extern std::string assetsRoot();

/*!
 * \brief Default directory for screenshots (if empty, store at the user directory)
 * \return Path to the system-wide screenshots directory
 */
extern std::string screenshotsRoot();

/*!
 * \brief Default directory for GIF recordings (if empty, store at the usre directory)
 * \return Path to the system-wide GIF recording directory
 */
extern std::string gifRecsRoot();

/*!
 * \brief Default directory for logs (if empty, store at the usre directory)
 * \return Path to the system-wide logs directory
 */
extern std::string logsRoot();

/*!
 * \brief Reports does platform supports portable mode
 * \return true if Portable mode is supported
 *
 * When it returns false, the "thextech.ini" with "force-portable" flag will never been checked
 */
extern bool portableAvailable();

/*!
 * \brief Run the FS synchronization (Emscripten only)
 */
extern void syncFS();

}

#endif // APP_PATH_PRIVATE_H
