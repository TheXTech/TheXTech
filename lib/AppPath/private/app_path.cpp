/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_filesystem.h>

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <IniProcessor/ini_processing.h>
#define FMT_NOEXCEPT
#include <fmt/fmt_format.h>

#include "../app_path.h"
#include "../../version.h"
#include "app_path_private.h"


std::string AppPathManager::m_settingsPath;
std::string AppPathManager::m_assetsPath;
std::string AppPathManager::m_userPath;

std::string AppPathManager::m_screenshotsPath;
std::string AppPathManager::m_gifrecordingsPath;
std::string AppPathManager::m_logsPath;

std::string AppPathManager::m_customAssetsRoot;
std::string AppPathManager::m_customUserDirectory;

bool AppPathManager::m_isPortable = false;


#if defined(USER_DIR_NAME)
#   define UserDirName "/" USER_DIR_NAME
#elif defined(__ANDROID__) || defined(__APPLE__) || defined(__HAIKU__)
#   define UserDirName "/PGE Project/thextech/"
#else
#   define UserDirName "/.PGE_Project/thextech/"
#endif

static void appendSlash(std::string &path)
{
    if(!path.empty() && path.back() != '/')
        path.push_back('/');
}

void AppPathManager::setAssetsRoot(const std::string &root)
{
    m_customAssetsRoot = root;
    appendSlash(m_customAssetsRoot);
}

void AppPathManager::setUserDirectory(const std::string& root)
{
    m_customUserDirectory = root;
    appendSlash(m_customUserDirectory);
}

void AppPathManager::initAppPath()
{
    AppPathP::initDefaultPaths(UserDirName);

    // When user directory is redefined externally
    if(!m_customUserDirectory.empty())
    {
        m_userPath = m_customUserDirectory;
        // Assets root matches to user directory if not specified other
        m_assetsPath = m_customAssetsRoot.empty() ? m_userPath : m_customAssetsRoot;
        initSettingsPath();
        return;
    }

    if(AppPathP::portableAvailable() && checkPortable())
        return;

#if defined(FIXED_ASSETS_PATH) // Fixed assets path, for the rest of UNIX-like OS packages
    m_assetsPath = FIXED_ASSETS_PATH;
#else
    m_assetsPath = m_customAssetsRoot.empty() ? AppPathP::assetsRoot() : m_customAssetsRoot;
#endif
    appendSlash(m_assetsPath);

    std::string userDirPath = AppPathP::userDirectory();

    if(userDirPath.empty())
        goto defaultSettingsPath;
    else
    {
        DirMan appDir(userDirPath);
        if(!appDir.exists() && !appDir.mkpath())
            goto defaultSettingsPath;

        m_userPath = appDir.absolutePath();
    }

    appendSlash(m_userPath);

    initSettingsPath();

    return;

defaultSettingsPath:
    m_userPath = AppPathP::appDirectory();
    m_assetsPath = AppPathP::appDirectory();
    initSettingsPath();
#if defined(__EMSCRIPTEN__) && !defined(DISABLE_LOGGING)
    std::printf("== App Path is %s\n", ApplicationPathSTD.c_str());
    std::printf("== User Path is %s\n", m_userPath.c_str());
    fflush(stdout);
#endif
}

bool AppPathManager::checkPortable()
{
    std::string appPath = AppPathP::appDirectory();

    if(appPath.empty())
        return false;

    if(m_settingsPath.empty())
        m_settingsPath = appPath;

    if(m_userPath.empty())
        m_userPath = appPath;

    if(!Files::fileExists(settingsFileSTD()))
        return false;

    m_isPortable = false;

    IniProcessing checkForPort(settingsFileSTD());
    checkForPort.beginGroup("Main");
    m_isPortable = checkForPort.value("force-portable", false).toBool();
    checkForPort.endGroup();

    if(m_isPortable)
    {
        m_assetsPath = appPath;
        initSettingsPath();
    }

    return m_isPortable;
}

void AppPathManager::initSettingsPath()
{
    // Default settings path
    m_settingsPath = m_userPath + "settings/";

    // Check if need to use system-wide screenshots directory
    m_screenshotsPath = AppPathP::screenshotsRoot();
    if(m_screenshotsPath.empty())
        m_screenshotsPath = m_userPath + "screenshots/";

    // Check if need to use system-wide gif recording directory
    m_gifrecordingsPath = AppPathP::gifRecsRoot();
    if(m_gifrecordingsPath.empty())
        m_gifrecordingsPath = m_userPath + "gif-recordings/";

    // Check if need to use system-wide logs directory
    m_logsPath = AppPathP::logsRoot();
    if(m_logsPath.empty())
        m_logsPath = m_userPath + "logs/";

    // Just in case, avoid mad jokes with making name-sake file as a settings folder
    if(Files::fileExists(m_settingsPath))
        Files::deleteFile(m_settingsPath);

    // Create the settings directory
    if(!DirMan::exists(m_settingsPath))
        DirMan::mkAbsPath(m_settingsPath);

    // Also create the game saves root folder to be exist
    if(!DirMan::exists(gameSaveRootDir()))
        DirMan::mkAbsPath(gameSaveRootDir());

    // And create empty worlds and battle directories too, make a hint for user
    if(!DirMan::exists(userWorldsRootDir()))
        DirMan::mkAbsPath(userWorldsRootDir());
    if(!DirMan::exists(userBattleRootDir()))
        DirMan::mkAbsPath(userBattleRootDir());
}


std::string AppPathManager::settingsFileSTD() // Writable
{
    return m_settingsPath + "thextech.ini";
}

std::string AppPathManager::settingsRoot() // Writable
{
    return m_settingsPath;
}

std::string AppPathManager::settingsControlsFileSTD() // Writable
{
    return m_settingsPath + "controls.ini";
}

std::string AppPathManager::userAppDirSTD() // Writable
{
    return m_userPath;
}

std::string AppPathManager::assetsRoot() // Readable
{
    return m_assetsPath;
}

std::string AppPathManager::logsDir() // Writable
{
    return m_logsPath;
}

std::string AppPathManager::languagesDir() // Readable
{
    return m_assetsPath + "languages/";
}

std::string AppPathManager::screenshotsDir() // Writable
{
    return m_screenshotsPath;
}

std::string AppPathManager::gifRecordsDir() // Writable
{
    return m_gifrecordingsPath;
}

std::string AppPathManager::gameSaveRootDir() // Writable
{
    return m_settingsPath + "gamesaves/";
}

std::string AppPathManager::gameplayRecordsRootDir() // Writable
{
    return m_userPath + "gameplay-records/";
}

std::string AppPathManager::userWorldsRootDir() // Readable
{
    return m_userPath + "worlds/";
}

std::string AppPathManager::userBattleRootDir() // Readable
{
    return m_userPath + "battle/";
}

void AppPathManager::install()
{
    std::string path = AppPathP::userDirectory();

    if(!path.empty())
    {
        DirMan appDir(path);
        if(!appDir.exists())
            appDir.mkpath(path);
    }
}

bool AppPathManager::userDirIsAvailable()
{
    return (m_userPath != m_assetsPath);
}

void AppPathManager::syncFs()
{
    AppPathP::syncFS();
}
