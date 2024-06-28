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

#include <vector>
#include <string>

#include <nn/save.h>
#include <coreinit/title.h>

#include "../app_path.h"
#include <DirManager/dirman.h>
#include <fmt/fmt_printf.h>
#include "app_path_private.h"

//! The assets&user directory for standalone RPX when SD card is plugged
constexpr const char* s_assetRootSD = "fs:/vol/external01/wiiu/" THEXTECH_DIRECTORY_PREFIX "/";
//! The assets&user directory for standalone RPX when USB stick is plugged
constexpr const char* s_assetRootUSB = "usb:/wiiu/" THEXTECH_DIRECTORY_PREFIX "/";
//! The assets directory for WUHB/installable package of the game
constexpr const char* s_assetRootContent = "fs:/vol/content/";

//! Settings and gamesave directory for WUHB and installable packages
constexpr const char* s_assetSaveRoot = "fs:/vol/save/common/";

//! Read-only assets directory (may be separated when WUHB or an installable thing)
static std::string s_assetRoot;
//! Writeable user directory for logs, screenshots, settings and extra user's episodes
static std::string s_userDir;


void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    (void)userDirName;

    // When running game under WUHB package or as an installable game (/vol/content and /vol/save dirs do exist)
    // Also check that /vol/content/graphics/ exists (assets non-empty)
    if(DirMan::exists(s_assetRootContent) && DirMan::exists(std::string(s_assetRootContent) + "graphics/"))
    {
        SAVEInit();
        SAVEInitCommonSaveDir();

        // Consider /vol/content (WUHB resources) as read-only Assets directory
        s_assetRoot = s_assetRootContent;
        // And consider /vol/save directory as default User Directory
        s_userDir = s_assetSaveRoot;

        // When USB stick is presented, store logs, screenshots and find user's episodes on it
        if(DirMan::exists("usb:/"))
            s_userDir = s_assetRootUSB;
        // When SD card is presented, store logs, screenshots and find user's episodes on it
        else if(DirMan::exists("fs:/vol/external01/"))
            s_userDir = s_assetRootSD;
    }
    else if(DirMan::exists(s_assetRootUSB))
    {
        s_assetRoot.clear();
        s_userDir = s_assetRootUSB;
    }
    else
    {
        s_assetRoot.clear();
        s_userDir = s_assetRootSD;
    }
}

std::string AppPathP::appDirectory()
{
    return std::string();
}

std::string AppPathP::userDirectory()
{
    return s_userDir;
}

std::string AppPathP::assetsRoot()
{
    return s_assetRoot;
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
    return false;
}

void AppPathP::syncFS()
{
    /* Run the FS synchronization (Implement this for Emscripten only) */
}
