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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_filesystem.h>

#include "app_path_private.h"

static std::string s_assetsRoot;
static std::string s_userDirectory;
static std::string s_applicationPath;


void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    std::string homePath;

#if defined(__HAIKU__)
    const char *home = SDL_getenv("HOME");
    if(home)
        homePath.append(home);
#else
    passwd *pw = getpwuid(getuid());
    if(pw)
        homePath.append(pw->pw_dir);
    else
    {
        const char *home = SDL_getenv("HOME");
        if(home)
            homePath.append(home);
    }
#endif

    if(homePath.empty())
        homePath = std::string(".");
    else
        homePath += userDirName;

    if(!homePath.empty() && homePath.back() != '/')
        homePath.push_back('/');

    s_userDirectory = homePath;
    s_assetsRoot = homePath;

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

void AppPathP::syncFS()
{}
