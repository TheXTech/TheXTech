/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_filesystem.h>

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <IniProcessor/ini_processing.h>
#define FMT_NOEXCEPT
#include <fmt/fmt_format.h>

#ifdef __ANDROID__
#   include <unistd.h>
//#   include <md5/md5.h>
#   include <jni.h>
#   if 1
#       undef JNIEXPORT
#       undef JNICALL
#       define JNIEXPORT extern "C"
#       define JNICALL
#   endif
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <PGE_File_Formats/pge_file_lib_private.h>//It's only exception for macOS here to get URL-Decode. Never include this!
#include "app_path_macos_dirs.h"
#endif

#ifdef __gnu_linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <winreg.h>
#include <algorithm> // std::replace from \\ into /
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

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
#endif

#include "../app_path.h"
#include "../../version.h"

static std::string ApplicationPathSTD;

std::string AppPathManager::m_settingsPath;
std::string AppPathManager::m_assetsPath;
std::string AppPathManager::m_userPath;

std::string AppPathManager::m_screenshotsPath;
std::string AppPathManager::m_gifrecordingsPath;
std::string AppPathManager::m_logsPath;

std::string AppPathManager::m_customAssetsRoot;
std::string AppPathManager::m_customUserDirectory;

#if defined(__APPLE__)
//! The name of application bundle to be re-used as the user directory name
static std::string s_bundleName;
static std::string s_defaultAssetsRoot;
#endif

#if defined(VITA)
static std::string m_vitaAppDataPath = "ux0:data/TheXTech/";
#endif

#if defined(__ANDROID__)
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
#endif

#ifdef __APPLE__

#   ifndef USERDATA_ROOT_NAME
#       define USERDATA_ROOT_NAME "TheXTech"
#   endif

std::string AppPathManager::m_userDataRoot;
#endif
bool AppPathManager::m_isPortable = false;

#if defined(USER_DIR_NAME)
#define UserDirName "/" USER_DIR_NAME
#elif defined(__ANDROID__) || defined(__APPLE__) || defined(__HAIKU__)
#define UserDirName "/PGE Project"
#else
#define UserDirName "/.PGE_Project"
#endif

/**
 * @brief Retreive User Home directory with appending of the PGE user data directory
 * @return Absolute directory path
 */
static std::string getPgeUserDirectory()
{
    std::string path;

#if defined(__APPLE__)
    {
        char *base_path = getAppSupportDir();
        if(base_path)
        {
            path.append(base_path);
            SDL_free(base_path);
        }
    }

#elif defined(_WIN32)
    {
        wchar_t pathW[MAX_PATH];
        DWORD path_len = GetEnvironmentVariableW(L"UserProfile", pathW, MAX_PATH);
        assert(path_len);
        path.resize(path_len * 2);
        path_len = WideCharToMultiByte(CP_UTF8, 0,
                                       pathW,       path_len,
                                       &path[0],    path.size(),
                                       0, FALSE);
        path.resize(path_len);
    }

#elif defined(__ANDROID__)
    if(!m_androidGameAssetsPath.empty())
    {
        if(access(m_androidGameAssetsPath.c_str(), W_OK) != 0) // If assets directory is not writable
        {
            // Use the application's data directory for logs / settings / screenshots / GIFs
            return fmt::format("{0}/{1}",
                               m_androidAppDataPath,
                               Files::basename(m_androidGameAssetsPath));
        }

        return m_androidGameAssetsPath; // Don't search the path, simply re-use the defined path
    }

    path = m_androidSdCardPath;

    DirMan homeDir(path);
    if(!homeDir.exists())
        path = "/sdcard";

#elif defined(__HAIKU__)
    {
        const char *home = SDL_getenv("HOME");
        path.append(home);
    }

#elif defined(__gnu_linux__)
    {
        passwd *pw = getpwuid(getuid());
        path.append(pw->pw_dir);
    }
#endif

    return path.empty() ? std::string(".") : (path + UserDirName);
}


