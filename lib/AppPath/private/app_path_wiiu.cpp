/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../app_path.h"
#include <DirManager/dirman.h>
#include "app_path_private.h"

constexpr const char* s_assetRootSD = "fs:/vol/external01/thextech/";
constexpr const char* s_assetRootUSB = "usb:/thextech/";
constexpr const char* s_assetRootContent = "fs:/vol/content/";

constexpr const char* s_assetSaveRoot = "fs:/vol/save/common/";

static std::string s_assetRoot;
static std::string s_userDir;
// Extra directories
static std::string s_logsDir;
static std::string s_screenshotsDir;
static std::string s_gifsDir;


void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    (void)userDirName;

    if(DirMan::exists(s_assetRootContent))
    {
        SAVEInit();
        SAVEInitCommonSaveDir();

        s_assetRoot = s_assetRootContent;
        s_userDir = s_assetSaveRoot;

        if(DirMan::exists("usb:/"))
        {
            std::string extraDir = "usb:/thextech-user/";
            s_logsDir = extraDir + "logs/";
            s_screenshotsDir = extraDir + "screenshots/";
            s_gifsDir = extraDir + "gif-recordings/";
        }
        else if(DirMan::exists("fs:/vol/external01/"))
        {
            std::string extraDir = "fs:/vol/external01/thextech-user/";
            s_logsDir = extraDir + "logs/";
            s_screenshotsDir = extraDir + "screenshots/";
            s_gifsDir = extraDir + "gif-recordings/";
        }
    }
    else if(DirMan::exists(s_assetRootUSB))
    {
        s_assetRoot = s_assetRootUSB;
        s_userDir = s_assetRootUSB;
    }
    else
    {
        s_assetRoot = s_assetRootSD;
        s_userDir = s_assetRootSD;
    }
}

std::string AppPathP::appDirectory()
{
    return s_assetRoot;
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
