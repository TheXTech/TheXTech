/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

static std::string s_userDirectory;

static std::string s_gameInstallDirectory;

static std::string s_applicationPath;

//! The root for installed data (note: asset packs are placed at /usr/share/games/TheXTech/<pack-name> by default)
#ifdef __HAIKU__
    // FIXME: check that this Haiku-specific logic is correct
    static const char* s_gamesSysDir = "/boot/system/data/" THEXTECH_DIRECTORY_PREFIX "/";
#else
    static const char* s_gamesSysDir = "/usr/share/games/" THEXTECH_DIRECTORY_PREFIX "/";
#endif

static std::string s_getEnvNotNull(const char *env)
{
    const char *e = SDL_getenv(env);
    if(e)
        return std::string(e);
    else
        return std::string();
}

void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    std::string homePath;
    std::string userDir;

    // check for deployment as AppImage when looking for system-installed assets
    s_gameInstallDirectory = s_getEnvNotNull("APPDIR");
    s_gameInstallDirectory += s_gamesSysDir;

    // Environment
    const char *env_home = SDL_getenv("HOME");
    userDir = s_getEnvNotNull("XDG_DATA_HOME");

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

    // Set default paths if environments aren't defined
    if(homePath.empty())
        homePath = std::string(".");

    if(userDir.empty())
        userDir = homePath + "/.local/share";

    userDir += "/";

    // use modern user directory by default
    s_userDirectory = userDir + userDirName;

    // fallback to legacy directory if there is no custom directory set
    std::string legacyUserDirectory = homePath + "/.PGE_Project/thextech";
    if(userDirName == "TheXTech" && !DirMan::exists(s_userDirectory) && DirMan::exists(legacyUserDirectory))
        s_userDirectory = legacyUserDirectory;

    // find the application's own path
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
    return s_gameInstallDirectory;
}

AssetsPathType AppPathP::assetsRootType()
{
    return AssetsPathType::Multiple;
}

std::string AppPathP::settingsRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide settings
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. settings saved at the user directory)
     */
    return std::string();
}

std::string AppPathP::gamesavesRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide gamesaves
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. gamesaves saved at the settings directory)
     */
    return std::string();
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
    return std::string();
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