void AppPathManager::initAppPath()
{
#if defined(__APPLE__)
    {
        CFURLRef appUrlRef;
        appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFStringRef filePathRef = CFURLGetString(appUrlRef);
        char temporaryCString[PATH_MAX];
        bzero(temporaryCString, PATH_MAX);
        CFStringGetCString(filePathRef, temporaryCString, PATH_MAX, kCFStringEncodingUTF8);
        ApplicationPathSTD = PGE_URLDEC(std::string(temporaryCString));
        {
            s_bundleName = USERDATA_ROOT_NAME;

            auto i = ApplicationPathSTD.find_last_of(".app");

            auto j = ApplicationPathSTD.find_last_of('/', i);
            if(j != std::string::npos)
            {
                s_bundleName = ApplicationPathSTD.substr(j + 1);
                auto k = s_bundleName.find_last_of(".app") - 3;
                s_bundleName.erase(k, s_bundleName.size() - k);
            }

            i = ApplicationPathSTD.find_last_of('/', i);

            ApplicationPathSTD.erase(i, ApplicationPathSTD.size() - i);
            if(ApplicationPathSTD.compare(0, 7, "file://") == 0)
                ApplicationPathSTD.erase(0, 7);

            if(!ApplicationPathSTD.empty() && (ApplicationPathSTD.back() != '/'))
                ApplicationPathSTD.push_back('/');
        }
        CFRelease(appUrlRef);
    }

#elif defined(__ANDROID__)
    ApplicationPathSTD = m_androidAppDataPath;

#elif defined(VITA)
    ApplicationPathSTD = m_vitaAppDataPath;

#else // all other platforms (Windows, Linux, Haiku, etc.)
    char *path = SDL_GetBasePath();
    if(!path)
    {
#   ifndef DISABLE_LOGGING
        std::fprintf(stderr, "== Failed to recognize application path by using of SDL_GetBasePath! Using current working directory \"./\" instead.\n");
        std::fflush(stderr);
#   endif
        path = SDL_strdup("./");
    }
    ApplicationPathSTD = std::string(path);
#   if defined(_WIN32)
    std::replace(ApplicationPathSTD.begin(), ApplicationPathSTD.end(), '\\', '/');
#   endif
    SDL_free(path);

#endif // __APPLE__/__ANDROID__


#ifdef __EMSCRIPTEN__
    loadCustomState();
#endif

    // When user directory is redefined externally
    if(!m_customUserDirectory.empty())
    {
        m_userPath = m_customUserDirectory;
        initSettingsPath();
        return;
    }

    if(checkPortable())
        return;

    std::string userDirPath = getPgeUserDirectory();
    if(!userDirPath.empty())
    {
        DirMan appDir(userDirPath);
        if(!appDir.exists() && !appDir.mkpath())
            goto defaultSettingsPath;
// #ifdef __APPLE__
//         if(!DirMan::exists(ApplicationPathSTD + "/Data directory"))
//             symlink((userDirPath).c_str(), (ApplicationPathSTD + "/Data directory").c_str());
// #endif

#ifdef __ANDROID__
        std::string noMediaFile = userDirPath + "/.nomedia";
        if(!Files::fileExists(noMediaFile))
        {
            SDL_RWops *noMediaRWops = SDL_RWFromFile(noMediaFile.c_str(), "w");
            if(noMediaRWops)
            {
                SDL_RWwrite(noMediaRWops, "\0", 1, 1);
                SDL_RWclose(noMediaRWops);
            }
        }
#endif
        m_userPath = appDir.absolutePath();

#if !defined(__EMSCRIPTEN__) && !defined(USER_DIR_NAME)
#   if defined(__ANDROID__)
        if(m_androidGameAssetsPath.empty())
            m_userPath.append("/thextech/");
        else
            m_userPath.append("/");
#   else
        m_userPath.append("/thextech/");
#   endif
#else
        m_userPath.append("/");
#endif
        initSettingsPath();
    }
    else
    {
        goto defaultSettingsPath;
    }

    return;
defaultSettingsPath:
    m_userPath = ApplicationPathSTD;
    initSettingsPath();
#if defined(__EMSCRIPTEN__) && !defined(DISABLE_LOGGING)
    std::printf("== App Path is %s\n", ApplicationPathSTD.c_str());
    std::printf("== User Path is %s\n", m_userPath.c_str());
    fflush(stdout);
#endif
}

