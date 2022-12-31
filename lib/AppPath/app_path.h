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

#ifndef APP_PATH_H
#define APP_PATH_H

#include <string>
#include <vector>

class AppPathManager
{
public:
    /*!
     * \brief Set the custom assets directory (will be used instead of default)
     * \param root Path to the assets directory to use
     *
     * This function must be called BEFORE calling the initAppPath()
     */
    static void setAssetsRoot(const std::string &root);

    /*!
     * \brief Custom user directory (will be used instead of default)
     * \param root Path to the writable user directory to use
     *
     * This function also sets the assets root in condition if a different one
     * hasn't specified.
     *
     * This function must be called BEFORE calling the initAppPath()
     */
    static void setUserDirectory(const std::string &root);

    /*!
     * \brief Initialise all paths
     */
    static void initAppPath();

    /*!
     * \brief Get the path to the game settings file
     * \return Path to the game settings file
     */
    static std::string settingsFileSTD(); // Must be writable

    /*!
     * \brief Get the settings directory path
     * \return Settings directory path, always ends with a slash
     */
    static std::string settingsRoot(); // Must be writable

    /*!
     * \brief Get the path to the controls settings file
     * \return Path to the controls settings file
     */
    static std::string settingsControlsFileSTD(); // Must be writable

    /*!
     * \brief Get the path to the writable user directory
     * \return Path to ther writable user directory, always ends with a slash
     */
    static std::string userAppDirSTD(); // Must be writable

    /*!
     * \brief Get the path to the read-only assets directory
     * \return Path to ther read-only assets directory, always ends with a slash
     */
    static std::string assetsRoot(); // Read-Only

    /*!
     * \brief Get the path to the logs output directory
     * \return Path to ther logs output directory, always ends with a slash
     */
    static std::string logsDir(); // Must be writable

    /*!
     * \brief Get the path to engine languages directory
     * \return Path to ther engine languages directory, always ends with a slash
     */
    static std::string languagesDir(); // Read-Only

    /*!
     * \brief Get the path to the screenshots output directory
     * \return Path to ther screenshots output directory, always ends with a slash
     */
    static std::string screenshotsDir(); // Must be writable

    /*!
     * \brief Get the path to the GIF recordings output directory
     * \return Path to ther GIF recordings output directory, always ends with a slash
     */
    static std::string gifRecordsDir(); // Must be writable

    /*!
     * \brief Get the path to the game saves directory
     * \return Path to ther game saves directory, always ends with a slash
     */
    static std::string gameSaveRootDir(); // Must be writable

    static std::string gameplayRecordsRootDir(); // Must be writable

    static std::string userWorldsRootDir(); // Read-Only, appears at writable directory

    static std::string userBattleRootDir(); // Read-Only, appears at writable directory

#ifdef __3DS__
    static const std::vector<std::string>& worldRootDirs(); // Read-Only, appears at writable directory
#endif

    static void install();
    static bool userDirIsAvailable();

    /*!
     * \brief Emscripten-only file system synchronization request
     */
    static void syncFs();

private:
    static bool checkPortable();
    /**
     * @brief Makes settings path if not exists
     */
    static void initSettingsPath();

    //! Location for writable settings and game-saves
    static std::string m_settingsPath;
    //! Location for writable user directory
    static std::string m_userPath;
    //! Location for read-only assets directory
    static std::string m_assetsPath;
    //! Screenshots output directory
    static std::string m_screenshotsPath;
    //! GIF recordings output directory
    static std::string m_gifrecordingsPath;
    //! Logs output directory
    static std::string m_logsPath;

    //! Location for read-only custom assets root
    static std::string m_customAssetsRoot;
    //! Location for writable custom user directory
    static std::string m_customUserDirectory;
    //! Is portable configuration active?
    static bool m_isPortable;
};

#endif // APP_PATH_H
