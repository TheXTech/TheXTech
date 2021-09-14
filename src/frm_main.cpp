/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_opengl.h>
#ifdef __ANDROID__
#include <SDL2/SDL_assert.h>
#endif

#include "globals.h"
#include "frame_timer.h"
#include "game_main.h"
#include "graphics.h"
#include "control/joystick.h"
#include "sound.h"
#include "editor.h"

#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <Utils/files.h>
#include <Utils/elapsed_timer.h>
#include <DirManager/dirman.h>
#include <Utils/maths.h>
#include <Graphics/graphics_funcs.h>
#include <FreeImageLite.h>
#include <chrono>
#include <fmt_time_ne.h>
#include <fmt_format_ne.h>
#include <gif.h>


#include "../version.h"

#include "video.h"
#include "frm_main.h"
#include "main/game_info.h"


VideoSettings_t g_videoSettings;


static SDL_bool IsFullScreen(SDL_Window *win)
{
    Uint32 flags = SDL_GetWindowFlags(win);
    return (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? SDL_TRUE : SDL_FALSE;
}

FrmMain::FrmMain()
{
    ScaleWidth = ScreenW;
    ScaleHeight = ScreenH;
}

SDL_Window *FrmMain::getWindow()
{
    return m_window;
}

Uint8 FrmMain::getKeyState(SDL_Scancode key)
{
    if(m_keyboardState)
        return m_keyboardState[key];
    return 0;
}

bool FrmMain::initSDL(const CmdLineSetup_t &setup)
{
    bool res = false;

    m_windowTitle = g_gameInfo.titleWindow;

    LoadLogSettings(setup.interprocess, setup.verboseLogging);
    //Write into log the application start event
    pLogDebug("<Application started>");

    m_screenshotPath = AppPathManager::screenshotsDir();
    m_gifRecordPath = AppPathManager::gifRecordsDir();

    //Initialize FreeImage
    GraphicsHelps::initFreeImage();

    if(setup.allowBgInput)
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

#if defined(__ANDROID__) || (defined(__APPLE__) && (defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_IPHONE)))
    // Restrict the landscape orientation only
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

#if defined(__ANDROID__)
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
#endif

    Uint32 sdlInitFlags = 0;
    // Prepare flags for SDL initialization
#ifndef __EMSCRIPTEN__
    sdlInitFlags |= SDL_INIT_TIMER;
#endif
    sdlInitFlags |= SDL_INIT_AUDIO;
    sdlInitFlags |= SDL_INIT_VIDEO;
    sdlInitFlags |= SDL_INIT_EVENTS;
    sdlInitFlags |= SDL_INIT_JOYSTICK;
    sdlInitFlags |= SDL_INIT_HAPTIC;
    sdlInitFlags |= SDL_INIT_GAMECONTROLLER;

    // Initialize SDL
    res = (SDL_Init(sdlInitFlags) < 0);
    m_sdlLoaded = !res;

    // Workaround: https://discourse.libsdl.org/t/26995
    setlocale(LC_NUMERIC, "C");

    const char *error = SDL_GetError();
    if(*error != '\0')
        pLogWarning("Error while SDL Initialization: %s", error);
    SDL_ClearError();

    SDL_GL_ResetAttributes();

    m_window = SDL_CreateWindow(m_windowTitle.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              ScaleWidth, ScaleHeight,
                              SDL_WINDOW_RESIZABLE |
                              SDL_WINDOW_HIDDEN |
                              SDL_WINDOW_ALLOW_HIGHDPI);

    if(m_window == nullptr)
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        return false;
    }

    if(isSdlError())
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        return false;
    }

#ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
    SDL_SetWindowMinimumSize(m_window, ScaleWidth / 2, ScaleHeight / 2);
#elif defined(__ANDROID__) // Set as small as possible
    SDL_SetWindowMinimumSize(m_window, 200, 150);
#else
    SDL_SetWindowMinimumSize(m_window, ScaleWidth, ScaleHeight);
#endif //__EMSCRIPTEN__

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

#if defined(__ANDROID__) // Use a full-screen on Android mode by default
    setFullScreen(true);
    show();
#endif


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

    pLogDebug("Init renderer settings...");

    Uint32 renderFlags = 0;
    if(g_videoSettings.renderMode == RENDER_SOFTWARE)
    {
        renderFlags = SDL_RENDERER_SOFTWARE;
        g_videoSettings.renderModeObtained = RENDER_SOFTWARE;
        pLogDebug("Using software rendering");
    }
    else if(g_videoSettings.renderMode == RENDER_ACCELERATED)
    {
        renderFlags = SDL_RENDERER_ACCELERATED;
        g_videoSettings.renderModeObtained = RENDER_ACCELERATED;
        pLogDebug("Using accelerated rendering");
    }
    else if(g_videoSettings.renderMode == RENDER_ACCELERATED_VSYNC)
    {
        renderFlags = SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC;
        g_videoSettings.renderModeObtained = RENDER_ACCELERATED_VSYNC;
        pLogDebug("Using accelerated rendering with a vertical synchronization");
    }

    m_gRenderer = SDL_CreateRenderer(m_window, -1, renderFlags | SDL_RENDERER_TARGETTEXTURE); // Try to make renderer

    if(!m_gRenderer && g_videoSettings.renderModeObtained == RENDER_ACCELERATED_VSYNC) // If was a V-Sync renderer, use non-V-Synced
    {
        pLogWarning("Failed to initialize V-Synced renderer, trying to create accelerated renderer...");
        renderFlags = SDL_RENDERER_ACCELERATED;
        g_videoSettings.renderModeObtained = RENDER_ACCELERATED;
        m_gRenderer = SDL_CreateRenderer(m_window, -1, renderFlags | SDL_RENDERER_TARGETTEXTURE);
    }

    if(!m_gRenderer && g_videoSettings.renderModeObtained == RENDER_ACCELERATED) // Fall back to software
    {
        pLogWarning("Failed to initialize accelerated renderer, trying to create a software renderer...");
        renderFlags = SDL_RENDERER_SOFTWARE;
        g_videoSettings.renderModeObtained = RENDER_SOFTWARE;
        m_gRenderer = SDL_CreateRenderer(m_window, -1, renderFlags | SDL_RENDERER_TARGETTEXTURE);
    }

    if(!m_gRenderer)
    {
        pLogCritical("Unable to create renderer!");
        freeSDL();
        return false;
    }

    SDL_GetRendererInfo(m_gRenderer, &m_ri);

    m_tBuffer = SDL_CreateTexture(m_gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, ScreenW, ScreenH);
    if(!m_tBuffer)
    {
        pLogCritical("Unable to create texture render buffer!");
        freeSDL();
        return false;
    }

    // Clean-up from a possible start-up junk
    clearBuffer();

    setTargetTexture();

    SDL_SetRenderDrawBlendMode(m_gRenderer, SDL_BLENDMODE_BLEND);

    m_keyboardState = SDL_GetKeyboardState(nullptr);

    updateViewport();

    // Clean-up the texture buffer from the same start-up junk
    clearBuffer();

    setTargetScreen();

    repaint();
    doEvents();

#ifndef __EMSCRIPTEN__
    m_gif.init();
#endif

    return res;
}

