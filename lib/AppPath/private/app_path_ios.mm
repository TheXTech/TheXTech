/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2026 Vitaly Novichkov <admin@wohlnet.ru>
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
#include <SDL2/SDL_stdinc.h>
#include <DirManager/dirman.h>
#include <Foundation/Foundation.h>
#include "app_path_private.h"


static std::string s_assetsRoot;
static std::string s_userDirectory;
static std::string s_applicationPath;
static std::string s_bundlePath;
static std::string s_screenshotsPath;
static std::string s_gifRecordPath;


void AppPathP::initDefaultPaths(const std::string &)
{
    char app_home[1024];

    // Bundle directory for local resources fetching
    {
        NSString *path_b = [NSBundle mainBundle].bundlePath;
        const char *path_s = path_b.UTF8String;
        std::string path = std::string(path_s) + "/";

        s_bundlePath = path;
    }

    // Initialize the application path
    strcpy(app_home, getenv("HOME"));
    s_applicationPath = std::string(app_home);

    if(!s_applicationPath.empty() && (s_applicationPath.back() != '/'))
        s_applicationPath.push_back('/');

    // Initialize the user directory
    s_userDirectory = s_applicationPath + "Documents/";
    DirMan::mkAbsDir(s_userDirectory);

    // Assets directory
    s_assetsRoot = s_userDirectory + "assets/";

    // Screenshots directory
    s_screenshotsPath = s_userDirectory + "screenshots/";
    s_gifRecordPath = s_userDirectory + "gif-recordings/";
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
    return s_screenshotsPath;
}

std::string AppPathP::gifRecsRoot()
{
    return s_gifRecordPath;
}

std::string AppPathP::logsRoot()
{
    return std::string();
}

bool AppPathP::portableAvailable()
{
    return false;
}

void AppPathP::syncFS()
{
    /* Run the FS synchronization (Implement this for Emscripten only) */
}

std::string AppPathManager::bundleResourcesPath()
{
    return s_bundlePath;
}
