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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <PGE_File_Formats/pge_file_lib_globs.h> // For URL-Decode.

#include <SDL2/SDL_stdinc.h>

#include <DirManager/dirman.h>

#include "app_path_macos_dirs.h"
#include "app_path_private.h"


static std::string s_assetsRoot;
static std::string s_userDirectory;
static std::string s_applicationPath;
static std::string s_screenshotsPath;
static std::string s_gifRecordPath;
//! The name of application bundle to be re-used as the user directory name
static std::string s_bundleName;


void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    // Initialize the application path
    {
        CFURLRef appUrlRef;
        appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFStringRef filePathRef = CFURLGetString(appUrlRef);
        char temporaryCString[PATH_MAX];
        bzero(temporaryCString, PATH_MAX);
        CFStringGetCString(filePathRef, temporaryCString, PATH_MAX, kCFStringEncodingUTF8);
        s_applicationPath = PGE_FileFormats_misc::url_decode(std::string(temporaryCString));
        {
            s_bundleName.clear();

            auto i = s_applicationPath.find_last_of(".app");

            auto j = s_applicationPath.find_last_of('/', i);
            if(j != std::string::npos)
            {
                s_bundleName = s_applicationPath.substr(j + 1);
                auto k = s_bundleName.find_last_of(".app") - 3;
                s_bundleName.erase(k, s_bundleName.size() - k);
            }

            i = s_applicationPath.find_last_of('/', i);

            s_applicationPath.erase(i, s_applicationPath.size() - i);
            if(s_applicationPath.compare(0, 7, "file://") == 0)
                s_applicationPath.erase(0, 7);

            if(!s_applicationPath.empty() && (s_applicationPath.back() != '/'))
                s_applicationPath.push_back('/');
        }
        CFRelease(appUrlRef);
    }

    // Assets directory
    s_assetsRoot.clear();

#if defined(USE_BUNDLED_ASSETS) // When its release game with assets shipped with a game
    {
        CFURLRef appUrlRef;
        appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("assets"), NULL, NULL);
        CFStringRef filePathRef = CFURLGetString(appUrlRef);
        char temporaryCString[PATH_MAX];
        bzero(temporaryCString, PATH_MAX);
        CFStringGetCString(filePathRef, temporaryCString, PATH_MAX, kCFStringEncodingUTF8);
        std::string path = PGE_FileFormats_misc::url_decode(std::string(temporaryCString));
        if(path.compare(0, 7, "file://") == 0)
            path.erase(0, 7);
        s_assetsRoot = path;
    }
#endif

    // Initialize the user directory
    {
        std::string appSupport;

        char *base_path = getAppSupportDir();
        if(base_path)
        {
            appSupport.append(base_path);
            SDL_free(base_path);
        }

        if(appSupport.empty())
            appSupport = std::string("/Library/Application Support/");

        appSupport += userDirName;

        if(!appSupport.empty() && appSupport.back() != '/')
            appSupport.push_back('/');

        const char *homeDir = SDL_getenv("HOME");
        if(homeDir)
        {
            // Current TheXTech distribution: per-bundle directories
            if(!s_assetsRoot.empty() && !s_bundleName.empty())
                s_userDirectory = std::string(homeDir) + "/TheXTech Games/" + s_bundleName;
            // Shared userdata directory
            else
            {
                s_userDirectory = std::string(homeDir) + userDirName;

                // fallback to legacy directory if there is no custom directory set
                std::string legacyUserDirectory = std::string(homeDir) + "/TheXTech Games/" + s_bundleName;
                std::string legacyAssetsDirectory = std::string(homeDir) + "/TheXTech Games/Debug Assets";
                if(userDirName == "TheXTech" && !DirMan::exists(s_userDirectory) && DirMan::exists(legacyUserDirectory))
                {
                    s_userDirectory = legacyUserDirectory;
                    s_assetsRoot = legacyAssetsDirectory;
                }
            }
        }
        else
            s_userDirectory = appSupport;
    }

    // Screenshots directory
    {
        std::string path = s_userDirectory;
        char *base_path = getScreenCaptureDir();
        if(base_path)
        {
            path = base_path;
            SDL_free(base_path);
        }

        s_screenshotsPath = path + "/TheXTech Game Screenshots/";
        s_gifRecordPath = path + "/TheXTech Game Screenshots/gif-recordings/";
    }
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
    return AssetsPathType::Legacy;
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
