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
//#   include <md5/md5.h>
#include <jni.h>
#if 1
#   undef JNIEXPORT
#   undef JNICALL
#   define JNIEXPORT extern "C"
#   define JNICALL
#endif

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_rwops.h>

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <IniProcessor/ini_processing.h>
#define FMT_NOEXCEPT
#include <fmt/fmt_format.h>

#include "app_path_private.h"

static std::string s_assetsRoot;
static std::string s_userDirectory;
static std::string s_applicationPath;

//! Default path to the internal sotrage directory
static std::string m_androidSdCardPath = "/storage/emulated/0";
//! Customized absolute path to the game assets directory
static std::string m_androidGameAssetsPath;
//! Application data absolute directory path (may be inaccessible from file managers since Android 11, etc.)
static std::string m_androidAppDataPath;


JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setSdCardPath(
    JNIEnv *env,
    jclass type,
    jstring sdcardPath_j
)
{
    const char *sdcardPath;
    (void)type;
    sdcardPath = env->GetStringUTFChars(sdcardPath_j, nullptr);
    m_androidSdCardPath = sdcardPath;
    env->ReleaseStringUTFChars(sdcardPath_j, sdcardPath);
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setAppDataPath(
    JNIEnv *env,
    jclass type,
    jstring appDataPath_j
)
{
    const char *appDataPath;
    (void)type;
    appDataPath = env->GetStringUTFChars(appDataPath_j, nullptr);
    m_androidAppDataPath = appDataPath;
    env->ReleaseStringUTFChars(appDataPath_j, appDataPath);
}

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setGameAssetsPath(
    JNIEnv *env,
    jclass type,
    jstring gameAssetsPath_j
)
{
    const char *gameAssetsPath;
    (void)type;
    gameAssetsPath = env->GetStringUTFChars(gameAssetsPath_j, nullptr);
    m_androidGameAssetsPath = gameAssetsPath;
    env->ReleaseStringUTFChars(gameAssetsPath_j, gameAssetsPath);
}


void AppPathP::initDefaultPaths(const std::string &userDirName)
{
    // Application path
    s_applicationPath = m_androidAppDataPath;

    // Detect the user directory
    if(!m_androidGameAssetsPath.empty())
    {
        s_userDirectory = m_androidGameAssetsPath; // Don't search the path, simply re-use the defined path
        s_assetsRoot = m_androidGameAssetsPath;
    }
    else
    {
        s_userDirectory = m_androidSdCardPath;
        DirMan homeDir(s_userDirectory);
        if(s_userDirectory.empty() || !homeDir.exists())
            s_userDirectory = "/sdcard";

        s_userDirectory += userDirName;
    }

    // Check does user directory exists
    DirMan appDir(s_userDirectory);
    if(!appDir.exists() && !appDir.mkpath())
        s_userDirectory = s_applicationPath;

    if(!s_userDirectory.empty() && s_userDirectory.back() != '/')
        s_userDirectory.push_back('/');

    // Check is selected user directory writable
    if(access(s_userDirectory.c_str(), W_OK) != 0) // If assets directory is not writable
    {
        // Use the application's data directory for logs / settings / screenshots / GIFs / gamesaves
        s_userDirectory = fmt::format("{0}/{1}",
                                      m_androidAppDataPath,
                                      Files::basename(s_userDirectory));
    }

    // Check presence for ".nomedia" file
    std::string noMediaFile = s_userDirectory + ".nomedia";
    if(!Files::fileExists(noMediaFile))
    {
        // If doesn't exists, make it
        SDL_RWops *noMediaRWops = SDL_RWFromFile(noMediaFile.c_str(), "w");
        if(noMediaRWops)
        {
            SDL_RWwrite(noMediaRWops, "\0", 1, 1);
            SDL_RWclose(noMediaRWops);
        }
    }

    // Choose the assets directory
    if(!m_androidGameAssetsPath.empty())
        s_assetsRoot = m_androidGameAssetsPath;
    else
        s_assetsRoot = s_userDirectory;

    if(!s_assetsRoot.empty() && s_assetsRoot.back() != '/')
        s_assetsRoot.push_back('/');
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
    return false;
}

void AppPathP::syncFS()
{
    /* Run the FS synchronization (Implement this for Emscripten only) */
}
