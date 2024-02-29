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

#include <windows.h>
#include <winreg.h>
#include <shlobj.h>
#include <algorithm> // std::replace from \\ into /

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_assert.h>
#include <SDL2/SDL_filesystem.h>

#include <DirManager/dirman.h>

#include "app_path_private.h"

static std::string s_assetsRoot;
static std::string s_userDirectory;
static std::string s_logsDirectory;
static std::string s_applicationPath;
//! The legacy debug root
static const char* s_legacyDebugDir = "/.PGE_Project/thextech/";

static void s_toUtf8(std::string &ret, wchar_t *str, DWORD len)
{
    int outLen;
    ret.clear();
    ret.resize(len * 4); // 4x size to ensure size is enough
    outLen = WideCharToMultiByte(CP_UTF8, 0,
                                 str,       len,
                                 &ret[0],   (int)ret.size(),
                                 0, FALSE);
    ret.resize(outLen);

    // Convert slashes
    for(auto &c : ret)
    {
        if(c == '\\')
            c = '/';
    }
}

void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    // FIXME: Implement new default directories format later
    (void)userDirName;
    wchar_t pathBuffer[MAX_PATH] = L"";

#if defined(THEXTECH_NEW_USER_PATHS)
    std::string localPath;
    std::string roamingPath;

    if(FAILED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, pathBuffer)))
    {
#   ifndef DISABLE_LOGGING
        std::fprintf(stderr, "== Failed to retrieve the common AppData\n");
        std::fflush(stderr);
#   endif
    }
    else
        s_toUtf8(roamingPath, pathBuffer, (DWORD)SDL_wcslen(pathBuffer));

    if(FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathBuffer)))
    {
#   ifndef DISABLE_LOGGING
        std::fprintf(stderr, "== Failed to retrieve the common AppData\n");
        std::fflush(stderr);
#   endif
    }
    else
        s_toUtf8(localPath, pathBuffer, (DWORD)SDL_wcslen(pathBuffer));
#endif // THEXTECH_NEW_USER_PATHS

    // Application path
    char *path = SDL_GetBasePath();
    if(!path)
    {
#   ifndef DISABLE_LOGGING
        std::fprintf(stderr, "== Failed to recognize application path by using of SDL_GetBasePath! Using current working directory \"./\" instead.\n");
        std::fflush(stderr);
#   endif
        path = SDL_strdup("./");
    }

    s_applicationPath = std::string(path);
    std::replace(s_applicationPath.begin(), s_applicationPath.end(), '\\', '/');
    SDL_free(path);

    // User directory
    DWORD path_len = GetEnvironmentVariableW(L"UserProfile", pathBuffer, MAX_PATH);
    SDL_assert_release(path_len);
    s_toUtf8(s_userDirectory, pathBuffer, path_len);

#if defined(THEXTECH_NEW_USER_PATHS)
    if(s_userDirectory.empty() && localPath.empty() && roamingPath.empty())
        s_userDirectory = "./";
    else
    {
        // Priority the legacy path for compatibility
        if(!ignoreLegacyDebugDir && DirMan::exists(s_userDirectory + s_legacyDebugDir))
        {
            s_userDirectory += s_legacyDebugDir;
            s_assetsRoot.clear();
            s_logsDirectory.clear();
        }
        else
        {
            s_userDirectory = roamingPath + userDirName + "UserData/";
            s_assetsRoot = roamingPath + userDirName + "DebugAssets/";
            s_logsDirectory = localPath + userDirName + "logs/";
        }
    }
#else
    s_userDirectory += s_legacyDebugDir;
    s_assetsRoot.clear();
    s_logsDirectory.clear();
#endif
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
{
    /* Run the FS synchronization (Implement this for Emscripten only) */
}