std::string AppPathManager::settingsFileSTD() // Writable
{
    return m_settingsPath + "thextech.ini";
}

std::string AppPathManager::settingsRoot() // Writable
{
    return m_settingsPath;
}

std::string AppPathManager::settingsControlsFileSTD() // Writable
{
    return m_settingsPath + "controls.ini";
}

std::string AppPathManager::userAppDirSTD() // Writable
{
    return m_userPath;
}

std::string AppPathManager::assetsRoot() // Readable
{
    if(!m_customAssetsRoot.empty())
        return m_customAssetsRoot;

#if defined(FIXED_ASSETS_PATH) // Fixed assets path, for the rest of UNIX-like OS packages
    std::string assets(FIXED_ASSETS_PATH);
    if(!assets.empty() && assets.back() != '/')
        assets.push_back('/');
    return assets;

#elif defined(__APPLE__)
#   if defined(USE_BUNDLED_ASSETS) // When its release game with assets shipped with a game
    CFURLRef appUrlRef;
    appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("assets"), NULL, NULL);
    CFStringRef filePathRef = CFURLGetString(appUrlRef);
    char temporaryCString[PATH_MAX];
    bzero(temporaryCString, PATH_MAX);
    CFStringGetCString(filePathRef, temporaryCString, PATH_MAX, kCFStringEncodingUTF8);
    std::string path = PGE_URLDEC(std::string(temporaryCString));
    if(path.compare(0, 7, "file://") == 0)
        path.erase(0, 7);
    return path;
#   else // When it's debug-mode built bundle, external assets will be used
    return s_defaultAssetsRoot;
#   endif

#elif defined(__ANDROID__)
    std::string assets = m_androidGameAssetsPath;
    if(!assets.empty() && assets.back() != '/')
        assets.push_back('/');
    return assets;

#else
    return m_userPath;

#endif
}

void AppPathManager::setAssetsRoot(const std::string &root)
{
    m_customAssetsRoot = root;
    if(!m_customAssetsRoot.empty() && m_customAssetsRoot.back() != '/')
        m_customAssetsRoot.push_back('/');
}

void AppPathManager::setUserDirectory(const std::string& root)
{
    m_customUserDirectory = root;
    if(!m_customUserDirectory.empty() && m_customUserDirectory.back() != '/')
        m_customUserDirectory.push_back('/');
}

std::string AppPathManager::logsDir() // Writable
{
    return m_userPath + "logs/";
}

std::string AppPathManager::languagesDir() // Readable
{
#if defined(__APPLE__)
    CFURLRef appUrlRef;
    appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("languages"), NULL, NULL);
    CFStringRef filePathRef = CFURLGetString(appUrlRef);
    char temporaryCString[PATH_MAX];
    bzero(temporaryCString, PATH_MAX);
    CFStringGetCString(filePathRef, temporaryCString, PATH_MAX, kCFStringEncodingUTF8);
    std::string path = PGE_URLDEC(std::string(temporaryCString));
    if(path.compare(0, 7, "file://") == 0)
        path.erase(0, 7);
    return path;

#elif defined(__ANDROID__)
    return AppPathManager::assetsRoot() + "languages/";

#else
    return ApplicationPathSTD + "languages/";

#endif
}

std::string AppPathManager::screenshotsDir() // Writable
{
#ifndef __APPLE__
    return m_userPath + "screenshots/";

#else
    std::string path = m_userPath;
    char *base_path = getScreenCaptureDir();
    if(base_path)
    {
        path = base_path;
        SDL_free(base_path);
    }
    return path + "/TheXTech Game Screenshots/";

#endif
}