void FrmMain::freeSDL()
{
    GFX.unLoad();
    clearAllTextures();
    joyCloseJoysticks();

#ifndef __EMSCRIPTEN__
    m_gif.quit();
#endif

    if(m_tBuffer)
        SDL_DestroyTexture(m_tBuffer);
    m_tBuffer = nullptr;

    if(m_gRenderer)
        SDL_DestroyRenderer(m_gRenderer);
    m_gRenderer = nullptr;

    if(m_window)
        SDL_DestroyWindow(m_window);
    m_window = nullptr;

    SDL_Quit();
    GraphicsHelps::closeFreeImage();

    pLogDebug("<Application closed>");
    CloseLog();
}

void FrmMain::show()
{
    SDL_ShowWindow(m_window);
}

void FrmMain::hide()
{
    SDL_HideWindow(m_window);
    showCursor(1);
}

void FrmMain::doEvents()
{
    while(SDL_PollEvent(&m_event))
    {
        processEvent();
    }
}

void FrmMain::processEvent()
{
    switch(m_event.type)
    {
    case SDL_QUIT:
        showCursor(1);
        KillIt();
        break;
    case SDL_JOYDEVICEADDED:
        joyDeviceAddEvent(&m_event.jdevice);
        break;
    case SDL_JOYDEVICEREMOVED:
        joyDeviceRemoveEvent(&m_event.jdevice);
        break;
    case SDL_WINDOWEVENT:
        switch(m_event.window.event)
        {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_MOVED:
            eventResize();
            break;
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            if(!neverPause)
                SoundPauseEngine(0);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            if(!neverPause)
                SoundPauseEngine(1);
            break;
//        case SDL_WINDOWEVENT_MAXIMIZED:
//            SDL_RestoreWindow(m_window);
//            SetRes();
//            break;
#endif
        default:
            break;
        }
        break;
    case SDL_KEYDOWN:
        eventKeyDown(m_event.key);
        eventKeyPress(m_event.key.keysym.scancode);
        break;
    case SDL_KEYUP:
        eventKeyUp(m_event.key);
        break;
    case SDL_MOUSEBUTTONDOWN:
        eventMouseDown(m_event.button);
        break;
    case SDL_MOUSEBUTTONUP:
        eventMouseUp(m_event.button);
        break;
    case SDL_MOUSEMOTION:
        eventMouseMove(m_event.motion);
        break;
#ifdef __ANDROID__
    case SDL_RENDER_DEVICE_RESET:
        D_pLogDebug("Android: Render Device Reset");
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        m_blockRender = true;
        D_pLogDebug("Android: Entering background");
        break;
    case SDL_APP_DIDENTERFOREGROUND:
        m_blockRender = false;
        D_pLogDebug("Android: Resumed foreground");
        break;
#endif
    }
}

void FrmMain::waitEvents()
{
    if(SDL_WaitEventTimeout(&m_event, 1000))
        processEvent();
    doEvents();
}

bool FrmMain::isWindowActive()
{
    if(!m_window)
        return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

bool FrmMain::hasWindowMouseFocus()
{
    if(!m_window)
        return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_MOUSE_FOCUS) != 0;
}

void FrmMain::eventDoubleClick()
{
    if(MagicHand)
        return; // Don't toggle fullscreen/window when magic hand is active
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    if(resChanged)
    {
        frmMain.setFullScreen(false);
        resChanged = false;
        SDL_RestoreWindow(m_window);
        SDL_SetWindowSize(m_window, ScreenW, ScreenH);
        if(!GameMenu && !MagicHand)
            showCursor(1);
    }
    else
        SetRes();
#endif
}

void FrmMain::eventKeyDown(SDL_KeyboardEvent &evt)
{
    int KeyCode = evt.keysym.scancode;
    inputKey = KeyCode;

    bool ctrlF = ((evt.keysym.mod & KMOD_CTRL) != 0 && evt.keysym.scancode == SDL_SCANCODE_F);
    bool altEnter = ((evt.keysym.mod & KMOD_ALT) != 0 && (evt.keysym.scancode == SDL_SCANCODE_RETURN || evt.keysym.scancode == SDL_SCANCODE_KP_ENTER));

#ifndef __ANDROID__
    if(ctrlF || altEnter)
        ChangeScreen();
#endif

#ifndef __EMSCRIPTEN__
    if(KeyCode == SDL_SCANCODE_F12)
        TakeScreen = true;
    else if(KeyCode == SDL_SCANCODE_F3)
        g_stats.enabled = !g_stats.enabled;
#   ifdef __APPLE__
    else if(KeyCode == SDL_SCANCODE_F10) // Reserved by macOS as "show desktop"
#   else
    else if(KeyCode == SDL_SCANCODE_F11)
#   endif
        toggleGifRecorder();
#endif
}

void FrmMain::eventKeyPress(SDL_Scancode KeyASCII)
{
    switch(KeyASCII)
    {
    case SDL_SCANCODE_A: CheatCode('a'); break;
    case SDL_SCANCODE_B: CheatCode('b'); break;
    case SDL_SCANCODE_C: CheatCode('c'); break;
    case SDL_SCANCODE_D: CheatCode('d'); break;
    case SDL_SCANCODE_E: CheatCode('e'); break;
    case SDL_SCANCODE_F: CheatCode('f'); break;
    case SDL_SCANCODE_G: CheatCode('g'); break;
    case SDL_SCANCODE_H: CheatCode('h'); break;
    case SDL_SCANCODE_I: CheatCode('i'); break;
    case SDL_SCANCODE_J: CheatCode('j'); break;
    case SDL_SCANCODE_K: CheatCode('k'); break;
    case SDL_SCANCODE_L: CheatCode('l'); break;
    case SDL_SCANCODE_M: CheatCode('m'); break;
    case SDL_SCANCODE_N: CheatCode('n'); break;
    case SDL_SCANCODE_O: CheatCode('o'); break;
    case SDL_SCANCODE_P: CheatCode('p'); break;
    case SDL_SCANCODE_Q: CheatCode('q'); break;
    case SDL_SCANCODE_R: CheatCode('r'); break;
    case SDL_SCANCODE_S: CheatCode('s'); break;
    case SDL_SCANCODE_T: CheatCode('t'); break;
    case SDL_SCANCODE_U: CheatCode('u'); break;
    case SDL_SCANCODE_V: CheatCode('v'); break;
    case SDL_SCANCODE_W: CheatCode('w'); break;
    case SDL_SCANCODE_X: CheatCode('x'); break;
    case SDL_SCANCODE_Y: CheatCode('y'); break;
    case SDL_SCANCODE_Z: CheatCode('z'); break;
    case SDL_SCANCODE_1: CheatCode('1'); break;
    case SDL_SCANCODE_2: CheatCode('2'); break;
    case SDL_SCANCODE_3: CheatCode('3'); break;
    case SDL_SCANCODE_4: CheatCode('4'); break;
    case SDL_SCANCODE_5: CheatCode('5'); break;
    case SDL_SCANCODE_6: CheatCode('6'); break;
    case SDL_SCANCODE_7: CheatCode('7'); break;
    case SDL_SCANCODE_8: CheatCode('8'); break;
    case SDL_SCANCODE_9: CheatCode('9'); break;
    case SDL_SCANCODE_0: CheatCode('0'); break;
    default: CheatCode(' '); break;
    }
}

void FrmMain::eventKeyUp(SDL_KeyboardEvent &evt)
{
    UNUSED(evt);
}

