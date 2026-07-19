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
#include <mutex>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_error.h>
#include <DirManager/dirman.h>
#include <Foundation/Foundation.h>
#include <PGE_File_Formats/pge_file_lib_private.h> // For URL-Decode.

#include "app_path_private.h"


static std::string s_assetsRoot;
static std::string s_worldRootDir;
static std::mutex s_worldRootDirMutex;
static std::vector<std::string> s_worldRootDirs;
static std::string s_worldRootDirLastId;
static std::string s_userDirectory;
static std::string s_applicationPath;
static std::string s_screenshotsPath;
static std::string s_gifRecordPath;
//! The name of application bundle to be re-used as the user directory name
static std::string s_bundleName;


void AppPathP::initDefaultPaths(const std::string &)
{
    std::string worlds_list;

    // Assets directory
    {
        NSString *path_b = [NSBundle mainBundle].bundlePath;
        const char *path_s = path_b.UTF8String;
        std::string path = std::string(path_s) + "/";

        s_applicationPath = path;
        s_assetsRoot = path + "assets/";
        s_worldRootDir = path + "worlds/";
    }

    // Get caches directory
    {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSString *str;
        const char *base;
        char *retval = NULL;
        NSArray<NSString *> *array = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);

        if([array count] > 0) /* we only want the first item in the list. */
        {
            str = [array objectAtIndex:0];
            base = [str fileSystemRepresentation];

            if(base)
            {
                const size_t len = SDL_strlen(base) + 4;
                retval = (char *)SDL_malloc(len);

                if(retval == NULL)
                    SDL_OutOfMemory();
                else
                    SDL_snprintf(retval, len, "%s", base);
            }
        }

        [pool drain];

        s_userDirectory = std::string(retval);
        SDL_free(retval);
    }

    // Initialize the user directory
    DirMan::mkAbsDir(s_userDirectory);

    // Screenshots directory (!!!WILL BE STORED AT CACHES!!!)
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

const std::vector<std::string>& AppPathManager::worldRootDirs() // Read-Only, appears at writable directory
{
    if(s_worldRootDirLastId != m_assetPackPostfix)
    {
        s_worldRootDirMutex.lock();
        s_worldRootDirs.clear();
        s_worldRootDirs.push_back(s_worldRootDir + m_assetPackPostfix);
        s_worldRootDirLastId = m_assetPackPostfix;
        s_worldRootDirMutex.unlock();
    }

    return s_worldRootDirs;
}
