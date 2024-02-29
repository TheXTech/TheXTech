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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <DirManager/dirman.h>

#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_filesystem.h"

#include "app_path_private.h"

static std::string s_assetsRoot;
static std::string s_userDirectory;

static std::string s_logsDirectory;
static std::string s_settingsDirectory;
static std::string s_gamesavesDirectory;

static std::string s_applicationPath;
//! The legacy debug root
static const char* s_legacyDebugDir = "/.PGE_Project/thextech/";

#ifdef THEXTECH_NEW_USER_PATHS
#   ifndef THEXTECH_SYSTEM_GAMES_DIR
static const char* s_gamesSysDir = "/usr/share/games";
#   else
static const char* s_gamesSysDir = THEXTECH_SYSTEM_GAMES_DIR;
#   endif
#endif

#if defined(THEXTECH_NEW_USER_PATHS)
static std::string s_getEnvNotNull(const char *env)
{
    const char *e = SDL_getenv(env);
    if(e)
        return std::string(e);
    else
        return std::string();
}
#endif

void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    std::string homePath;
#if defined(THEXTECH_NEW_USER_PATHS)
    std::string userDir;
    std::string logsDir;
    std::string setupDir;
#endif

    // Environment
    const char *env_home = SDL_getenv("HOME");
#if defined(THEXTECH_NEW_USER_PATHS)
    setupDir = s_getEnvNotNull("XDG_CONFIG_HOME");
    logsDir = s_getEnvNotNull("XDG_STATE_HOME");
    userDir = s_getEnvNotNull("XDG_DATA_HOME");
#endif

    // Init home directory
#if defined(__HAIKU__)
    if(env_home)
        homePath.append(env_home);
#else
    passwd *pw = getpwuid(getuid());
    if(pw)
        homePath.append(pw->pw_dir);
    else if(env_home)
        homePath.append(env_home);
#endif

#if defined(THEXTECH_NEW_USER_PATHS)
    // Set default paths if environments aren't defined
    if(!homePath.empty())
    {
        if(setupDir.empty())
            setupDir = homePath + "/.config";

        if(logsDir.empty())
            logsDir = homePath + "/.local/state";

        if(userDir.empty())
            userDir = homePath + "/.local/share";
    }
#endif

    if(homePath.empty())
        homePath = std::string(".");

#if defined(THEXTECH_NEW_USER_PATHS)

    bool legacyRoot = DirMan::exists(homePath + s_legacyDebugDir);
    bool legacyRoot2 = DirMan::exists(homePath + userDirName);

    if(!ignoreLegacyDebugDir && (legacyRoot || legacyRoot2)) // Legacy debug root has the highest priority!
    {
        if(legacyRoot2)
            homePath.append(userDirName);
        else
            homePath.append(s_legacyDebugDir);

        if(!homePath.empty() && homePath.back() != '/')
            homePath.push_back('/');

        s_userDirectory = homePath;
        s_assetsRoot.clear();
        s_logsDirectory.clear();
        s_settingsDirectory.clear();
        s_gamesavesDirectory.clear();
    }
    else
    {
        s_userDirectory = userDir + userDirName + "userdata/";
        s_assetsRoot = userDir + userDirName + "debug-assets/";
        s_logsDirectory = logsDir + userDirName;
        s_settingsDirectory = setupDir + userDirName;
        s_gamesavesDirectory = userDir + userDirName + "gamesaves/";
        // If debug assets are not exists, find the globally installed assets instead
        if(!DirMan::exists(s_assetsRoot))
            s_assetsRoot = s_gamesSysDir + userDirName;
    }

#else // THEXTECH_NEW_USER_PATHS
    // Use old default paths logic
    if(DirMan::exists(homePath + userDirName))
        homePath.append(userDirName);
    else
        homePath.append(s_legacyDebugDir);

    if(!homePath.empty() && homePath.back() != '/')
        homePath.push_back('/');

    s_userDirectory = homePath;
    s_assetsRoot.clear();
    s_logsDirectory.clear();
    s_settingsDirectory.clear();
    s_gamesavesDirectory.clear();
#endif // THEXTECH_NEW_USER_PATHS

    char *appPath = SDL_GetBasePath();
    if(!appPath)
    {
#ifndef DISABLE_LOGGING
        std::fprintf(stderr, "== Failed to recognize application path by using of SDL_GetBasePath! Using current working directory \"./\" instead.\n");
        std::fflush(stderr);
#endif
        appPath = SDL_strdup("./");
    }

    s_applicationPath = std::string(appPath);
    SDL_free(appPath);
}

std::string AppPathP::appDirectory()
{
    return s_applicationPath;
}

std::string AppPathP::userDirectory()
{
    return s_userDirectory;
}

std::string AppPathP::assetsRoot()
{
    return s_assetsRoot;
}

std::string AppPathP::settingsRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide settings
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. settings saved at the user directory)
     */
    return s_settingsDirectory;
}

std::string AppPathP::gamesavesRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide gamesaves
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. gamesaves saved at the settings directory)
     */
    return s_gamesavesDirectory;
}

std::string AppPathP::screenshotsRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide screenshots
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. screenshots saved at the user directory)
     */
    return std::string();
}

std::string AppPathP::gifRecsRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide GIF recordings
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. GIF recordings saved at the user directory)
     */
    return std::string();
}

std::string AppPathP::logsRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide logs
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. logs saved at the user directory)
     */
    return s_logsDirectory;
}

bool AppPathP::portableAvailable()
{
    /*
     * Report, does this platfor support portable mode or not
     */
    return true;
}

void AppPathP::syncFS()
{}
