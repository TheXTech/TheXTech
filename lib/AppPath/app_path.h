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

#ifndef APP_PATH_H
#define APP_PATH_H

#include <string>

extern std::string  ApplicationPathSTD;

class AppPathManager
{
public:
    static void initAppPath();

    static std::string settingsFileSTD(); // Must be writable
    static std::string settingsRoot(); // Must be writable
    static std::string settingsControlsFileSTD(); // Must be writable
    static std::string userAppDirSTD(); // Must be writable
    static std::string assetsRoot(); // Read-Only
    static void setAssetsRoot(const std::string &root);
    static std::string logsDir(); // Must be writable
    static std::string languagesDir(); // Read-Only
    static std::string screenshotsDir(); // Must be writable
    static std::string gifRecordsDir(); // Must be writable
    static std::string gameSaveRootDir(); // Must be writable
    static std::string gameplayRecordsRootDir(); // Must be writable
    static std::string userWorldsRootDir(); // Read-Only, appears at writable directory
    static std::string userBattleRootDir(); // Read-Only, appears at writable directory

    static void install();
    static bool checkPortable();
    static bool isPortable();
    static bool userDirIsAvailable();
#ifdef __EMSCRIPTEN__
    static void syncFs();
#endif

private:
    /**
     * @brief Makes settings path if not exists
     */
    static void initSettingsPath();
    //! Location for writable settings and game-saves
    static std::string m_settingsPath;
    //! Location for writable user directory
    static std::string m_userPath;
    //! Location for read-only custom assets root
    static std::string m_customAssetsRoot;
#ifdef __APPLE__
    static std::string m_userDataRoot; // A game media root at home directory
#endif
    static bool m_isPortable;
};

#endif // APP_PATH_H
