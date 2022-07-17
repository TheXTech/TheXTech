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

#include <emscripten.h>

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_filesystem.h>

#include "app_path_private.h"


static std::string s_assetsRoot;
static std::string s_userDirectory;
static std::string s_applicationPath;

static bool loadingLocked = false;


extern "C" void unlockLoadingCustomState()
{
    loadingLocked = false;
}

static void loadCustomState()
{
    loadingLocked = true;
    EM_ASM(
        FS.mkdir('/settings');
        FS.mount(IDBFS, {}, '/settings');

        // sync from persisted state into memory and then
        // run the 'test' function
        FS.syncfs(true, function (err) {
            assert(!err);
            ccall('unlockLoadingCustomState', 'v', null, null, {async: true});
        });
    );

    while(loadingLocked)
        emscripten_sleep(10);
}

static void saveCustomState()
{
    loadingLocked = true;
    EM_ASM(
        FS.syncfs(function (err) {
            assert(!err);
            ccall('unlockLoadingCustomState', 'v');
        });
    );

    while(loadingLocked)
        emscripten_sleep(10);
}




void AppPathP::initDefaultPaths(const std::string & /*userDirName*/)
{
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
    SDL_free(path);

    if(!s_applicationPath.empty() && s_applicationPath.back() != '/')
        s_applicationPath.push_back('/');

    s_assetsRoot = s_applicationPath;
    s_userDirectory = s_applicationPath;

    loadCustomState();

#if !defined(DISABLE_LOGGING)
    std::printf("== App Path is %s\n", s_applicationPath.c_str());
    std::printf("== Assets Path is %s\n", s_assetsRoot.c_str());
    std::printf("== User Path is %s\n", s_userDirectory.c_str());
    fflush(stdout);
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
    return false;
}

void AppPathP::syncFS()
{
    saveCustomState();
}
