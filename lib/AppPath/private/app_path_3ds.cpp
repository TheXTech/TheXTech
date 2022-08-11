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

#include <3ds.h>

#include <vector>
#include <string>

#include <DirManager/dirman.h>

#include "../app_path.h"
#include "app_path_private.h"

static std::vector<std::string> s_worldRootDirs;
static std::string s_assetRoot;

// special 3DS functions to handle fast romfs archives
bool mountRomfsFile(const char* path, const char* mount_label)
{
    // for some reason dirent returns a very strange format that appears to be
    // utf-8 expressed as utf-16 and then converted back to utf-8.
    uint16_t utf16_buf[4096+1];
    uint8_t utf8_buf[4096+1];

    ssize_t units = utf8_to_utf16(utf16_buf, (const uint8_t*)path, 4096);
    if(units < 0 || units > 4096)
        return false;
    utf16_buf[units] = 0;
    for(int i = 0; i < units; i++)
    {
        utf8_buf[i] = (uint8_t) (0xff & utf16_buf[i]);
    }
    utf8_buf[units] = 0;

    Handle fd = 0;
    FS_Path archPath = { PATH_EMPTY, 1, "" };
    FS_Path filePath = { PATH_ASCII, (size_t)units+1, utf8_buf };

    Result rc = FSUSER_OpenFileDirectly(&fd, ARCHIVE_SDMC, archPath, filePath, FS_OPEN_READ, 0);
    if(R_FAILED(rc))
        return false;

    rc = romfsMountFromFile(fd, 0, mount_label);
    if(R_FAILED(rc))
        return false;

    return true;
}

// find additional user worlds packaged in .romfs files
void findUserWorlds()
{
    s_worldRootDirs.push_back(AppPathP::appDirectory() + "worlds/");
    s_worldRootDirs.push_back(AppPathP::userDirectory() + "worlds/");

    std::vector<std::string> romfsFiles;
    static const std::vector<std::string> romfsExt = {".romfs"};

    DirMan userDir(AppPathP::userDirectory());
    userDir.getListOfFiles(romfsFiles, romfsExt);

    std::string fullPath;
    char mount_label[9] = "romfsA:/";
    for(std::string& s : romfsFiles)
    {
        if(s == "assets.romfs")
            continue;

        fullPath = userDir.absolutePath() + "/" + s;
        mount_label[6] = '\0';

        if(!mountRomfsFile(fullPath.c_str(), mount_label))
            continue;

        mount_label[6] = ':';
        s_worldRootDirs.push_back(std::string(mount_label));
        mount_label[5] ++;
        // from Z to a.
        if(mount_label[5] == 91)
            mount_label[5] = 97;
        // max of 52 mounts.
        if(mount_label[5] == 123)
            break;
    }
}

void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    (void)userDirName;

    s_assetRoot = "romfs:/";

    // try to mount packaged files
    if(R_FAILED(romfsInit()))
    {
        // fallback to assets.romfs
        if(!mountRomfsFile("/3ds/thextech/assets.romfs", "romfs"))
        {
            // this is really bad. everything will be insanely slow.
            s_assetRoot = "/3ds/thextech/";
        }
    }

    findUserWorlds();
}

std::string AppPathP::appDirectory()
{
    return s_assetRoot;
}

std::string AppPathP::userDirectory()
{
    return "/3ds/thextech";
}

std::string AppPathP::assetsRoot()
{
    return s_assetRoot;
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

const std::vector<std::string>& AppPathManager::worldRootDirs() // Read-Only, appears at writable directory
{
    return s_worldRootDirs;
}
