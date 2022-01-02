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

#include "globals.h"
#include "frame_timer.h"
#include "game_main.h"
#include "graphics.h"
#include "control/joystick.h"
#include "sound.h"
#include "gfx.h"
#include "editor.h"

#include <Logger/logger.h>
#include <Graphics/graphics_funcs.h>
#include <fmt_time_ne.h>
#include <fmt_format_ne.h>


#include "../version.h"

#include "core/render.h"
#include "core/render_sdl.h"

#include "video.h"
#include "frm_main.h"
#include "main/game_info.h"


FrmMain frmMain;


static SDL_bool IsFullScreen(SDL_Window *win)
{
    Uint32 flags = SDL_GetWindowFlags(win);
    return (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? SDL_TRUE : SDL_FALSE;
}

FrmMain::FrmMain() noexcept
{
    SDL_memset(&m_event, 0, sizeof(SDL_Event));
    SDL_memset(&m_ri, 0, sizeof(SDL_RendererInfo));
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
    RenderSDL_t *render;
    bool res = false;

    m_windowTitle = g_gameInfo.titleWindow;

    LoadLogSettings(setup.interprocess, setup.verboseLogging);
    //Write into log the application start event
    pLogDebug("<Application started>");

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

    // Workaround: https://discourse.libsdl.org/t/26995
    setlocale(LC_NUMERIC, "C");

    const char *error = SDL_GetError();
    if(*error != '\0')
        pLogWarning("Error while SDL Initialization: %s", error);
    SDL_ClearError();

    SDL_GL_ResetAttributes();

    render = new RenderSDL_t();
    render->init();

    m_window = SDL_CreateWindow(m_windowTitle.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              ScreenW, ScreenH,
                              SDL_WINDOW_RESIZABLE |
                              SDL_WINDOW_HIDDEN |
                              SDL_WINDOW_ALLOW_HIGHDPI |
                              render->SDL_InitFlags());

    if(m_window == nullptr)
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        delete render;
        return false;
    }

    if(isSdlError())
    {
        pLogCritical("Unable to create window!");
        SDL_ClearError();
        delete render;
        return false;
    }

#ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
    SDL_SetWindowMinimumSize(m_window, ScaleWidth / 2, ScaleHeight / 2);
#elif defined(__ANDROID__) // Set as small as possible
    SDL_SetWindowMinimumSize(m_window, 200, 150);
#else
    SDL_SetWindowMinimumSize(m_window, ScreenW, ScreenH);
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

    if(!render->initRender(setup, m_window))
    {
        delete render;
        freeSDL();
        return false;
    }

    m_keyboardState = SDL_GetKeyboardState(nullptr);
    doEvents();

    g_render = render;

    return res;
}

void FrmMain::freeSDL()
{
    GFX.unLoad();
    if(g_render)
        g_render->clearAllTextures();
    joyCloseJoysticks();

    if(g_render)
    {
        g_render->close();
        delete g_render;
        g_render = nullptr;
    }

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
            if(!neverPause && !LoadingInProcess)
                SoundPauseEngine(0);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            if(!neverPause && !LoadingInProcess)
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
    case SDL_MOUSEWHEEL:
        eventMouseWheel(m_event.wheel);
        break;
#ifdef __ANDROID__
    case SDL_RENDER_DEVICE_RESET:
        D_pLogDebug("Android: Render Device Reset");
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        g_render->m_blockRender = true;
        D_pLogDebug("Android: Entering background");
        break;
    case SDL_APP_DIDENTERFOREGROUND:
        g_render->m_blockRender = false;
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

#ifdef USE_SCREENSHOTS_AND_RECS
    if(KeyCode == SDL_SCANCODE_F12)
        TakeScreen = true;
    else if(KeyCode == SDL_SCANCODE_F3)
        g_stats.enabled = !g_stats.enabled;
    else if(KeyCode == SDL_SCANCODE_F4)
        ShowOnScreenMeta = !ShowOnScreenMeta;
#   ifdef __APPLE__
    else if(KeyCode == SDL_SCANCODE_F10) // Reserved by macOS as "show desktop"
#   else
    else if(KeyCode == SDL_SCANCODE_F11)
#   endif
        g_render->toggleGifRecorder();
#endif // USE_SCREENSHOTS_AND_RECS
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
    SDL_Point p;
    g_render->mapToScreen(event.x, event.y, &p.x, &p.y);
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

void FrmMain::eventMouseWheel(SDL_MouseWheelEvent &event)
{
    MenuWheelDelta = event.y;
    MenuWheelMoved = true;
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
    g_render->updateViewport();
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

void FrmMain::setWindowSize(int w, int h)
{
    SDL_SetWindowSize(m_window, w, h);
}

void FrmMain::getWindowSize(int *w, int *h)
{
    SDL_GetWindowSize(m_window, w, h);
}

int FrmMain::simpleMsgBox(uint32_t flags, const std::string &title, const std::string &message)
{
    Uint32 dFlags = 0;

    if(flags & MESSAGEBOX_ERROR)
        dFlags |= SDL_MESSAGEBOX_ERROR;

    if(flags & MESSAGEBOX_WARNING)
        dFlags |= SDL_MESSAGEBOX_WARNING;

    if(flags & MESSAGEBOX_INFORMATION)
        dFlags |= SDL_MESSAGEBOX_INFORMATION;

    if(flags & MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT)
        dFlags |= SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;

    if(flags & MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT)
        dFlags |= SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT;

    return SDL_ShowSimpleMessageBox(dFlags,
                                    title.c_str(),
                                    message.c_str(),
                                    m_window);
}

void FrmMain::errorMsgBox(const std::string &title, const std::string &message)
{
    const std::string &ttl = title;
    const std::string &msg = message;
    SDL_MessageBoxData mbox;
    SDL_MessageBoxButtonData mboxButton;
    const SDL_MessageBoxColorScheme colorScheme =
    {
        { /* .colors (.r, .g, .b) */
            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
            { 200, 200, 200 },
            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
            {   0,   0,   0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
            { 255, 255, 255 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
            { 150, 150, 150 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
            { 255, 255, 255 }
        }
    };
    mboxButton.buttonid = 0;
    mboxButton.flags    = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    mboxButton.text     = "Ok";
    mbox.flags          = SDL_MESSAGEBOX_ERROR;
    mbox.window         = m_window;
    mbox.title          = ttl.c_str();
    mbox.message        = msg.c_str();
    mbox.numbuttons     = 1;
    mbox.buttons        = &mboxButton;
    mbox.colorScheme    = &colorScheme;
    SDL_ShowMessageBox(&mbox, nullptr);
}

bool FrmMain::isSdlError()
{
    const char *error = SDL_GetError();
    return (*error != '\0');
}
