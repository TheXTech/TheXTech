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
constexpr const char* s_assetRootSD = "fs:/vol/external01/wiiu/thextech/";
//! The assets&user directory for standalone RPX when USB stick is plugged
constexpr const char* s_assetRootUSB = "usb:/wiiu/thextech/";
//! The assets directory for WUHB/installable package of the game
constexpr const char* s_assetRootContent = "fs:/vol/content/";

//! Settings and gamesave directory for WUHB and installable packages
constexpr const char* s_assetSaveRoot = "fs:/vol/save/common/";

//! Read-only assets directory (may be separated when WUHB or an installable thing)
static std::string s_assetRoot;
//! Writeable user directory for logs, screenshots, settings and extra user's episodes
static std::string s_userDir;
//! Directory to output log files
static std::string s_logsDir;
//! Directory to store taken screenshots
static std::string s_screenshotsDir;
//! Directory to store GIF recordings
static std::string s_gifsDir;
//! Settings directory
static std::string s_settingsDir;
//! Game saves storage directory
static std::string s_gamesavesDir;


void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    (void)userDirName;

    uint16_t titleId = OSGetTitleID();
    std::string titleIdString = fmt::sprintf("%016llX/", titleId);

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
        s_settingsDir.clear();
        s_gamesavesDir.clear();

        // When USB stick is presented, store logs, screenshots and find user's episodes on it
        if(DirMan::exists("usb:/"))
        {
            // FIXME: Find a way to obtain executable/WUHB name, and put all the stuff into sub-directory
            // That should avoid collisions between different game packages using the same engine.
            s_userDir = "usb:/wiiu/thextech-user/" + titleIdString;
            s_logsDir = s_userDir + "logs/";
            s_screenshotsDir = s_userDir + "screenshots/";
            s_gifsDir = s_userDir + "gif-recordings/";
            // Keep storing of settings and gamesaves at /val/save virtual directory
            s_settingsDir = std::string(s_assetSaveRoot) + "settings/";
            s_gamesavesDir = std::string(s_assetSaveRoot) + "gamesaves/";
        }
        // When SD card is presented, store logs, screenshots and find user's episodes on it
        else if(DirMan::exists("fs:/vol/external01/"))
        {
            // FIXME: Find a way to obtain executable/WUHB name, and put all the stuff into sub-directory
            // That should avoid collisions between different game packages using the same engine.
            s_userDir = "fs:/vol/external01/wiiu/thextech-user/" + titleIdString;
            s_logsDir = s_userDir + "logs/";
            s_screenshotsDir = s_userDir + "screenshots/";
            s_gifsDir = s_userDir + "gif-recordings/";
            // Keep storing of settings and gamesaves at /val/save virtual directory
            s_settingsDir = std::string(s_assetSaveRoot) + "settings/";
            s_gamesavesDir = std::string(s_assetSaveRoot) + "gamesaves/";
        }
    }
    else if(DirMan::exists(s_assetRootUSB))
    {
        s_assetRoot = s_assetRootUSB;
        s_userDir = s_assetRootUSB;
        s_settingsDir.clear();
        s_gamesavesDir.clear();
    }
    else
    {
        s_assetRoot = s_assetRootSD;
        s_userDir = s_assetRootSD;
        s_settingsDir.clear();
        s_gamesavesDir.clear();
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
    return s_settingsDir;
}

std::string AppPathP::gamesavesRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide gamesaves
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. gamesaves saved at the settings directory)
     */
    return s_gamesavesDir;
}

std::string AppPathP::screenshotsRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide screenshots
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. screenshots saved at the user directory)
     */
    return s_screenshotsDir;
}

std::string AppPathP::gifRecsRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide GIF recordings
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. GIF recordings saved at the user directory)
     */
    return s_gifsDir;
}

std::string AppPathP::logsRoot()
{
    /*
     * Fill this in only condition when you want to use the system-wide logs
     * directory out of user directory. Keep it empty if you want to keep the
     * default behaviour (i.e. logs saved at the user directory)
     */
    return s_logsDir;
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
