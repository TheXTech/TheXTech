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

#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>

#include "events_base.h"
#include "window_base.h"
#include "../render.h"
#include "../window.h"
#include "control/joystick.h"
#include "main/cheat_code.h"
#include "globals.h"
#include "graphics.h"
#include "editor.h"
#include "frame_timer.h"


AbstractEvents_t *g_events = nullptr;

uint32_t AbstractEvents_t::m_lastMousePress = 0;
FrmMain *AbstractEvents_t::m_form = nullptr;


void AbstractEvents_t::init(FrmMain *form)
{
    m_form = form;
}

void AbstractEvents_t::eventDoubleClick()
{
    if(MagicHand)
        return; // Don't toggle fullscreen/window when magic hand is active

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    if(resChanged)
    {
        XWindow::setFullScreen(false);
        resChanged = false;
        XWindow::restoreWindow();
        XWindow::setWindowSize(ScreenW, ScreenH);
        if(!GameMenu && !MagicHand)
            XWindow::showCursor(1);
    }
    else
        SetRes();
#endif
}

void AbstractEvents_t::eventKeyPress(int scan_code)
{
    switch(scan_code)
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
    case SDL_SCANCODE_SEMICOLON: CheatCode(';'); break; // for AZERTY support
    default: CheatCode(' '); break;
    }
}

void AbstractEvents_t::eventKeyDown(const KeyboardEvent_t &evt)
{
    int KeyCode = evt.scancode;
    inputKey = KeyCode;

    bool ctrlF = ((evt.mod & KEYMOD_CTRL) != 0 && evt.scancode == SDL_SCANCODE_F);
    bool altEnter = ((evt.mod & KEYMOD_ALT) != 0 && (evt.scancode == SDL_SCANCODE_RETURN || evt.scancode == SDL_SCANCODE_KP_ENTER));

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
        XRender::toggleGifRecorder();
#endif // USE_SCREENSHOTS_AND_RECS
}

void AbstractEvents_t::eventKeyUp(const KeyboardEvent_t &evt)
{
    UNUSED(evt);
}

void AbstractEvents_t::eventMouseDown(const MouseButtonEvent_t &event)
{
    switch(event.button)
    {
    case MOUSE_BUTTON_LEFT:
    {
        MenuMouseDown = true;
        MenuMouseMove = true;
        if(LevelEditor || MagicHand || TestLevel)
            EditorControls.Mouse1 = true;
        break;
    }

    case MOUSE_BUTTON_RIGHT:
    {
        MenuMouseBack = true;
        if(LevelEditor || MagicHand || TestLevel)
        {
            optCursor.current = OptCursor_t::LVL_SELECT;
            MouseMove(float(MenuMouseX), float(MenuMouseY));
            SetCursor();
        }
        break;
    }

    case MOUSE_BUTTON_MIDDLE:
    {
        if(LevelEditor || MagicHand || TestLevel)
        {
            optCursor.current = OptCursor_t::LVL_ERASER;
            MouseMove(float(MenuMouseX), float(MenuMouseY));
            SetCursor();
        }
        break;
    }
    }
}

void AbstractEvents_t::eventMouseMove(const MouseMoveEvent_t &event)
{
    int px, py;
    XRender::mapToScreen(event.x, event.y, &px, &py);
    MenuMouseX = px; //int(event.x * ScreenW / ScaleWidth);
    MenuMouseY = py; //int(event.y * ScreenH / ScaleHeight);
    MenuMouseMove = true;

    if(LevelEditor || MagicHand || TestLevel)
    {
        EditorCursor.X = CursorPos.X;
        EditorCursor.Y = CursorPos.Y;
        MouseMove(EditorCursor.X, EditorCursor.Y, true);
        MouseRelease = true;
    }
}

void AbstractEvents_t::eventMouseWheel(const MouseWheelEvent_t &event)
{
    MenuWheelDelta = event.y;
    MenuWheelMoved = true;
}

void AbstractEvents_t::eventMouseUp(const MouseButtonEvent_t &event)
{
    bool doubleClick = false;
    MenuMouseDown = false;
    MenuMouseRelease = true;
    if(LevelEditor || MagicHand || TestLevel)
        EditorControls.Mouse1 = false;

    if(event.button == MOUSE_BUTTON_LEFT)
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

void AbstractEvents_t::eventResize()
{
    XRender::updateViewport();
    SetupScreens();
#ifdef USE_TOUCHSCREEN_CONTROLLER
    UpdateTouchScreenSize();
#endif
}
