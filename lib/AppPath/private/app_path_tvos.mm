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
#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <Logger/logger.h>
#include <miniz.h>
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

static bool zipSettingsPack()
{
    std::string ar_file = s_userDirectory + "/tmp.zip";
    mz_zip_archive ar;
    std::string settings_root = AppPathP::settingsRoot();
    DirMan setup_dir(settings_root);

    if(!setup_dir.exists())
        return false; // Do nothing, directory does not exists

    memset(&ar, 0, sizeof(mz_zip_archive));

    Files::deleteFile(ar_file.c_str());

    if(mz_zip_writer_init_file_v2(&ar, ar_file.c_str(), 0, MZ_ZIP_FLAG_WRITE_ALLOW_READING))
    {
        std::string cur_path;
        std::vector<std::string> cur_files;
        bool failed = false;

        setup_dir.beginWalking();

        while(!failed && setup_dir.fetchListFromWalker(cur_path, cur_files))
        {
            for(std::string &file : cur_files)
            {
                if(file == ".DS_Store")
                    continue; // Don't store a junk!

                std::string cur_file = cur_path + "/" + file;
                std::string cur_relative_file = cur_file.substr(settings_root.size());

                D_pLogDebug("Adding to archive: %s", cur_relative_file.c_str());

                if(!mz_zip_writer_add_file(&ar, cur_relative_file.c_str(), cur_file.c_str(), NULL, 0, MZ_BEST_COMPRESSION|MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE))
                {
                    pLogWarning("tvOS: Failed to store %s into archive %s: %s", cur_relative_file.c_str(), ar_file.c_str(), mz_zip_get_error_string(ar.m_last_error));
                    failed = true;
                    break;
                }
            }
        }

        if(!failed)
            mz_zip_writer_finalize_archive(&ar);

        mz_zip_writer_end(&ar);
        return !failed;
    }
    else
    {
        pLogWarning("tvOS: Failed to open an archive %s: %s", ar_file.c_str(), mz_zip_get_error_string(ar.m_last_error));
    }

    return false;
}

static void zipSettingsExtract()
{
    std::string ar_file = s_userDirectory + "/tmp-out.zip";
    mz_zip_archive ar;
    std::string settings_root = AppPathP::settingsRoot();
    DirMan setup_dir(settings_root);

    if(!Files::fileExists(ar_file))
        return; // Nothing to do!

    memset(&ar, 0, sizeof(mz_zip_archive));

    if(mz_zip_reader_init_file_v2(&ar, ar_file.c_str(), 0, 0, 0))
    {
        mz_uint files = mz_zip_reader_get_num_files(&ar);
        char file[PATH_MAX];
        char out_path[PATH_MAX];
        std::string rel_dir;

        setup_dir.mkpath();

        for(mz_uint i = 0; i < files; ++i)
        {
            if(!mz_zip_reader_get_filename(&ar, i, file, PATH_MAX))
            {
                pLogWarning("tvOS: Can't get filename for file %d in archive %s: %s", i, ar_file.c_str(),  mz_zip_get_error_string(ar.m_last_error));
                continue;
            }

            memset(out_path, 0, sizeof(out_path));
            strncpy(out_path, settings_root.c_str(), PATH_MAX);
            strncat(out_path, file, PATH_MAX);

            if(mz_zip_reader_is_file_a_directory(&ar, i))
            {
                setup_dir.mkpath(file);
                continue;
            }

            rel_dir = Files::dirname(file);
            setup_dir.mkpath(rel_dir);

            if(!mz_zip_reader_extract_to_file(&ar, i, out_path, 0))
            {
                pLogWarning("tvOS: Can't extract file %s from archive %s: %s", file, ar_file.c_str(),  mz_zip_get_error_string(ar.m_last_error));
                continue;
            }
        }

        mz_zip_reader_end(&ar);
    }
    else
    {
        pLogWarning("tvOS: Can't open ZIP archive %s: %s", ar_file.c_str(),  mz_zip_get_error_string(ar.m_last_error));
    }
}

static void saveZipSettings()
{
    NSAutoreleasePool *pool;
    NSData *data;
    NSUserDefaults *defaults;
    Files::Data arDara;
    std::string ar_file = s_userDirectory + "/tmp.zip";

    arDara = Files::load_file(ar_file);

    if(arDara.empty())
    {
        pLogWarning("tvOS: Failed to dump the archive %s", ar_file.c_str());
        return;
    }

    pool = [[NSAutoreleasePool alloc] init];
    data = [NSData dataWithBytes:arDara.begin() length:arDara.size()];
    defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:data forKey:@"settings-zip"];
    [pool drain];
}

static bool loadZipSettings()
{
    NSAutoreleasePool *pool;
    NSData *data;
    NSUserDefaults *defaults;
    std::string ar_file = s_userDirectory + "/tmp-out.zip";
    bool ret = false;

    pool = [[NSAutoreleasePool alloc] init];
    defaults = [NSUserDefaults standardUserDefaults];
    data = [defaults objectForKey:@"settings-zip"];

    if(data)
    {
        FILE *out = fopen(ar_file.c_str(), "wb");
        if(out)
        {
            fwrite([data bytes], 1, [data length], out);
            fclose(out);
            ret = true;
        }
    }

    [pool drain];
    return ret;
}

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
    s_screenshotsPath = s_userDirectory + "/screenshots/";
    s_gifRecordPath = s_userDirectory + "/gif-recordings/";

    // Attempt to restore settings directory from a ZIP archive
    if(!DirMan::exists(AppPathP::settingsRoot()) && loadZipSettings())
        zipSettingsExtract();
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
    // To ensure the directiry will be same!
    return s_userDirectory + "/settings/";
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
    if(zipSettingsPack())
        saveZipSettings();
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