void FrmMain::eventMouseDown(SDL_MouseButtonEvent &event)
{
    if(event.button == SDL_BUTTON_LEFT)
    {
        MenuMouseDown = true;
        MenuMouseMove = true;
        if(LevelEditor || MagicHand || TestLevel)
            EditorControls.Mouse1 = true;
    }
    else if(event.button == SDL_BUTTON_RIGHT)
    {
        MenuMouseBack = true;
        if(LevelEditor || MagicHand || TestLevel)
        {
            optCursor.current = OptCursor_t::LVL_SELECT;
            MouseMove(float(MenuMouseX), float(MenuMouseY));
            SetCursor();
        }
    }
    else if(event.button == SDL_BUTTON_MIDDLE)
    {
        if(LevelEditor || MagicHand || TestLevel)
        {
            optCursor.current = OptCursor_t::LVL_ERASER;
            MouseMove(float(MenuMouseX), float(MenuMouseY));
            SetCursor();
        }
    }
}

void FrmMain::eventMouseMove(SDL_MouseMotionEvent &event)
{
    SDL_Point p = MapToScr(event.x, event.y);
    MenuMouseX = p.x;// int(event.x * ScreenW / ScaleWidth);
    MenuMouseY = p.y;//int(event.y * ScreenH / ScaleHeight);
    MenuMouseMove = true;
    if(LevelEditor || MagicHand || TestLevel)
    {
        EditorCursor.X = CursorPos.X;
        EditorCursor.Y = CursorPos.Y;
        MouseMove(EditorCursor.X, EditorCursor.Y, true);
        MouseRelease = true;
    }
}

void FrmMain::eventMouseUp(SDL_MouseButtonEvent &event)
{
    bool doubleClick = false;
    MenuMouseDown = false;
    MenuMouseRelease = true;
    if(LevelEditor || MagicHand || TestLevel)
        EditorControls.Mouse1 = false;

    if(event.button == SDL_BUTTON_LEFT)
    {
        doubleClick = (m_lastMousePress + 300) >= SDL_GetTicks();
        m_lastMousePress = SDL_GetTicks();
    }

    if(doubleClick)
    {
        eventDoubleClick();
        m_lastMousePress = 0;
    }
}

void FrmMain::eventResize()
{
    updateViewport();
    SetupScreens();
#ifdef USE_TOUCHSCREEN_CONTROLLER
    UpdateTouchScreenSize();
#endif
}

