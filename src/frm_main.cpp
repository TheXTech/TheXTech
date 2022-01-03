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

#include <memory>
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

#include "core/window.h"
#include "core/window_sdl.h"

#include "video.h"
#include "frm_main.h"
#include "main/game_info.h"


FrmMain frmMain;


FrmMain::FrmMain() noexcept
{
    SDL_memset(&m_event, 0, sizeof(SDL_Event));
}

Uint8 FrmMain::getKeyState(SDL_Scancode key)
{
    if(m_keyboardState)
        return m_keyboardState[key];
    return 0;
}

bool FrmMain::initSystem(const CmdLineSetup_t &setup)
{
    std::unique_ptr<RenderSDL> render;
    std::unique_ptr<WindowSDL> window;
    bool res = false;

    LoadLogSettings(setup.interprocess, setup.verboseLogging);
    //Write into log the application start event
    pLogDebug("<Application started>");

    //Initialize FreeImage
    GraphicsHelps::initFreeImage();

    window.reset(new WindowSDL());
    render.reset(new RenderSDL());

    render->init();
    res = window->initSDL(setup, render->SDL_InitFlags());

    if(!res)
        return true;

    pLogDebug("Init renderer settings...");

    if(!render->initRender(setup, window->getWindow()))
    {
        freeSystem();
        return true;
    }

    m_keyboardState = SDL_GetKeyboardState(nullptr);
    doEvents();

    g_render = render.get();
    m_render.reset(render.get());
    render.release();

    g_window = window.get();
    m_win.reset(window.get());
    window.release();

    return !res;
}

void FrmMain::freeSystem()
{
    GFX.unLoad();
    if(m_render)
        m_render->clearAllTextures();

    joyCloseJoysticks();

    m_render->close();
    m_render.reset();
    g_render = nullptr;

    m_win->close();
    m_win.reset();
    g_window = nullptr;
    GraphicsHelps::closeFreeImage();

    pLogDebug("<Application closed>");
    CloseLog();
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

void FrmMain::eventDoubleClick()
{
    if(MagicHand)
        return; // Don't toggle fullscreen/window when magic hand is active

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    if(resChanged)
    {
        g_window->setFullScreen(false);
        resChanged = false;
        g_window->restoreWindow();
        g_window->setWindowSize(ScreenW, ScreenH);
        if(!GameMenu && !MagicHand)
            g_window->showCursor(1);
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
                                    nullptr);
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
    mbox.window         = nullptr;
    mbox.title          = ttl.c_str();
    mbox.message        = msg.c_str();
    mbox.numbuttons     = 1;
    mbox.buttons        = &mboxButton;
    mbox.colorScheme    = &colorScheme;
    SDL_ShowMessageBox(&mbox, nullptr);
}
