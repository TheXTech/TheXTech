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

#include <SDL2/SDL_stdinc.h>

#include <Logger/logger.h>

#include "events_sdl.h"
#include "core/render.h"
#include "core/window.h"
#include "../frm_main.h"
#include "../game_main.h"
#include "../sound.h"
#include "../control/joystick.h"


EventsSDL::EventsSDL() :
    AbstractEvents_t()
{
    SDL_memset(&m_event, 0, sizeof(SDL_Event));
}

EventsSDL::~EventsSDL()
{}

void EventsSDL::init(FrmMain *form)
{
    AbstractEvents_t::init(form);
    m_keyboardState = SDL_GetKeyboardState(nullptr);
    doEvents();
}

void EventsSDL::doEvents()
{
    while(SDL_PollEvent(&m_event))
    {
        processEvent();
    }
}

void EventsSDL::waitEvents()
{
    if(SDL_WaitEventTimeout(&m_event, 1000))
        processEvent();
    doEvents();
}

uint8_t EventsSDL::getKeyState(int scan_code)
{
    if(m_keyboardState)
        return m_keyboardState[scan_code];
    return 0;
}

const char *EventsSDL::getScanCodeName(int scan_code)
{
    SDL_Scancode k = static_cast<SDL_Scancode>(scan_code);
    return SDL_GetScancodeName(k);
}

static int keyModConvert(Uint16 mod)
{
    int ret = KEYMOD_NONE;

    if(KMOD_LCTRL & mod)
        ret |= KEYMOD_LCTRL;
    if(KMOD_RCTRL & mod)
        ret |= KEYMOD_RCTRL;
    if(KMOD_LALT & mod)
        ret |= KEYMOD_LALT;
    if(KMOD_RALT & mod)
        ret |= KEYMOD_RALT;
    if(KMOD_LSHIFT & mod)
        ret |= KEYMOD_LSHIFT;
    if(KMOD_RSHIFT & mod)
        ret |= KEYMOD_RSHIFT;

    return ret;
}

void EventsSDL::processEvent()
{
    switch(m_event.type)
    {
    case SDL_QUIT:
        g_window->showCursor(1);
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
    {
        KeyboardEvent_t e;
        e.scancode = m_event.key.keysym.scancode;
        e.mod = keyModConvert(m_event.key.keysym.mod);

        eventKeyDown(e);
        eventKeyPress(e.scancode);
        break;
    }
    case SDL_KEYUP:
    {
        KeyboardEvent_t e;
        e.scancode = m_event.key.keysym.scancode;
        e.mod = keyModConvert(m_event.key.keysym.mod);

        eventKeyUp(e);
        break;
    }
    case SDL_MOUSEBUTTONDOWN:
    {
        MouseButtonEvent_t e;
        switch(m_event.button.button)
        {
        case SDL_BUTTON_LEFT:
            e.button = MOUSE_BUTTON_LEFT;
            break;
        case SDL_BUTTON_MIDDLE:
            e.button = MOUSE_BUTTON_MIDDLE;
            break;
        case SDL_BUTTON_RIGHT:
            e.button = MOUSE_BUTTON_RIGHT;
            break;
        }
        eventMouseDown(e);
        break;
    }
    case SDL_MOUSEBUTTONUP:
    {
        MouseButtonEvent_t e;
        switch(m_event.button.button)
        {
        case SDL_BUTTON_LEFT:
            e.button = MOUSE_BUTTON_LEFT;
            break;
        case SDL_BUTTON_MIDDLE:
            e.button = MOUSE_BUTTON_MIDDLE;
            break;
        case SDL_BUTTON_RIGHT:
            e.button = MOUSE_BUTTON_RIGHT;
            break;
        }
        eventMouseUp(e);
        break;
    }
    case SDL_MOUSEMOTION:
    {
        MouseMoveEvent_t e;
        e.x = m_event.motion.x;
        e.y = m_event.motion.y;
        eventMouseMove(e);
        break;
    }
    case SDL_MOUSEWHEEL:
    {
        MouseWheelEvent_t e;
        e.x = m_event.wheel.x;
        e.y = m_event.wheel.y;
        eventMouseWheel(e);
        break;
    }
#ifdef __ANDROID__
    case SDL_RENDER_DEVICE_RESET:
        D_pLogDebug("Android: Render Device Reset");
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        g_render->setBlockRender(true);
        D_pLogDebug("Android: Entering background");
        break;
    case SDL_APP_DIDENTERFOREGROUND:
        g_render->setBlockRender(false);
        D_pLogDebug("Android: Resumed foreground");
        break;
#endif
    }
}