int FrmMain::setFullScreen(bool fs)
{
    if(m_window == nullptr)
        return -1;

    if(fs != IsFullScreen(m_window))
    {
        if(fs)
        {
            // Swith to FULLSCREEN mode
            if(SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
            {
                //Hide mouse cursor in full screen mdoe
                pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
                return -1;
            }

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
#ifdef __EMSCRIPTEN__
            SDL_SetWindowSize(m_window, ScaleWidth, ScaleHeight);
#endif
            return 0;
        }
    }

    return 0;
}

bool FrmMain::isSdlError()
{
    const char *error = SDL_GetError();
    return (*error != '\0');
}

void FrmMain::repaint()
{
#ifdef __ANDROID__
    if(m_blockRender)
        return;
#endif

    int w, h, off_x, off_y, wDst, hDst;
    float scale_x, scale_y;

    setTargetScreen();

#ifndef __EMSCRIPTEN__
    processRecorder();
    drawBatteryStatus();
#endif

    // Get the size of surface where to draw the scene
    SDL_GetRendererOutputSize(m_gRenderer, &w, &h);

    // Calculate the size difference factor
    scale_x = float(w) / ScaleWidth;
    scale_y = float(h) / ScaleHeight;

    wDst = w;
    hDst = h;

    // Keep aspect ratio
    if(scale_x > scale_y) // Width more than height
    {
        wDst = int(scale_y * ScaleWidth);
        hDst = int(scale_y * ScaleHeight);
    }
    else if(scale_x < scale_y) // Height more than width
    {
        hDst = int(scale_x * ScaleHeight);
        wDst = int(scale_x * ScaleWidth);
    }

    // Align the rendering scene to the center of screen
    off_x = (w - wDst) / 2;
    off_y = (h - hDst) / 2;

    SDL_SetRenderDrawColor(m_gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(m_gRenderer);

    SDL_Rect destRect = {off_x, off_y, wDst, hDst};
    SDL_Rect sourceRect = {0, 0, ScaleWidth, ScaleHeight};

    SDL_SetTextureColorMod(m_tBuffer, 255, 255, 255);
    SDL_SetTextureAlphaMod(m_tBuffer, 255);
    SDL_RenderCopyEx(m_gRenderer, m_tBuffer, &sourceRect, &destRect, 0.0, nullptr, SDL_FLIP_NONE);

#ifdef USE_TOUCHSCREEN_CONTROLLER
    RenderTouchControls();
#endif

    SDL_RenderPresent(m_gRenderer);
}

void FrmMain::updateViewport()
{
    float w, w1, h, h1;
    int   wi, hi;

#ifndef __EMSCRIPTEN__
    SDL_GetWindowSize(m_window, &wi, &hi);
#else
    if(IsFullScreen(m_window))
    {
        SDL_GetWindowSize(m_window, &wi, &hi);
    }
    else
    {
        wi = ScreenW;
        hi = ScreenH;
    }
#endif

    w = wi;
    h = hi;
    w1 = w;
    h1 = h;

    scale_x = w / ScaleWidth;
    scale_y = h / ScaleHeight;
    viewport_scale_x = scale_x;
    viewport_scale_y = scale_y;

    viewport_offset_x = 0;
    viewport_offset_y = 0;

    if(scale_x > scale_y)
    {
        w1 = scale_y * ScaleWidth;
        viewport_scale_x = w1 / ScaleWidth;
    }
    else if(scale_x < scale_y)
    {
        h1 = scale_x * ScaleHeight;
        viewport_scale_y = h1 / ScaleHeight;
    }

    offset_x = (w - w1) / 2;
    offset_y = (h - h1) / 2;

    viewport_x = 0;
    viewport_y = 0;
    viewport_w = static_cast<int>(w1);
    viewport_h = static_cast<int>(h1);
}

void FrmMain::resetViewport()
{
    updateViewport();
    SDL_RenderSetViewport(m_gRenderer, nullptr);
}

void FrmMain::setViewport(int x, int y, int w, int h)
{
    SDL_Rect topLeftViewport = {x, y, w, h};
    SDL_RenderSetViewport(m_gRenderer, &topLeftViewport);

    viewport_x = x;
    viewport_y = y;
    viewport_w = w;
    viewport_h = h;
}

void FrmMain::offsetViewport(int x, int y)
{
    if(viewport_offset_x != x || viewport_offset_y != y)
    {
        viewport_offset_x = x;
        viewport_offset_y = y;
    }
}

void FrmMain::setTargetTexture()
{
    if(m_recentTarget == m_tBuffer)
        return;
    SDL_SetRenderTarget(m_gRenderer, m_tBuffer);
    m_recentTarget = m_tBuffer;
}

void FrmMain::setTargetScreen()
{
    if(m_recentTarget == nullptr)
        return;
    SDL_SetRenderTarget(m_gRenderer, nullptr);
    m_recentTarget = nullptr;
}

StdPicture FrmMain::LoadPicture(std::string path, std::string maskPath, std::string maskFallbackPath)
{
    StdPicture target;
    FIBITMAP *sourceImage;

    if(!GameIsActive)
        return target; // do nothing when game is closed

    if(path.empty())
        return target;

#ifdef DEBUG_BUILD
    target.origPath = path;
#endif

    sourceImage = GraphicsHelps::loadImage(path);

    // Don't load mask if PNG image is used
    if(Files::hasSuffix(path, ".png"))
    {
        maskPath.clear();
        maskFallbackPath.clear();
    }

    if(!sourceImage)
    {
        pLogWarning("Error loading of image file:\n"
                    "%s\n"
                    "Reason: %s.",
                    path.c_str(),
                    (Files::fileExists(path) ? "wrong image format" : "file not exist"));
        // target = g_renderer->getDummyTexture();
        return target;
    }

#ifdef DEBUG_BUILD
    ElapsedTimer totalTime;
    ElapsedTimer maskMergingTime;
    ElapsedTimer bindingTime;
    ElapsedTimer unloadTime;
    totalTime.start();
    int64_t maskElapsed = 0;
    int64_t bindElapsed = 0;
    int64_t unloadElapsed = 0;
#endif

    //Apply Alpha mask
    if(!maskPath.empty() && Files::fileExists(maskPath))
    {
#ifdef DEBUG_BUILD
        maskMergingTime.start();
#endif
        GraphicsHelps::mergeWithMask(sourceImage, maskPath);
#ifdef DEBUG_BUILD
        maskElapsed = maskMergingTime.nanoelapsed();
#endif
    }
    else if(!maskFallbackPath.empty())
    {
#ifdef DEBUG_BUILD
        maskMergingTime.start();
#endif
        GraphicsHelps::mergeWithMask(sourceImage, "", maskFallbackPath);
#ifdef DEBUG_BUILD
        maskElapsed = maskMergingTime.nanoelapsed();
#endif
    }

    uint32_t w = static_cast<uint32_t>(FreeImage_GetWidth(sourceImage));
    uint32_t h = static_cast<uint32_t>(FreeImage_GetHeight(sourceImage));

    if((w == 0) || (h == 0))
    {
        FreeImage_Unload(sourceImage);
        pLogWarning("Error loading of image file:\n"
                    "%s\n"
                    "Reason: %s.",
                    path.c_str(),
                    "Zero image size!");
        //target = g_renderer->getDummyTexture();
        return target;
    }

#ifdef DEBUG_BUILD
    bindingTime.start();
#endif
    RGBQUAD upperColor;
    FreeImage_GetPixelColor(sourceImage, 0, 0, &upperColor);
    target.ColorUpper.r = float(upperColor.rgbRed) / 255.0f;
    target.ColorUpper.b = float(upperColor.rgbBlue) / 255.0f;
    target.ColorUpper.g = float(upperColor.rgbGreen) / 255.0f;
    RGBQUAD lowerColor;
    FreeImage_GetPixelColor(sourceImage, 0, static_cast<unsigned int>(h - 1), &lowerColor);
    target.ColorLower.r = float(lowerColor.rgbRed) / 255.0f;
    target.ColorLower.b = float(lowerColor.rgbBlue) / 255.0f;
    target.ColorLower.g = float(lowerColor.rgbGreen) / 255.0f;
    FreeImage_FlipVertical(sourceImage);
    target.nOfColors = GL_RGBA;
    target.format = GL_BGRA;
    target.w = static_cast<int>(w);
    target.h = static_cast<int>(h);
    target.frame_w = static_cast<int>(w);
    target.frame_h = static_cast<int>(h);
    GLubyte *textura = reinterpret_cast<GLubyte *>(FreeImage_GetBits(sourceImage));
    loadTexture(target, w, h, textura);
#ifdef DEBUG_BUILD
    bindElapsed = bindingTime.nanoelapsed();
    unloadTime.start();
#endif
    //SDL_FreeSurface(sourceImage);
    GraphicsHelps::closeImage(sourceImage);
#ifdef DEBUG_BUILD
    unloadElapsed = unloadTime.nanoelapsed();
#endif
#ifdef DEBUG_BUILD
    pLogDebug("Mask merging of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(maskElapsed));
    pLogDebug("Binding time of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(bindElapsed));
    pLogDebug("Unload time of %s passed in %d nanoseconds", path.c_str(), static_cast<int>(unloadElapsed));
    pLogDebug("Total Loading of texture %s passed in %d nanoseconds (%dx%d)",
              path.c_str(),
              static_cast<int>(totalTime.nanoelapsed()),
              static_cast<int>(w),
              static_cast<int>(h));
#endif

    return target;
}

static void dumpFullFile(std::vector<char> &dst, const std::string &path)
{
    dst.clear();
    SDL_RWops *f;

    f = SDL_RWFromFile(path.c_str(), "rb");
    if(!f)
        return;

    Sint64 fSize = SDL_RWsize(f);
    if(fSize < 0)
    {
        pLogWarning("Failed to get size of the file: %s", path.c_str());
        SDL_RWclose(f);
        return;
    }

    dst.resize(size_t(fSize));
    if(SDL_RWread(f, dst.data(), 1, fSize) != size_t(fSize))
    {
        pLogWarning("Failed to dump file on read operation: %s", path.c_str());
    }

    SDL_RWclose(f);
}

StdPicture FrmMain::lazyLoadPicture(std::string path, std::string maskPath, std::string maskFallbackPath)
{
    StdPicture target;
    PGE_Size tSize;
    if(!GameIsActive)
        return target; // do nothing when game is closed

    if(path.empty())
        return target;

#ifdef DEBUG_BUILD
    target.origPath = path;
#endif

    // Don't load mask if PNG image is used
    if(Files::hasSuffix(path, ".png"))
    {
        maskPath.clear();
        maskFallbackPath.clear();
    }

    if(!GraphicsHelps::getImageMetrics(path, &tSize))
    {
        pLogWarning("Error loading of image file:\n"
                    "%s\n"
                    "Reason: %s.",
                    path.c_str(),
                    (Files::fileExists(path) ? "wrong image format" : "file not exist"));
        // target = g_renderer->getDummyTexture();
        return target;
    }

    target.w = tSize.w();
    target.h = tSize.h();

    dumpFullFile(target.raw, path);

    //Apply Alpha mask
    if(!maskPath.empty() && Files::fileExists(maskPath))
    {
        dumpFullFile(target.rawMask, maskPath);
        target.isMaskPng = false;
    }
    else if(!maskFallbackPath.empty())
    {
        dumpFullFile(target.rawMask, maskFallbackPath);
        target.isMaskPng = true;
    }

    target.inited = true;
    target.lazyLoaded = true;
    target.texture = nullptr;

    return target;
}

void FrmMain::loadTexture(StdPicture &target, uint32_t width, uint32_t height, uint8_t *RGBApixels)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    surface = SDL_CreateRGBSurfaceFrom(RGBApixels,
                                       static_cast<int>(width),
                                       static_cast<int>(height),
                                       32,
                                       static_cast<int>(width * 4),
                                       FI_RGBA_RED_MASK,
                                       FI_RGBA_GREEN_MASK,
                                       FI_RGBA_BLUE_MASK,
                                       FI_RGBA_ALPHA_MASK);
    texture = SDL_CreateTextureFromSurface(m_gRenderer, surface);
    if(!texture)
    {
        pLogWarning("Render SW-SDL: Failed to load texture! (%s)", SDL_GetError());
        SDL_FreeSurface(surface);
        target.texture = nullptr;
        target.inited = false;
        return;
    }

    SDL_FreeSurface(surface);

    target.texture = texture;
    m_textureBank.insert(texture);

    target.inited = true;
}

void FrmMain::lazyLoad(StdPicture &target)
{
    if(!target.inited || !target.lazyLoaded || target.texture)
        return;

    FIBITMAP *sourceImage = GraphicsHelps::loadImage(target.raw);
    if(!sourceImage)
    {
        pLogCritical("Lazy-decompress has failed: invalid image data");
        return;
    }

    if(!target.rawMask.empty())
        GraphicsHelps::mergeWithMask(sourceImage, target.rawMask, target.isMaskPng);

    uint32_t w = static_cast<uint32_t>(FreeImage_GetWidth(sourceImage));
    uint32_t h = static_cast<uint32_t>(FreeImage_GetHeight(sourceImage));

    if((w == 0) || (h == 0))
    {
        GraphicsHelps::closeImage(sourceImage);
        pLogWarning("Error lazy-decompressing of image file:\n"
                    "Reason: %s."
                    "Zero image size!");
        //target = g_renderer->getDummyTexture();
        return;
    }

    m_lazyLoadedBytes += (w * h * 4);
    if(!target.rawMask.empty())
        m_lazyLoadedBytes += (w * h * 4);

    RGBQUAD upperColor;
    FreeImage_GetPixelColor(sourceImage, 0, 0, &upperColor);
    target.ColorUpper.r = float(upperColor.rgbRed) / 255.0f;
    target.ColorUpper.b = float(upperColor.rgbBlue) / 255.0f;
    target.ColorUpper.g = float(upperColor.rgbGreen) / 255.0f;
    RGBQUAD lowerColor;
    FreeImage_GetPixelColor(sourceImage, 0, static_cast<unsigned int>(h - 1), &lowerColor);
    target.ColorLower.r = float(lowerColor.rgbRed) / 255.0f;
    target.ColorLower.b = float(lowerColor.rgbBlue) / 255.0f;
    target.ColorLower.g = float(lowerColor.rgbGreen) / 255.0f;
    FreeImage_FlipVertical(sourceImage);
    target.nOfColors = GL_RGBA;
    target.format = GL_BGRA;
    target.w = static_cast<int>(w);
    target.h = static_cast<int>(h);
    target.frame_w = static_cast<int>(w);
    target.frame_h = static_cast<int>(h);

    bool shrink2x = false;

    if(g_videoSettings.scaleDownAllTextures || GraphicsHelps::validateFor2xScaleDown(sourceImage, StdPictureGetOrigPath(target)))
    {
        target.w_orig = int(w);
        target.h_orig = int(h);
        w /= 2;
        h /= 2;
        shrink2x = true;
    }

    bool wLimitExcited = m_ri.max_texture_width > 0 && w > Uint32(m_ri.max_texture_width);
    bool hLimitExcited = m_ri.max_texture_height > 0 && h > Uint32(m_ri.max_texture_height);

    if(wLimitExcited || hLimitExcited || shrink2x)
    {
        if(!shrink2x)
        {
            target.w_orig = int(w);
            target.h_orig = int(h);
        }

        // WORKAROUND: down-scale too big textures
        if(w > Uint32(m_ri.max_texture_width))
            w = Uint32(m_ri.max_texture_width);
        if(h > Uint32(m_ri.max_texture_height))
            h = Uint32(m_ri.max_texture_height);

        if(wLimitExcited || hLimitExcited)
        {
            pLogWarning("Texture is too big for a given hardware limit (%dx%d). "
                        "Shrinking texture to %dx%d, quality may be distorted!",
                        m_ri.max_texture_width, m_ri.max_texture_height,
                        w, h);
        }

        FIBITMAP *d = FreeImage_Rescale(sourceImage, int(w), int(h), FILTER_BOX);
        if(d)
        {
            GraphicsHelps::closeImage(sourceImage);
            sourceImage = d;
        }
        target.w_scale = float(w) / float(target.w_orig);
        target.h_scale = float(h) / float(target.h_orig);
    }

    GLubyte *textura = reinterpret_cast<GLubyte *>(FreeImage_GetBits(sourceImage));
    loadTexture(target, w, h, textura);

    GraphicsHelps::closeImage(sourceImage);
}

void FrmMain::lazyUnLoad(StdPicture &target)
{
    if(!target.inited || !target.lazyLoaded || !target.texture)
        return;
    deleteTexture(target, true);
}

void FrmMain::lazyPreLoad(StdPicture &target)
{
    if(!target.texture && target.lazyLoaded)
        lazyLoad(target);
}

size_t FrmMain::lazyLoadedBytes()
{
    return m_lazyLoadedBytes;
}

void FrmMain::lazyLoadedBytesReset()
{
    m_lazyLoadedBytes = 0;
}

#ifdef __ANDROID__
bool FrmMain::renderBlocked()
{
    return m_blockRender;
}
#endif

#ifndef __EMSCRIPTEN__

void FrmMain::makeShot()
{
    if(!m_gRenderer || !m_tBuffer)
        return;

    const int w = ScreenW, h = ScreenH;
    uint8_t *pixels = new uint8_t[size_t(4 * w * h)];
    getScreenPixelsRGBA(0, 0, w, h, pixels);
    PGE_GL_shoot *shoot = new PGE_GL_shoot();
    shoot->pixels = pixels;
    shoot->w = w;
    shoot->h = h;
    shoot->pitch = w * 4;
    shoot->me = this;

#ifndef PGE_NO_THREADING
    m_screenshot_thread = SDL_CreateThread(makeShot_action, "scrn_maker", reinterpret_cast<void *>(shoot));
#else
    makeShot_action(reinterpret_cast<void *>(shoot));
#endif

}

#ifndef __EMSCRIPTEN__
void FrmMain::drawBatteryStatus()
{
    int secs, pct, status;
    // Battery status
    int bw = 40;
    int bh = 22;
    int bx = ScreenW - (bw + 8);
    int by = 24;
    int segmentsFullLen = 14;
    int segments = 0;
    float alhpa = 0.7f;
    float alhpaB = 0.8f;
    float r = 0.4f, g = 0.4f, b = 0.4f;
    float br = 0.0f, bg = 0.0f, bb = 0.0f;
    bool isLow = false;

#ifndef __ANDROID__
    const bool isFullScreen = resChanged;
#endif

    if(g_videoSettings.batteryStatus == BATTERY_STATUS_OFF)
        return;

    status = SDL_GetPowerInfo(&secs, &pct);

    if(status == SDL_POWERSTATE_NO_BATTERY || status == SDL_POWERSTATE_UNKNOWN)
        return;

    isLow = (pct <= 35);

    if(status == SDL_POWERSTATE_CHARGED)
    {
        br = 0.f;
        bg = 1.f;
        bb = 0.f;
    }
    else if(status == SDL_POWERSTATE_CHARGING)
    {
        br = 1.f;
        bg = 0.64f;
        bb = 0.f;
    }
    else if(isLow)
        br = 1.f;

    segments = ((pct * segmentsFullLen) / 100) * 2;
    if(segments == 0)
        segments = 2;

    bool showBattery = false;

    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_ALWAYS_ON);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_ANY_WHEN_LOW && isLow);
#ifndef __ANDROID__
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_WHEN_LOW && isLow && isFullScreen);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_ON && isFullScreen);
#else
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_WHEN_LOW && isLow);
    showBattery |= (g_videoSettings.batteryStatus == BATTERY_STATUS_FULLSCREEN_ON);
