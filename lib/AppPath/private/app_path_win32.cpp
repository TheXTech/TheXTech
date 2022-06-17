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

#include <windows.h>
#include <winreg.h>
#include <algorithm> // std::replace from \\ into /

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_assert.h>
#include <SDL2/SDL_filesystem.h>

#include "app_path_private.h"

static std::string s_assetsRoot;
static std::string s_userDirectory;
static std::string s_applicationPath;


void AppPathP::initDefaultPaths(const std::string &userDirName)
{
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
    wchar_t pathW[MAX_PATH];
    DWORD path_len = GetEnvironmentVariableW(L"UserProfile", pathW, MAX_PATH);
    SDL_assert_release(path_len);
    s_userDirectory.resize(path_len * 2);
    path_len = WideCharToMultiByte(CP_UTF8, 0,
                                   pathW,       path_len,
                                   &s_userDirectory[0],    s_userDirectory.size(),
                                   0, FALSE);
    s_userDirectory.resize(path_len);

    if(s_userDirectory.empty())
        s_userDirectory = "./";
    else
        s_userDirectory += userDirName;

    s_assetsRoot = s_userDirectory;
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
{
    /* Run the FS synchronization (Implement this for Emscripten only) */
}
