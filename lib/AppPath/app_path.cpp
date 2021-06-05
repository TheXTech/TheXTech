/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2021 Vitaly Novichkov <admin@wohlnet.ru>
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


#ifndef __3DS__
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_filesystem.h>
#else
#include <3ds.h>
#endif

#include <DirManager/dirman.h>
#include <Utils/files.h>
#include <IniProcessor/ini_processing.h>
#define FMT_NOEXCEPT
#include <fmt/fmt_format.h>

#ifdef __ANDROID__
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
#include "app_path_macosx.h"
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

#include "app_path.h"
#include "../version.h"

std::string  ApplicationPathSTD;

std::string AppPathManager::m_settingsPath;
std::string AppPathManager::m_userPath;
#ifdef __3DS__
std::vector<std::string> AppPathManager::m_worldRootDirs;
#endif
std::string AppPathManager::m_customAssetsRoot;

#if defined(__ANDROID__)
//! Default path to the internal sotrage directory
static std::string m_androidSdCardPath = "/storage/emulated/0";
//! Customized absolute path to the game assets directory
static std::string m_androidGameAssetsPath;

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
#       define USERDATA_ROOT_NAME "TheXTech Episodes"
#   endif

std::string AppPathManager::m_userDataRoot;
#endif
bool AppPathManager::m_isPortable = false;

#if defined(USER_DIR_NAME)
#define UserDirName "/" USER_DIR_NAME
#elif defined(__3DS__)
#define UserDirName "/3ds/thextech"
#define FIXED_ASSETS_PATH "romfs:/"
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
#if defined(__3DS__)
    return UserDirName;
#elif defined(__APPLE__)
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
        return m_androidGameAssetsPath; // Don't search the path, simply re-use the defined path

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
            std::string::size_type i = ApplicationPathSTD.find_last_of(".app");
            i = ApplicationPathSTD.find_last_of('/', i);
            ApplicationPathSTD.erase(i, ApplicationPathSTD.size() - i);
            if(ApplicationPathSTD.compare(0, 7, "file://") == 0)
                ApplicationPathSTD.erase(0, 7);
            if(!ApplicationPathSTD.empty() && (ApplicationPathSTD.back() != '/'))
                ApplicationPathSTD.push_back('/');
        }
        CFRelease(appUrlRef);
    }
#elif defined(__3DS__)
    ApplicationPathSTD = "romfs:/";
#else
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
#endif //__APPLE__

#ifdef __EMSCRIPTEN__
    loadCustomState();
#endif

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
#   elif !defined(__3DS__)
        m_userPath.append("/thextech/");
#else
        m_userPath.append("/");
#   endif
#else
        m_userPath.append("/");
#endif
        initSettingsPath();
#ifdef __3DS__
        findUserWorlds(appDir);
#endif
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

#ifdef __3DS__
// find additional user worlds packaged in .romfs files
void AppPathManager::findUserWorlds(DirMan userDir)
{
    m_worldRootDirs.push_back(UserDirName "/worlds/");
    m_worldRootDirs.push_back(assetsRoot() + "worlds/");
    std::vector<std::string> romfsFiles;
    static const std::vector<std::string> romfsExt = {".romfs"};
    userDir.getListOfFiles(romfsFiles, romfsExt);
    // for some reason dirent returns a very strange format that appears to be
    // utf-8 expressed as utf-16 and then converted back to utf-8.
    uint16_t utf16_buf[4096+1];
    uint8_t utf8_buf[4096+1];
    std::string fullPath;
    char mount_label[9] = "romfsA:/";
    for (std::string& s : romfsFiles)
    {
        fullPath = userDir.absolutePath() + "/" + s;
        ssize_t units = utf8_to_utf16(utf16_buf, (const uint8_t*)fullPath.c_str(), 4096);
        if (units < 0 || units > 4096) continue;
        utf16_buf[units] = 0;
        for (int i = 0; i < units; i++)
        {
            utf8_buf[i] = (uint8_t) (0xff & utf16_buf[i]);
        }
        utf8_buf[units] = 0;

        Handle fd = 0;
        FS_Path archPath = { PATH_EMPTY, 1, "" };
        FS_Path filePath = { PATH_ASCII, units+1, utf8_buf };
        Result rc = FSUSER_OpenFileDirectly(&fd, ARCHIVE_SDMC, archPath, filePath, FS_OPEN_READ, 0);
        if (R_FAILED(rc))
            continue;
        mount_label[6] = '\0';
        rc = romfsMountFromFile(fd, 0, mount_label);
        if (R_FAILED(rc))
            continue;
        mount_label[6] = ':';
        m_worldRootDirs.push_back(std::string(mount_label));
        mount_label[5] ++;
        // from Z to a.
        if (mount_label[5] == 91)
            mount_label[5] = 97;
        // max of 52 mounts.
        if (mount_label[5] == 123)
            break;
    }
}
#endif