#endif

    if(showBattery)
    {
        setTargetTexture();

        frmMain.renderRect(bx, by, bw - 4, bh, 0.f, 0.f, 0.f, alhpa, true);//Edge
        frmMain.renderRect(bx + 2, by + 2, bw - 8, bh - 4, r, g, b, alhpa, true);//Box
        frmMain.renderRect(bx + 36, by + 6, 4, 10, 0.f, 0.f, 0.f, alhpa, true);//Edge
        frmMain.renderRect(bx + 34, by + 8, 4, 6, r, g, b, alhpa, true);//Box
        frmMain.renderRect(bx + 4, by + 4, segments, 14, br, bg, bb, alhpaB / 2.f, true);//Level

        setTargetScreen();
    }
}
#endif

static std::string shoot_getTimedString(std::string path, const char *ext = "png")
{
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm t = fmt::localtime_ne(in_time_t);
    static int prevSec = 0;
    static int prevSecCounter = 0;
    if(prevSec != t.tm_sec)
    {
        prevSec = t.tm_sec;
        prevSecCounter = 0;
    }
    else
        prevSecCounter++;

    if(!prevSecCounter)
    {
        return fmt::sprintf_ne("%s/Scr_%04d-%02d-%02d_%02d-%02d-%02d.%s",
                               path,
                               (1900 + t.tm_year), (1 + t.tm_mon), t.tm_mday,
                               t.tm_hour, t.tm_min, t.tm_sec,
                               ext);
    }
    else
    {
        return fmt::sprintf_ne("%s/Scr_%04d-%02d-%02d_%02d-%02d-%02d_(%d).%s",
                               path,
                               (1900 + t.tm_year), (1 + t.tm_mon), t.tm_mday,
                               t.tm_hour, t.tm_min, t.tm_sec,
                               prevSecCounter,
                               ext);
    }
}

