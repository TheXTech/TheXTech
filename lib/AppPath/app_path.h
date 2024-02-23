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

#ifndef APP_PATH_H
#define APP_PATH_H

#include <string>
#include <vector>

#if defined(__3DS__)
#   define APP_PATH_HAS_EXTRA_WORLDS
#endif


class AppPathManager
{
public:
    /*!
     * \brief Prepend a single custom assets directory to the search path
     * \param root Path to the assets directory to use
     *
     * This function must be called BEFORE calling the initAppPath()
     */
    static void addAssetsRoot(const std::string &root);

    /*!
     * \brief Custom user directory (will be used instead of default)
     * \param root Path to the writable user directory to use
     *
     * The custom user directory is prepended to the search path but has lower precedence than an added assets root.
     * Overrides a custom-set GameDirName
     *
     * This function must be called BEFORE calling the initAppPath()
     */
    static void setUserDirectory(const std::string &root);

    /*!
     * \brief Custom game directory name for default locations
     * \param dirName Name of directory name that will be used for game initialisation
     *
     * This function must be called BEFORE calling the initAppPath()
     */
    static void setGameDirName(const std::string &dirName);

    /*!
     * \brief Initialise all paths
     */
    static void initAppPath();

    /*!
     * \brief Check the user added assets root (so that it can be treated differently than other members of the search path)
     * \return root passed to addAssetsRoot()
     */
    static std::string userAddedAssetsRoot(); // Read-Only

    /*!
     * \brief Get a list of folders that should be searched for asset packs (each folder directly, and also each subdirectory in folder + "/assets"). Does not change after initAppPath() is called.
     * \return Vector of paths to the read-only assets directory, always ends with a slash
     */
    static std::vector<std::string> assetsSearchPath(); // Read-Only

    /*!
     * \brief Set the custom assets directory by ID and absolute path (will be used instead of default)
     * \param id Name of the asset pack (used to postfix user paths if not empty)
     * \param path Path to the assets directory to use
     *
     * This function should be called AFTER calling the initAppPath(), and it will update all other paths
     */
    static void setCurrentAssetPack(const std::string &id, const std::string &path);

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

#ifdef APP_PATH_HAS_EXTRA_WORLDS
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

    static void initString(std::string &text, const std::string& inValue, const std::string &defValue);

    //! Location for writable settings
    static std::string m_settingsPath;
    //! Location for writable gamesaves
    static std::string m_gamesavesPath;
    //! Location for writable user directory
    static std::string m_userPath;
    //! Screenshots output directory
    static std::string m_screenshotsPath;
    //! GIF recordings output directory
    static std::string m_gifrecordingsPath;
    //! Logs output directory
    static std::string m_logsPath;

    //! Location for read-only current asset pack
    static std::string m_currentAssetPackPath;
    //! Asset pack-specific postfix for some user directories (either "id/" or "")
    static std::string m_assetPackPostfix;

    //! Location for read-only custom assets root
    static std::string m_customAssetsRoot;
    //! Location for writable custom user directory
    static std::string m_customUserDirectory;
    //! Custom name for the game directory name at default locations
    static std::string m_customGameDirName;
    //! Is portable configuration active?
    static bool m_isPortable;
};

#endif // APP_PATH_H
