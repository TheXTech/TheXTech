/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifdef _WIN32
#   include <windows.h>
#   include <winternl.h>
#   include <sysinfoapi.h>
#   include <initguid.h>
#   include <dxdiag.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_cpuinfo.h>

#include <Logger/logger.h>
#include <Graphics/graphics_funcs.h>

#include "globals.h"
#include "main/game_info.h"
#include "window_sdl.h"
#include "../render.h"
#include "config.h"
#include "../version.h"


#ifdef RENDER_FULLSCREEN_TYPES_SUPPORTED
#   include <map>
#   include <set>
#   include "pge_cpu_arch.h"

static bool s_hasFrameBuffer = false;
static std::vector<AbstractWindow_t::VideoModeRes> s_availableRes;
static std::vector<uint8_t> s_availableColours;
static std::map<uint8_t, uint32_t> s_availableColoursMap;


static uint8_t s_getColorBits(uint32_t format)
{
    switch(format)
    {
    case SDL_PIXELFORMAT_BGR565:
    case SDL_PIXELFORMAT_RGB565:
        return 16u;
    case SDL_PIXELFORMAT_RGB888:
    case SDL_PIXELFORMAT_BGR888:
    case SDL_PIXELFORMAT_RGBA8888:
    case SDL_PIXELFORMAT_BGRA8888:
        return 32u;
    }

    return 32u;
}

static uint32_t s_getColorFromBits(uint8_t bits)
{
    auto f = s_availableColoursMap.find(bits);
    if(f != s_availableColoursMap.end())
        return f->second;

    switch(bits)
    {
    case 16u:
        return SDL_PIXELFORMAT_RGB565;
    case 32u:
        return SDL_PIXELFORMAT_RGB888;
    }

    return SDL_PIXELFORMAT_RGB888;
}