void FrmMain::GifRecorder::init()
{
    mutex = SDL_CreateMutex();
}

void FrmMain::GifRecorder::quit()
{
    if(enabled)
    {
        enabled = false;
        doFinalize = true;
        if(worker) // Let worker complete it's mad job
            SDL_WaitThread(worker, nullptr);
        worker = nullptr; // and only then, quit a thing
    }

    SDL_DestroyMutex(mutex);
    mutex = nullptr;
}

void FrmMain::GifRecorder::drawRecCircle()
{
    if(fadeForward)
    {
        fadeValue += 0.01f;
        if(fadeValue >= 1.0f)
        {
            fadeValue = 1.0f;
            fadeForward = !fadeForward;
        }
    }
    else
    {
        fadeValue -= 0.01f;
        if(fadeValue < 0.5f)
        {
            fadeValue = 0.5f;
            fadeForward = !fadeForward;
        }
    }

    if(doFinalize)
    {
        frmMain.renderCircle(50, 50, 20, 0.f, 0.6f, 0.f, fadeValue, true);
        SuperPrint("SAVING", 3, 2, 80, 0.f, 0.6f, 0.f, fadeValue);
    }
    else
    {
        frmMain.renderCircle(50, 50, 20, 1.f, 0.f, 0.f, fadeValue, true);
        SuperPrint("REC", 3, 25, 80, 1.f, 0.f, 0.f, fadeValue);
    }
}

bool FrmMain::GifRecorder::hasSome()
{
    SDL_LockMutex(mutex);
    bool ret = !queue.empty();
    SDL_UnlockMutex(mutex);
    return ret;
}

void FrmMain::GifRecorder::enqueue(const FrmMain::PGE_GL_shoot &entry)
{
    SDL_LockMutex(mutex);
    queue.push_back(entry);
    SDL_UnlockMutex(mutex);
}

FrmMain::PGE_GL_shoot FrmMain::GifRecorder::dequeue()
{
    SDL_LockMutex(mutex);
    PGE_GL_shoot ret = queue.front();
    queue.pop_front();
    SDL_UnlockMutex(mutex);
    return ret;
}

bool FrmMain::recordInProcess()
{
    return m_gif.enabled;
}

void FrmMain::toggleGifRecorder()
{
    UNUSED(GIF_H::GifOverwriteLastDelay);// shut up a warning about unused function

    if(!m_gif.enabled)
    {
        if(!DirMan::exists(m_gifRecordPath))
            DirMan::mkAbsPath(m_gifRecordPath);

        std::string saveTo = shoot_getTimedString(m_gifRecordPath, "gif");

        if(m_gif.worker)
            SDL_WaitThread(m_gif.worker, nullptr);
        m_gif.worker = nullptr;

        FILE *gifFile = Files::utf8_fopen(saveTo.data(), "wb");
        if(GIF_H::GifBegin(&m_gif.writer, gifFile, ScreenW, ScreenH, m_gif.delay, false))
        {
            m_gif.enabled = true;
            m_gif.doFinalize = false;
            PlaySound(SFX_PlayerGrow);
        }

        m_gif.worker = SDL_CreateThread(processRecorder_action, "gif_recorder", reinterpret_cast<void *>(&m_gif));
    }
    else
    {
        if(!m_gif.doFinalize)
        {
            m_gif.doFinalize = true;
            PlaySound(SFX_PlayerShrink);
        }
        else
        {
            PlaySound(SFX_BlockHit);
        }
    }
}

void FrmMain::processRecorder()
{
    if(!m_gif.enabled)
        return;

    setTargetTexture();

    m_gif.delayTimer += int(1000.0 / 65.0);
    if(m_gif.delayTimer >= m_gif.delay * 10)
        m_gif.delayTimer = 0.0;
    if(m_gif.doFinalize || (m_gif.delayTimer != 0.0))
    {
        m_gif.drawRecCircle();
        setTargetScreen();
        return;
    }

    const int w = ScreenW, h = ScreenH;

    uint8_t *pixels = reinterpret_cast<uint8_t*>(SDL_malloc(size_t(4 * w * h) + 42));
    if(!pixels)
    {
        pLogCritical("Can't allocate memory for a next GIF frame: out of memory");
        setTargetScreen();
        return; // Drop frame (out of memory)
    }

    getScreenPixelsRGBA(0, 0, w, h, pixels);

    PGE_GL_shoot shoot;
    shoot.pixels = pixels;
    shoot.w = w;
    shoot.h = h;
    shoot.pitch = w * 4;
    shoot.me = this;

    m_gif.enqueue(shoot);

    m_gif.drawRecCircle();
    setTargetScreen();
}

int FrmMain::processRecorder_action(void *_recorder)
{
    GifRecorder *recorder = reinterpret_cast<GifRecorder *>(_recorder);

    while(true)
    {
        if(!recorder->hasSome()) // Wait for a next frame
        {
            if(recorder->doFinalize)
                break;
            SDL_Delay(1);
            continue;
        }

        PGE_GL_shoot sh = recorder->dequeue();
        GifWriteFrame(&recorder->writer, sh.pixels,
                      unsigned(sh.w),
                      unsigned(sh.h),
                      recorder->delay/*uint32_t((ticktime)/10.0)*/, 8, false);
        SDL_free(sh.pixels);
        sh.pixels = nullptr;
    }

    // Once GIF recorder was been disabled, finalize it
    GIF_H::GifEnd(&recorder->writer);
    recorder->worker = nullptr;
    recorder->enabled = false;

    return 0;
}