std::string AppPathManager::gifRecordsDir() // Writable
{
#ifndef __APPLE__
    return m_userPath + "gif-recordings/";

#else
    std::string path = m_userPath;
    char *base_path = getScreenCaptureDir();
    if(base_path)
    {
        path = base_path;
        SDL_free(base_path);
    }
    return path + "/TheXTech Game Screenshots/gif-recordings/";

#endif
}

std::string AppPathManager::gameSaveRootDir() // Writable
{
    return m_settingsPath + "gamesaves/";
}

std::string AppPathManager::gameplayRecordsRootDir() // Writable
{
    return m_userPath + "gameplay-records/";
}

std::string AppPathManager::userWorldsRootDir() // Readable
{
#ifdef __APPLE__
    return m_userDataRoot + "worlds/";
#else
    return m_userPath + "worlds/";
#endif
}

std::string AppPathManager::userBattleRootDir() // Readable
{
#ifdef __APPLE__
    return m_userDataRoot + "battle/";
#else
    return m_userPath + "battle/";
#endif
}

void AppPathManager::install()
{
    std::string path = getPgeUserDirectory();

    if(!path.empty())
    {
        DirMan appDir(path);
        if(!appDir.exists())
            appDir.mkpath(path);
    }
}

//bool AppPathManager::isPortable()
//{
//    return m_isPortable;
//}

bool AppPathManager::checkPortable()
{
    if(m_settingsPath.empty())
        m_settingsPath = ApplicationPathSTD;

    if(m_userPath.empty())
        m_userPath = ApplicationPathSTD;

    if(!Files::fileExists(settingsFileSTD()))
        return false;

    m_isPortable = false;

    IniProcessing checkForPort(settingsFileSTD());
    checkForPort.beginGroup("Main");
    m_isPortable = checkForPort.value("force-portable", false).toBool();
    checkForPort.endGroup();

    if(m_isPortable)
        initSettingsPath();

    return m_isPortable;
}

bool AppPathManager::userDirIsAvailable()
{
    return (m_userPath != assetsRoot());
}

#ifdef __EMSCRIPTEN__
void AppPathManager::syncFs()
{
    saveCustomState();
}
#endif


void AppPathManager::initSettingsPath()
{
    m_settingsPath = m_userPath + "settings/";

#ifdef __APPLE__
    {
        const char *homeDir = std::getenv("HOME");
        if(homeDir)
        {
            m_userDataRoot = std::string(homeDir) + "/TheXTech Games/" + s_bundleName;
            m_userDataRoot.append("/");
            // Automatically create an infrastructure
            if(!DirMan::exists(m_userDataRoot))
                DirMan::mkAbsPath(m_userDataRoot);
            if(!DirMan::exists(m_userDataRoot + "worlds"))
                DirMan::mkAbsPath(m_userDataRoot + "worlds");
            if(!DirMan::exists(m_userDataRoot + "battle"))
                DirMan::mkAbsPath(m_userDataRoot + "battle");
            m_settingsPath = m_userDataRoot + "settings/";

            s_defaultAssetsRoot = std::string(homeDir) + "/TheXTech Games/Debug Assets/";
        }
        else
            m_userDataRoot = m_userPath;
    }
#endif

    if(Files::fileExists(m_settingsPath))
        Files::deleteFile(m_settingsPath);//Just in case, avoid mad jokes with making same-named file as settings folder

    if(!DirMan::exists(m_settingsPath))
        DirMan::mkAbsPath(m_settingsPath);

    // Also make the game saves root folder to be exist
    if(!DirMan::exists(gameSaveRootDir()))
        DirMan::mkAbsPath(gameSaveRootDir());

    // And make empty worlds and battle directories too, make a hint for user
    if(!DirMan::exists(userWorldsRootDir()))
        DirMan::mkAbsPath(userWorldsRootDir());
    if(!DirMan::exists(userBattleRootDir()))
        DirMan::mkAbsPath(userBattleRootDir());
}