static inline bool s_isExclusiveFullScreen(Uint32 flags)
{
    return (flags & SDL_WINDOW_FULLSCREEN) == (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void WindowSDL::setHasFrameBuffer(bool has)
{
    s_hasFrameBuffer = has;
}

static void s_fillScreenModes()
{
    SDL_DisplayMode mode;
    int modes = SDL_GetNumDisplayModes(0);

    std::map<uint32_t, std::set<std::pair<int, int>>> m_hasRes;
    std::map<uint32_t, std::vector<AbstractWindow_t::VideoModeRes>> m_listModes;
    std::set<uint32_t> m_listColours;

    s_availableColours.clear();
    s_availableColoursMap.clear();

    pLogDebug("List of available screen modes:");
    for(int i = modes - 1; i >= 0; --i)
    {
        SDL_GetDisplayMode(0, i, &mode);

        pLogDebug("-- C=%u (%s), W=%d, H=%d, R=%d",
                  mode.format, SDL_GetPixelFormatName(mode.format), mode.w, mode.h, mode.refresh_rate);

        auto &hasRes = m_hasRes[mode.format];

        if(hasRes.find({mode.w, mode.h}) == hasRes.end())
        {
            pLogDebug("-- Insert C=%u (%s), W=%d, H=%d",
                      mode.format, SDL_GetPixelFormatName(mode.format), mode.w, mode.h);
            hasRes.insert({mode.w, mode.h});
            m_listModes[mode.format].push_back({mode.w, mode.h});
            if(m_listColours.find(mode.format) == m_listColours.end())
                m_listColours.insert(mode.format);
        }
    }

    // The list should NOT be empty!
    SDL_assert_release(!m_listModes.empty() && !m_listModes.begin()->second.empty());

    // If only one colour mode caught, just add everything into the list
    if(m_listModes.size() == 1)
    {
        auto &e = *m_listModes.begin();
        auto col = s_getColorBits(e.first);
        pLogDebug("Only one colour mode is available - C=%u (%s, %u bits):", e.first, SDL_GetPixelFormatName(e.first), (uint32_t)col);
        s_availableColours.push_back(col);
        s_availableColoursMap.insert({col, e.first});

        for(auto jt = e.second.begin(); jt != e.second.end(); ++jt)
            s_availableRes.push_back(*jt);
    }
    else
    {
        bool notEqual = false;

        pLogDebug("List of modes per colour depth:");

        for(auto it = m_listModes.begin(); it != m_listModes.end(); ++it)
        {
            auto col = s_getColorBits(it->first);
            pLogDebug("- C=%u (%s, %u bits):", it->first, SDL_GetPixelFormatName(it->first), (uint32_t)col);
            s_availableColours.push_back(col);
            s_availableColoursMap.insert({col, it->first});
        }

        auto &first = m_listModes.begin()->second;

        for(auto it = m_listModes.begin() ; it != m_listModes.end(); ++it)
        {
            s_availableRes.clear();

            pLogDebug("-- C=%u (%s):", it->first, SDL_GetPixelFormatName(it->first));

            if(it == m_listModes.begin())
            {
                for(auto jt = it->second.begin();  jt != it->second.end() ; ++jt)
                {
                    pLogDebug("-- %d x %d", jt->w, jt->h);
                    s_availableRes.push_back(*jt);
                }

                continue;
            }

            for(auto ft = first.begin(), jt = it->second.begin();  ; ++jt, ++ft)
            {
                if(jt == it->second.end() && ft == first.end())
                    break; // Both equal

                if((jt == it->second.end()) ^ (ft == first.end()))
                {
                    // Different length
                    pLogDebug("-- Not equal by length");
                    notEqual = true;
                    break;
                }

                if(jt->w != ft->w || jt->h != ft->h)
                {
                    // Not equal content
                    pLogDebug("-- Not equal by resolutions: %d x %d vs %d x %d", jt->w, jt->h, ft->w, ft->h);
                    notEqual = true;
                    break;
                }

                pLogDebug("-- %d x %d", jt->w, jt->h);
                s_availableRes.push_back(*ft);
            }

            if(notEqual)
                break; // Work finished!
        }
    }
}

#ifdef _WIN32

static bool s_hasWinSysInfo = false;
static int32_t s_suggestedFullscreenType = 0;

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

static HRESULT GetStringValue(IDxDiagContainer* pObject, const wchar_t* wstrName, char* strValue, int nStrLen)
{
    HRESULT hr;
    VARIANT var;
    VariantInit( &var );

    if(FAILED(hr = pObject->GetProp(wstrName, &var)))
        return hr;

    if(var.vt != VT_BSTR)
        return E_INVALIDARG;

    wcstombs(strValue, var.bstrVal, nStrLen);
    strValue[nStrLen - 1] = '\0';
    VariantClear(&var);

    return S_OK;
}

static uint32_t s_getVideoRam()
{
    unsigned int ret = 0;
    HRESULT hr;

    IDxDiagProvider* pProvider = 0;
    IDxDiagContainer* pRoot = 0;
    IDxDiagContainer* pObject = 0;
    IDxDiagContainer* pContainer = 0;
    DWORD count = 0;
    WCHAR wszContainer[256];
    char videoMemoryStr[128] = "";

    DXDIAG_INIT_PARAMS dxDiagInitParam;

    hr = CoCreateInstance(CLSID_DxDiagProvider,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDxDiagProvider,
                          (LPVOID*)&pProvider);

    if (!pProvider || FAILED(hr))
        return 0;

    SDL_memset(&dxDiagInitParam, 0, sizeof(DXDIAG_INIT_PARAMS));

    dxDiagInitParam.dwSize = sizeof(DXDIAG_INIT_PARAMS);
    dxDiagInitParam.dwDxDiagHeaderVersion = DXDIAG_DX9_SDK_VERSION;
    dxDiagInitParam.bAllowWHQLChecks = true;
    dxDiagInitParam.pReserved = NULL;

    hr = pProvider->Initialize(&dxDiagInitParam);
    if(SUCCEEDED(hr))
    {
        hr = pProvider->GetRootContainer( &pRoot);
        if (SUCCEEDED(hr))
        {
            // fills in the struct from DXDIAG...
            HRESULT hr = pRoot->GetChildContainer(L"DxDiag_DisplayDevices", &pContainer);
            if(FAILED(hr))
            {
                SAFE_RELEASE(pProvider);
                return 0;
            }

            DWORD nItem = 0;

            if(SUCCEEDED(hr = pContainer->GetNumberOfChildContainers(&count)))
            {
                for(; nItem < count&& nItem < 16; nItem++)
                {
                    hr = pContainer->EnumChildContainerNames(nItem, wszContainer, 256);
                    if(FAILED(hr))
                    {
                        SAFE_RELEASE(pObject);
                        SAFE_RELEASE(pContainer);
                        SAFE_RELEASE(pProvider);
                        return 0;
                    }

                    hr = pContainer->GetChildContainer(wszContainer, &pObject);
                    if(FAILED(hr) || pObject == NULL)
                    {
                        SAFE_RELEASE(pObject);
                        SAFE_RELEASE(pContainer);
                        SAFE_RELEASE(pProvider);
                        return 0;
                    }

                    if(FAILED(hr = GetStringValue(pObject, L"szDisplayMemoryEnglish", videoMemoryStr, sizeof(videoMemoryStr))))
                    {
                        SAFE_RELEASE(pObject);
                        SAFE_RELEASE(pContainer);
                        SAFE_RELEASE(pProvider);
                        return 0;
                    }

                    pLogDebug("Video memory value: %s", videoMemoryStr);

                    char *num = videoMemoryStr;
                    char *suff = nullptr;

                    for(size_t i = 0; i < sizeof(videoMemoryStr) && videoMemoryStr[i] != '\0'; ++i)
                    {
                        if(videoMemoryStr[i] == ' ')
                        {
                            videoMemoryStr[i] = '\0';
                            suff = videoMemoryStr + i + 1;
                            ret = SDL_strtoul(num, nullptr, 10);

                            if(SDL_strncasecmp(suff, "GB", 2) == 0)
                                ret *= 1024;
                            else if(SDL_strncasecmp(suff, "TB", 2) == 0)
                                ret *= 1024 * 1024;

                            break;
                        }
                    }
                }
            }
        }
    }

    SAFE_RELEASE(pObject);
    SAFE_RELEASE(pContainer);
    SAFE_RELEASE(pProvider);

    return ret;
}

static void s_updateSysInfo()
{
    if(s_hasWinSysInfo)
        return;

    // Try to collect various system-wide information to heuristically detect legacy computer
    OSVERSIONINFO osvi;
    SYSTEM_INFO sysInfo;
    uint32_t videoMemSize = 0;

    bool isWinXPorOlder = false;

    SDL_memset(&osvi, 0, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);

    GetSystemInfo(&sysInfo);

    isWinXPorOlder = osvi.dwMajorVersion < 5 || (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion <= 1);

    videoMemSize = s_getVideoRam();

    pLogDebug("Windows Version: %u.%u, CPU type: %u, Video memory: %d MB",
              osvi.dwMajorVersion,
              osvi.dwMinorVersion,
              sysInfo.dwProcessorType,
              videoMemSize
    );

    s_suggestedFullscreenType = SDL_WINDOW_FULLSCREEN_DESKTOP;

    if(isWinXPorOlder && videoMemSize > 0 && videoMemSize < 128)
        s_suggestedFullscreenType = SDL_WINDOW_FULLSCREEN;

    s_hasWinSysInfo = true;
}

#endif

static int s_fsTypeToSDL(int type)
{
    switch(type)
    {
    case 0:
    {
#ifdef _WIN32
        if(s_suggestedFullscreenType > 0)
            return s_suggestedFullscreenType;
#endif

#if defined(PGE_CPU_x86_32) || defined(PGE_CPU_ARM32) || defined(PGE_CPU_PPC32)
        // On 32-bit architecures
        return SDL_WINDOW_FULLSCREEN;
#else
        if(!s_hasFrameBuffer)
            return SDL_WINDOW_FULLSCREEN;
#endif
        break;
    }
    case 1:
    default:
        return SDL_WINDOW_FULLSCREEN_DESKTOP;
    case 2:
        return SDL_WINDOW_FULLSCREEN;
    }

    return SDL_WINDOW_FULLSCREEN_DESKTOP;
}
#endif


#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>

void s_emscriptenFillBrowser()
{
    EmscriptenFullscreenStrategy strategy;
    strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
    strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
    strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;

    strategy.canvasResizedCallback = nullptr;

    pLogDebug("Activating canvas resize code");

    emscripten_enter_soft_fullscreen("canvas", &strategy);
    EM_ASM(currentFullscreenStrategy_bak = currentFullscreenStrategy;);
}

EM_BOOL s_restoreSoftFullscreen(int eventType, const void *reserved, void *userData)
{
    (void)(eventType);
    (void)(reserved);
    (void)(userData);

    pLogDebug("Restoring canvas resize code after entering fullscreen...");

    EM_ASM(
        currentFullscreenStrategy = currentFullscreenStrategy_bak;
    );
    return false;
}

void s_emscriptenRealFullscreen()
{
    EmscriptenFullscreenStrategy strategy;

    strategy.scaleMode = 0;
    strategy.canvasResolutionScaleMode = 0;
    strategy.filteringMode = 0;
    strategy.canvasResizedCallback = s_restoreSoftFullscreen;

    pLogDebug("Requesting HTML5 fullscreen mode...");

    emscripten_request_fullscreen_strategy("#thextech-document", true, &strategy);
}

void s_emscriptenLeaveRealFullscreen()
{
    EM_ASM(
        if(document.fullscreenElement || document.webkitFullscreenElement)
        {
            if(document.webkitExitFullscreen)
                document.webkitExitFullscreen();
            else
                document.exitFullscreen();

            setTimeout(() => {
                console.log("Restoring canvas on return from fullscreen");
                softFullscreenResizeWebGLRenderTarget();
                window.dispatchEvent(new Event('resize'));
            }, 500);
        }
    );
}

#endif

//! Path to game resources assets (by default it's ~/.PGE_Project/thextech/)
extern std::string AppPath;


WindowSDL::WindowSDL() :
    AbstractWindow_t()
{}

WindowSDL::~WindowSDL()
{}

bool WindowSDL::initSDL(uint32_t windowInitFlags)
{
    SDL_ClearError();

    bool res = true;

#ifdef _WIN32
    s_updateSysInfo();
#endif

    // SDL_GL_ResetAttributes();

#if defined(__SWITCH__) /* On Switch, expect the initial size 1920x1080 */
    const int initWindowW = 1920;
    const int initWindowH = 1080;
#else
    const auto initWindowW = XRender::TargetW;
    const auto initWindowH = XRender::TargetH;
#endif

#if defined(RENDER_FULLSCREEN_ALWAYS)
    windowInitFlags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_SHOWN;
#else
    windowInitFlags |= SDL_WINDOW_HIDDEN;
#endif

#ifdef RENDER_FULLSCREEN_TYPES_SUPPORTED
    m_fullscreen_type = -1; // Will be initialized later
#endif

    // restore fullscreen state
    if(m_fullscreen)
        windowInitFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

#ifdef __EMSCRIPTEN__
    // don't use SDL fullscreen API on Emscripten
    windowInitFlags &= ~SDL_WINDOW_FULLSCREEN;
#endif

    std::string window_name = "TheXTech Engine - (TheXTech v" V_LATEST_STABLE ", #" V_BUILD_VER ")";
    if(!g_gameInfo.title.empty())
        window_name = g_gameInfo.titleWindow();

    D_pLogDebug("SDL2: Extra WindowFlags: %u", windowInitFlags);

    m_window = SDL_CreateWindow(window_name.c_str(),
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                initWindowW, initWindowH,
                                SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_ALLOW_HIGHDPI |
                                windowInitFlags);

    if(m_window == nullptr)
    {
        const char *error = SDL_GetError();
        if(*error != '\0')
            pLogFatal("Unable to create window: %s", error);
        else
            pLogFatal("Unable to create window!");

        SDL_ClearError();
        return false;
    }
    else if(isSdlError())
    {
        const char *error = SDL_GetError();
        pLogCritical("SDL error on window creation: %s", error);
        SDL_ClearError();
    }

    SDL_SetWindowMinimumSize(m_window, 240, 160);

#ifdef __EMSCRIPTEN__ // Set canvas be 1/2 size for a faster rendering
    SDL_SetWindowSize(m_window, XRender::TargetW / 2, XRender::TargetH / 2);
#elif defined(__ANDROID__) || defined(__SWITCH__) // Set as small as possible
    SDL_SetWindowMinimumSize(m_window, 200, 150);
#elif defined(VITA)
    SDL_SetWindowSize(m_window, 960, 544);
#else
    if(g_config.scale_mode == Config_t::SCALE_FIXED_05X)
        SDL_SetWindowSize(m_window, XRender::TargetW / 2, XRender::TargetH / 2);
    else if(g_config.scale_mode == Config_t::SCALE_FIXED_2X)
        SDL_SetWindowSize(m_window, XRender::TargetW * 2, XRender::TargetH * 2);
    else if(g_config.scale_mode == Config_t::SCALE_FIXED_3X)
        SDL_SetWindowSize(m_window, XRender::TargetW * 3, XRender::TargetH * 3);
    else
        SDL_SetWindowSize(m_window, XRender::TargetW, XRender::TargetH);
#endif //__EMSCRIPTEN__

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

#ifdef RENDER_FULLSCREEN_TYPES_SUPPORTED
    s_fillScreenModes();

    m_curColour = g_config.fullscreen_depth;
    m_curRes.w = g_config.fullscreen_res.m_value.first;
    m_curRes.h = g_config.fullscreen_res.m_value.second;

    D_pLogDebug("Loaded video mode from config: %d -> %d x %d", m_curColour, m_curRes.w, m_curRes.h);

    if(m_screen_orig_w == 0 || m_screen_orig_h == 0)
    {
        int display = SDL_GetWindowDisplayIndex(m_window);
        SDL_DisplayMode mode;

        if(SDL_GetCurrentDisplayMode(display, &mode) < 0)
        {
            pLogWarning("Getting current display mode failed: %s", SDL_GetError());
            return -1;
        }

        m_screen_orig_w = mode.w;
        m_screen_orig_h = mode.h;
    }
#endif // RENDER_FULLSCREEN_TYPES_SUPPORTED

#ifdef __EMSCRIPTEN__
    EM_ASM(
        document.documentElement.id = "thextech-document";
        document.body.style.overflow = "hidden";
    );
    s_emscriptenFillBrowser();
#endif

    return res;
}

void WindowSDL::close()
{
    if(m_window)
    {
#ifdef RENDER_FULLSCREEN_TYPES_SUPPORTED
        // Turn the windowed mode before quit, otherwise, on some old systems screen resolution won't restore back
        if((SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN)
            SDL_SetWindowFullscreen(m_window, SDL_FALSE);
#endif
        SDL_DestroyWindow(m_window);
    }
    m_window = nullptr;
}

void WindowSDL::updateWindowIcon()
{
#ifdef _WIN32
    FIBITMAP *img[2];
    img[0] = GraphicsHelps::loadImage(AppPath + "/graphics/ui/icon/thextech_16.png");
    img[1] = GraphicsHelps::loadImage(AppPath + "/graphics/ui/icon/thextech_32.png");

    if(img[0] && !GraphicsHelps::setWindowIcon(m_window, img[0], 16))
    {
        pLogWarning("Unable to setup window icon!");
        SDL_ClearError();
    }

    if(img[1] && !GraphicsHelps::setWindowIcon(m_window, img[1], 32))
    {
        pLogWarning("Unable to setup window icon!");
        SDL_ClearError();
    }

    GraphicsHelps::closeImage(img[0]);
    GraphicsHelps::closeImage(img[1]);
#else//IF _WIN32

    FIBITMAP *img;
#   ifdef __APPLE__
    img = GraphicsHelps::loadImage(AppPath + "/graphics/ui/icon/thextech_256.png");
#   else
    img = GraphicsHelps::loadImage(AppPath + "/graphics/ui/icon/thextech_32.png");
#   endif //__APPLE__

    if(img)
    {
        SDL_Surface *sIcon = GraphicsHelps::fi2sdl(img);
        SDL_SetWindowIcon(m_window, sIcon);
        GraphicsHelps::closeImage(img);
        SDL_FreeSurface(sIcon);

        if(isSdlError())
        {
            pLogWarning("Unable to setup window icon!");
            SDL_ClearError();
        }
    }
#endif//IF _WIN32 #else
}

SDL_Window *WindowSDL::getWindow()
{
    return m_window;
}

bool WindowSDL::isSdlError()
{
    const char *error = SDL_GetError();
    return (*error != '\0');
}

void WindowSDL::show()
{
    SDL_ShowWindow(m_window);
}

void WindowSDL::hide()
{
    SDL_HideWindow(m_window);
    showCursor(1);
}

int WindowSDL::showCursor(int show)
{
    return SDL_ShowCursor(show);
}

void WindowSDL::setCursor(WindowCursor_t cursor)
{
    m_cursor = cursor;
    // Do nothing, just remember the last cursor type was set
}

WindowCursor_t WindowSDL::getCursor()
{
    return m_cursor;
}

void WindowSDL::placeCursor(int window_x, int window_y)
{
    if(!this->hasWindowInputFocus())
        return;

    int old_window_x, old_window_y;
    SDL_GetMouseState(&old_window_x, &old_window_y);
    int o_sx, o_sy, n_sx, n_sy;

    XRender::mapToScreen(old_window_x, old_window_y, &o_sx, &o_sy);
    XRender::mapToScreen(window_x, window_y, &n_sx, &n_sy);

    if(n_sx - o_sx < -2 || n_sx - o_sx > 2 || n_sy - o_sy < -2 || n_sy - o_sy > 2)
    {
        int window_w, window_h;
        this->getWindowSize(&window_w, &window_h);
        if(window_x >= 0 && window_x < window_w && window_y >= 0 && window_y < window_h)
            SDL_WarpMouseInWindow(m_window, window_x, window_y);
    }
}

bool WindowSDL::isFullScreen()
{
    return m_fullscreen;
}

int WindowSDL::setFullScreen(bool fs)
{
    if(!m_window)
        return -1;

    if((SDL_GetWindowFlags(m_window) & SDL_WINDOW_RESIZABLE) == 0)
        return -1; // Can't switch fullscreen mode when window is not resizable

    if(fs != m_fullscreen)
    {
        m_fullscreen = fs;

#ifdef __EMSCRIPTEN__
        if(fs)
            s_emscriptenRealFullscreen();
        else
            s_emscriptenLeaveRealFullscreen();

        return m_fullscreen;
#else
        if(fs)
        {
            // Swith to FULLSCREEN mode
#ifdef RENDER_FULLSCREEN_TYPES_SUPPORTED
            if(SDL_SetWindowFullscreen(m_window, m_fullscreen_type_real) < 0)
            {
                pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
                return -1;
            }

            syncFullScreenRes();
#else
            if(SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
            {
                pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
                return -1;
            }
#endif

            // Hide mouse cursor in full screen mdoe
            SDL_ShowCursor(SDL_DISABLE);
            return 1;
        }
        else
        {
            // Swith to WINDOWED mode
            if(SDL_SetWindowFullscreen(m_window, SDL_FALSE) < 0)
            {
                pLogWarning("Setting windowed failed: %s", SDL_GetError());
                return -1;
            }
            return 0;
        }
#endif
    }

    return 0;
}


#ifdef RENDER_FULLSCREEN_TYPES_SUPPORTED

const std::vector<AbstractWindow_t::VideoModeRes> &WindowSDL::getAvailableVideoResolutions()
{
    return s_availableRes;
}

const std::vector<uint8_t> &WindowSDL::getAvailableColourDepths()
{
    return s_availableColours;
}

void WindowSDL::getCurrentVideoMode(VideoModeRes &res, uint8_t &colourDepth)
{
    res = m_curRes;
    colourDepth = m_curColour;
}

void WindowSDL::setVideoMode(const VideoModeRes &res, uint8_t colourDepth)
{
    m_curRes = res;

    if(colourDepth == 0 || s_availableColoursMap.find(colourDepth) != s_availableColoursMap.end())
        m_curColour = colourDepth;
    else
        m_curColour = 0;

    syncFullScreenRes();
}

int WindowSDL::setFullScreenType(int type)
{
    bool change_needed = m_fullscreen_type != type;
    m_fullscreen_type = type;
    m_fullscreen_type_real = s_fsTypeToSDL(type);

    if(change_needed && m_fullscreen && m_window)
    {
        // Switch into normal mode temporarily
        if((SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN) != 0)
            SDL_SetWindowFullscreen(m_window, SDL_FALSE);

        if(m_fullscreen_type_real == SDL_WINDOW_FULLSCREEN_DESKTOP)
        {
            if(SDL_SetWindowFullscreen(m_window, m_fullscreen_type_real) < 0)
            {
                pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
                return -1;
            }
        }
        else if(m_fullscreen_type_real == SDL_WINDOW_FULLSCREEN)
            return syncFullScreenRes();
    }

    return 0;
}

int WindowSDL::getFullScreenType()
{
    int flags = SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN_DESKTOP;

    if(flags == SDL_WINDOW_FULLSCREEN_DESKTOP)
        return FULLSCREEN_TYPE_DESKTOP;

    if(flags == SDL_WINDOW_FULLSCREEN)
        return FULLSCREEN_TYPE_REAL;

    return 0;
}

int WindowSDL::syncFullScreenRes()
{
    if(!m_fullscreen || m_fullscreen_type_real != SDL_WINDOW_FULLSCREEN || !m_window)
        return 0; // Nothing to do

    SDL_DisplayMode mode, modeDst, *modeClose;

    const auto defColour = s_availableColours.empty() ? 32 : s_availableColours.front();

    mode.format = s_getColorFromBits(m_curColour > 0 ? m_curColour : defColour);
    mode.w = m_curRes.w;
    mode.h = m_curRes.h;
    mode.refresh_rate = 60;
    mode.driverdata = nullptr;

    modeClose = SDL_GetClosestDisplayMode(0, &mode, &modeDst);

    pLogDebug("Requesting screen mode: C=%u (%s), W=%d, H=%d, R=%d",
              mode.format, SDL_GetPixelFormatName(mode.format), mode.w, mode.h, mode.refresh_rate);
    if(modeClose)
    {
        pLogDebug("Obtained screen mode: C=%u (%s), W=%d, H=%d, R=%d",
                  modeDst.format, SDL_GetPixelFormatName(modeDst.format), modeDst.w, modeDst.h, modeDst.refresh_rate);
    }
    else
    {
        pLogDebug("Closest mode is not available, using defaults...");
    }

    if(s_isExclusiveFullScreen(SDL_GetWindowFlags(m_window)))
    {
        SDL_SetWindowSize(m_window, modeDst.w, modeDst.h);
        pLogDebug("Toggling screen into %d x %d resolution", modeDst.w, modeDst.h);

        if(SDL_SetWindowFullscreen(m_window, m_fullscreen_type_real) < 0)
        {
            pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
            return -1;
        }
    }

    if(modeClose && SDL_SetWindowDisplayMode(m_window, &modeDst) < 0)
    {
        pLogWarning("Setting fullscreen display mode failed: %s", SDL_GetError());
        return -1;
    }

    SDL_SetWindowSize(m_window, modeDst.w, modeDst.h);

    return 0;
}

#endif // RENDER_FULLSCREEN_TYPES_SUPPORTED

void WindowSDL::restoreWindow()
{
    SDL_RestoreWindow(m_window);
}

void WindowSDL::setWindowSize(int w, int h)
{
    // doesn't make sense on Emscripten, actually causes crashes on Wii U
#if !defined(__EMSCRIPTEN__) && !defined(__WIIU__)
    // try to figure out whether requested size is bigger than the screen
    int display = SDL_GetWindowDisplayIndex(m_window);
    if(display >= 0)
    {
        SDL_Rect bounds;
        if(SDL_GetDisplayUsableBounds(display, &bounds) == 0)
        {
            if(w > bounds.w || h > bounds.h)
                return;
        }
    }

    SDL_SetWindowSize(m_window, w, h);
#else
    UNUSED(w); UNUSED(h);
#endif
}

void WindowSDL::getWindowSize(int *w, int *h)
{
    SDL_GetWindowSize(m_window, w, h);
}

bool WindowSDL::hasWindowInputFocus()
{
    if(!m_window)
        return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

bool WindowSDL::hasWindowMouseFocus()
{
    if(!m_window)
        return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_MOUSE_FOCUS) != 0;
}

bool WindowSDL::isMaximized()
{
    if(!m_window)
        return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_MAXIMIZED) != 0;
}

void WindowSDL::setTitle(const char* title)
{
    if(!m_window)
        return;

    SDL_SetWindowTitle(m_window, title);
}