int FrmMain::makeShot_action(void *_pixels)
{
    PGE_GL_shoot *shoot = reinterpret_cast<PGE_GL_shoot *>(_pixels);
    FrmMain *me = shoot->me;
    FIBITMAP *shotImg = FreeImage_AllocateT(FIT_BITMAP, shoot->w, shoot->h, 32);

    if(!shotImg)
    {
        delete []shoot->pixels;
        shoot->pixels = nullptr;
        delete shoot;
        me->m_screenshot_thread = nullptr;
        return 0;
    }

    uint8_t *px = shoot->pixels;
    unsigned w = unsigned(shoot->w), x = 0;
    unsigned h = unsigned(shoot->h), y = 0;
    RGBQUAD p;

    for(y = 0; y < h; ++y)
    {
        for(x = 0; x < w; ++x)
        {
            p.rgbRed = px[0];
            p.rgbGreen = px[1];
            p.rgbBlue = px[2];
            p.rgbReserved = px[3];
            FreeImage_SetPixelColor(shotImg, x, (h - 1) - y, &p);
            px += 4;
        }
    }

    if(!DirMan::exists(me->m_screenshotPath))
        DirMan::mkAbsPath(me->m_screenshotPath);

    std::string saveTo = shoot_getTimedString(me->m_screenshotPath, "png");
    pLogDebug("%s %d %d", saveTo.c_str(), shoot->w, shoot->h);

    if(FreeImage_HasPixels(shotImg) == FALSE)
        pLogWarning("Can't save screenshot: no pixel data!");
    else
    {
        BOOL ret = FreeImage_Save(FIF_PNG, shotImg, saveTo.data(), PNG_Z_BEST_COMPRESSION);
        if(!ret)
        {
            pLogWarning("Failed to save screenshot!");
            Files::deleteFile(saveTo);
        }
    }

    FreeImage_Unload(shotImg);
    delete []shoot->pixels;
    shoot->pixels = nullptr;
    delete shoot;

    me->m_screenshot_thread = nullptr;
    return 0;
}

#endif // __EMSCRIPTEN__


SDL_Point FrmMain::MapToScr(int x, int y)
{
    return {
        static_cast<int>((static_cast<float>(x) - offset_x) / viewport_scale_x),
        static_cast<int>((static_cast<float>(y) - offset_y) / viewport_scale_y)
    };
}

void FrmMain::deleteTexture(StdPicture &tx, bool lazyUnload)
{
    if(!tx.inited || !tx.texture)
    {
        if(!lazyUnload)
            tx.inited = false;
        return;
    }

    if(!tx.texture)
    {
        if(!lazyUnload)
            tx.inited = false;
        return;
    }

    auto corpseIt = m_textureBank.find(tx.texture);
    if(corpseIt == m_textureBank.end())
    {
        if(tx.texture)
            SDL_DestroyTexture(tx.texture);
        tx.texture = nullptr;
        if(!lazyUnload)
            tx.inited = false;
        return;
    }

    SDL_Texture *corpse = *corpseIt;
    if(corpse)
        SDL_DestroyTexture(corpse);
    m_textureBank.erase(corpse);

    tx.texture = nullptr;
    if(!lazyUnload)
        tx.inited = false;

    if(!lazyUnload)
    {
        tx.raw.clear();
        tx.rawMask.clear();
        tx.lazyLoaded = false;
        tx.isMaskPng = false;
        tx.w = 0;
        tx.h = 0;
        tx.frame_w = 0;
        tx.frame_h = 0;
    }
    tx.format = 0;
    tx.nOfColors = 0;
    tx.ColorUpper.r = 0;
    tx.ColorUpper.g = 0;
    tx.ColorUpper.b = 0;
    tx.ColorLower.r = 0;
    tx.ColorLower.g = 0;
    tx.ColorLower.b = 0;
}

void FrmMain::clearAllTextures()
{
    for(SDL_Texture *tx : m_textureBank)
        SDL_DestroyTexture(tx);
    m_textureBank.clear();
}

void FrmMain::clearBuffer()
{
#ifdef __ANDROID__
    SDL_assert(!m_blockRender);
#endif
    SDL_SetRenderDrawColor(m_gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(m_gRenderer);
}

void FrmMain::renderRect(int x, int y, int w, int h, float red, float green, float blue, float alpha, bool filled)
{
#ifdef __ANDROID__
    SDL_assert(!m_blockRender);
#endif
    SDL_Rect aRect = {x + viewport_offset_x,
                      y + viewport_offset_y,
                      w, h};
    SDL_SetRenderDrawColor(m_gRenderer,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue),
                           static_cast<unsigned char>(255.f * alpha)
                          );

    if(filled)
        SDL_RenderFillRect(m_gRenderer, &aRect);
    else
        SDL_RenderDrawRect(m_gRenderer, &aRect);
}

void FrmMain::renderRectBR(int _left, int _top, int _right, int _bottom, float red, float green, float blue, float alpha)
{
#ifdef __ANDROID__
    SDL_assert(!m_blockRender);
#endif
    SDL_Rect aRect = {_left + viewport_offset_x,
                      _top + viewport_offset_y,
                      _right - _left, _bottom - _top};
    SDL_SetRenderDrawColor(m_gRenderer,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue),
                           static_cast<unsigned char>(255.f * alpha)
                          );
    SDL_RenderFillRect(m_gRenderer, &aRect);
}

void FrmMain::renderCircle(int cx, int cy, int radius, float red, float green, float blue, float alpha, bool filled)
{
#ifdef __ANDROID__
    SDL_assert(!m_blockRender);
#endif
    UNUSED(filled);

    SDL_SetRenderDrawColor(m_gRenderer,
                               static_cast<unsigned char>(255.f * red),
                               static_cast<unsigned char>(255.f * green),
                               static_cast<unsigned char>(255.f * blue),
                               static_cast<unsigned char>(255.f * alpha)
                          );

    cx += viewport_offset_x;
    cy += viewport_offset_y;

    for(double dy = 1; dy <= radius; dy += 1.0)
    {
        double dx = std::floor(std::sqrt((2.0 * radius * dy) - (dy * dy)));
        SDL_RenderDrawLine(m_gRenderer,
                           int(cx - dx),
                           int(cy + dy - radius),
                           int(cx + dx),
                           int(cy + dy - radius));
        if(dy < radius) // Don't cross lines
        {
            SDL_RenderDrawLine(m_gRenderer,
                               int(cx - dx),
                               int(cy - dy + radius),
                               int(cx + dx),
                               int(cy - dy + radius));
        }
    }
}

void FrmMain::renderTextureI(int xDst, int yDst, int wDst, int hDst,
                             StdPicture &tx,
                             int xSrc, int ySrc,
                             double rotateAngle, SDL_Point *center, unsigned int flip,
                             float red, float green, float blue, float alpha)
{
#ifdef __ANDROID__
    SDL_assert(!m_blockRender);
#endif
    if(!tx.inited)
        return;

    if(!tx.texture && tx.lazyLoaded)
        lazyLoad(tx);

    if(!tx.texture)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

    SDL_assert_release(tx.texture);

    // Don't go more than size of texture
    if(xSrc + wDst > tx.w)
    {
        wDst = tx.w - xSrc;
        if(wDst < 0)
            wDst = 0;
    }
    if(ySrc + hDst > tx.h)
    {
        hDst = tx.h - ySrc;
        if(hDst < 0)
            hDst = 0;
    }

    SDL_Rect destRect = {xDst + viewport_offset_x, yDst + viewport_offset_y, wDst, hDst};
    SDL_Rect sourceRect;
    if(tx.w_orig == 0 && tx.h_orig == 0)
        sourceRect = {xSrc, ySrc, wDst, hDst};
    else
        sourceRect = {int(tx.w_scale * xSrc), int(tx.h_scale * ySrc), int(tx.w_scale * wDst), int(tx.h_scale * hDst)};

    SDL_SetTextureColorMod(tx.texture,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue));
    SDL_SetTextureAlphaMod(tx.texture, static_cast<unsigned char>(255.f * alpha));
    SDL_RenderCopyEx(m_gRenderer, tx.texture, &sourceRect, &destRect,
                     rotateAngle, center, static_cast<SDL_RendererFlip>(flip));
}