std::string AppPathManager::settingsFileSTD()
{
    return m_settingsPath + "thextech.ini";
}

std::string AppPathManager::settingsControlsFileSTD()
{
    return m_settingsPath + "controls.ini";
}

std::string AppPathManager::userAppDirSTD()
{
    return m_userPath;
}

std::string AppPathManager::assetsRoot()
{
    if(!m_customAssetsRoot.empty())
        return m_customAssetsRoot;
#if defined(FIXED_ASSETS_PATH) // Fixed assets path, for the rest of UNIX-like OS packages
    std::string assets(FIXED_ASSETS_PATH);
    if(!assets.empty() && assets.back() != '/')
        assets.push_back('/');
    return assets;

#elif defined(__APPLE__) && defined(USE_BUNDLED_ASSETS)
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

#elif defined(__ANDROID__)
    return m_userPath; //"assets/";

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

std::string AppPathManager::logsDir()
{
    return ApplicationPathSTD + "logs";
}

std::string AppPathManager::languagesDir()
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
    return "languages";
#else
    return ApplicationPathSTD + "languages";
#endif
}

std::string AppPathManager::screenshotsDir()
{
#ifndef __APPLE__
    return m_userPath + "screenshots";
#else
    std::string path = m_userPath;
    char *base_path = getScreenCaptureDir();
    if(base_path)
    {
        path = base_path;
        SDL_free(base_path);
    }
    return path + "/TheXTech Game Screenshots";
#endif
}

std::string AppPathManager::gifRecordsDir()
{
#ifndef __APPLE__
    return m_userPath + "gif-recordings";
#else
    std::string path = m_userPath;
    char *base_path = getScreenCaptureDir();
    if(base_path)
    {
        path = base_path;
        SDL_free(base_path);
    }
    return path + "/TheXTech Game Screenshots/gif-recordings";
#endif
}

std::string AppPathManager::gameSaveRootDir()
{
    return m_settingsPath + "gamesaves";
}

std::string AppPathManager::gameplayRecordsRootDir()
{
    return m_userPath + "gameplay-records";
}

std::string AppPathManager::userWorldsRootDir()
{
#ifdef __APPLE__
    return m_userDataRoot + "worlds";
#else
    return m_userPath + "worlds";
#endif
}

#ifdef __3DS__
const std::vector<std::string>& AppPathManager::worldRootDirs()
{
    return m_worldRootDirs;
}
#endif

std::string AppPathManager::userBattleRootDir()
{
#ifdef __APPLE__
    return m_userDataRoot + "battle";
#else
    return m_userPath + "battle";
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

bool AppPathManager::isPortable()
{
    return m_isPortable;
}

bool AppPathManager::checkPortable()
{
#ifdef __3DS__
    return false;
#endif
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
    return (m_userPath != ApplicationPathSTD);
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
            m_userDataRoot = std::string(homeDir) + "/" USERDATA_ROOT_NAME;
            m_userDataRoot.append("/");
            // Automatically create an infrastructure
            if(!DirMan::exists(m_userDataRoot))
                DirMan::mkAbsPath(m_userDataRoot);
            if(!DirMan::exists(m_userDataRoot + "worlds"))
                DirMan::mkAbsPath(m_userDataRoot + "worlds");
            if(!DirMan::exists(m_userDataRoot + "battle"))
                DirMan::mkAbsPath(m_userDataRoot + "battle");
        }
        else
            m_userDataRoot = m_userPath;
    }
#endif

    if(Files::fileExists(m_settingsPath))
        Files::deleteFile(m_settingsPath);//Just in case, avoid mad jokes with making same-named file as settings folder

    if(!DirMan::exists(m_settingsPath))
        DirMan::mkAbsPath(m_settingsPath);

    // Also make the gamesaves root folder to be exist
    if(!DirMan::exists(gameSaveRootDir()))
        DirMan::mkAbsPath(gameSaveRootDir());
}