void FrmMain::renderTextureScaleI(int xDst, int yDst, int wDst, int hDst,
                             StdPicture &tx,
                             int xSrc, int ySrc,
                             int wSrc, int hSrc,
                             double rotateAngle, SDL_Point *center, unsigned int flip,
                             float red, float green, float blue, float alpha)
{
#ifdef __ANDROID__
    SDL_assert(!m_blockRender);
#endif
    if(!tx.inited)
        return;

    if(!tx.texture && tx.lazyLoaded)
        lazyLoad(tx);

    if(!tx.texture)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

    SDL_assert_release(tx.texture);

    // Don't go more than size of texture
    if(xSrc + wSrc > tx.w)
    {
        wSrc = tx.w - xSrc;
        if(wSrc < 0)
            wSrc = 0;
    }
    if(ySrc + hSrc > tx.h)
    {
        hSrc = tx.h - ySrc;
        if(hSrc < 0)
            hSrc = 0;
    }

    SDL_Rect destRect = {xDst + viewport_offset_x, yDst + viewport_offset_y, wDst, hDst};
    SDL_Rect sourceRect;
    if(tx.w_orig == 0 && tx.h_orig == 0)
        sourceRect = {xSrc, ySrc, wSrc, hSrc};
    else
        sourceRect = {int(tx.w_scale * xSrc), int(tx.h_scale * ySrc), int(tx.w_scale * wSrc), int(tx.h_scale * hSrc)};

    SDL_SetTextureColorMod(tx.texture,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue));
    SDL_SetTextureAlphaMod(tx.texture, static_cast<unsigned char>(255.f * alpha));
    SDL_RenderCopyEx(m_gRenderer, tx.texture, &sourceRect, &destRect,
                     rotateAngle, center, static_cast<SDL_RendererFlip>(flip));
}

void FrmMain::renderTexture(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            float red, float green, float blue, float alpha)
{
    const unsigned int flip = SDL_FLIP_NONE;
    renderTextureI(Maths::iRound(xDst),
                   Maths::iRound(yDst),
                   Maths::iRound(wDst),
                   Maths::iRound(hDst),
                   tx,
                   xSrc,
                   ySrc,
                   0.0, nullptr, flip,
                   red, green, blue, alpha);
}

void FrmMain::renderTextureScale(double xDst, double yDst, double wDst, double hDst,
                            StdPicture &tx,
                            int xSrc, int ySrc,
                            int wSrc, int hSrc,
                            float red, float green, float blue, float alpha)
{
    const unsigned int flip = SDL_FLIP_NONE;
    renderTextureScaleI(Maths::iRound(xDst),
                        Maths::iRound(yDst),
                        Maths::iRound(wDst),
                        Maths::iRound(hDst),
                        tx,
                        xSrc, ySrc,
                        wSrc, hSrc,
                        0.0, nullptr, flip,
                        red, green, blue, alpha);
}

void FrmMain::renderTextureFL(double xDst, double yDst, double wDst, double hDst,
                              StdPicture &tx,
                              int xSrc, int ySrc,
                              double rotateAngle, SDL_Point *center, unsigned int flip,
                              float red, float green, float blue, float alpha)
{
    renderTextureI(Maths::iRound(xDst),
                   Maths::iRound(yDst),
                   Maths::iRound(wDst),
                   Maths::iRound(hDst),
                   tx,
                   xSrc,
                   ySrc,
                   rotateAngle, center, flip,
                   red, green, blue, alpha);
}

void FrmMain::renderTexture(int xDst, int yDst, StdPicture &tx, float red, float green, float blue, float alpha)
{
#ifdef __ANDROID__
    SDL_assert(!m_blockRender);
#endif
    const unsigned int flip = SDL_FLIP_NONE;

    if(!tx.inited)
        return;

    if(!tx.texture && tx.lazyLoaded)
        lazyLoad(tx);

    if(!tx.texture)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

    SDL_Rect destRect = {xDst, yDst, tx.w, tx.h};
    SDL_Rect sourceRect;
    if(tx.w_orig == 0 && tx.h_orig == 0)
        sourceRect = {0, 0, tx.w, tx.h};
    else
        sourceRect = {0, 0, tx.w_orig, tx.h_orig};

    SDL_SetTextureColorMod(tx.texture,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue));
    SDL_SetTextureAlphaMod(tx.texture, static_cast<unsigned char>(255.f * alpha));
    SDL_RenderCopyEx(m_gRenderer, tx.texture, &sourceRect, &destRect,
                     0.0, nullptr, static_cast<SDL_RendererFlip>(flip));
}

void FrmMain::renderTextureScale(int xDst, int yDst, int wDst, int hDst, StdPicture &tx, float red, float green, float blue, float alpha)
{
#ifdef __ANDROID__
    SDL_assert(!m_blockRender);
#endif
    const unsigned int flip = SDL_FLIP_NONE;

    if(!tx.inited)
        return;

    if(!tx.texture && tx.lazyLoaded)
        lazyLoad(tx);

    if(!tx.texture)
    {
        D_pLogWarningNA("Attempt to render an empty texture!");
        return;
    }

    SDL_Rect destRect = {xDst, yDst, wDst, hDst};
    SDL_Rect sourceRect;
    if(tx.w_orig == 0 && tx.h_orig == 0)
        sourceRect = {0, 0, tx.w, tx.h};
    else
        sourceRect = {0, 0, tx.w_orig, tx.h_orig};

    SDL_SetTextureColorMod(tx.texture,
                           static_cast<unsigned char>(255.f * red),
                           static_cast<unsigned char>(255.f * green),
                           static_cast<unsigned char>(255.f * blue));
    SDL_SetTextureAlphaMod(tx.texture, static_cast<unsigned char>(255.f * alpha));
    SDL_RenderCopyEx(m_gRenderer, tx.texture, &sourceRect, &destRect,
                     0.0, nullptr, static_cast<SDL_RendererFlip>(flip));
}

void FrmMain::getScreenPixels(int x, int y, int w, int h, unsigned char *pixels)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderReadPixels(m_gRenderer,
                         &rect,
                         SDL_PIXELFORMAT_BGR24,
                         pixels,
                         w * 3 + (w % 4));
}

void FrmMain::getScreenPixelsRGBA(int x, int y, int w, int h, unsigned char *pixels)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderReadPixels(m_gRenderer,
                         &rect,
                         SDL_PIXELFORMAT_ABGR8888,
                         pixels,
                         w * 4);
}

int FrmMain::getPixelDataSize(const StdPicture &tx)
{
    if(!tx.texture)
        return 0;
    return (tx.w * tx.h * 4);
}

void FrmMain::getPixelData(const StdPicture &tx, unsigned char *pixelData)
{
    if(!tx.texture)
        return;
    int pitch, w, h, a;
    void *pixels;
    SDL_SetTextureBlendMode(tx.texture, SDL_BLENDMODE_BLEND);
    SDL_QueryTexture(tx.texture, nullptr, &a, &w, &h);
    SDL_LockTexture(tx.texture, nullptr, &pixels, &pitch);
    std::memcpy(pixelData, pixels, static_cast<size_t>(pitch * h));
    SDL_UnlockTexture(tx.texture);
}
