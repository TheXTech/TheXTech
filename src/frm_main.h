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

#pragma once
#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <SDL2/SDL_version.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>

#include <memory>
#include <string>
#include <set>

#ifndef __EMSCRIPTEN__
#include <deque>
#endif

#include <gif_writer.h>

#include "std_picture.h"
#include "cmd_line_setup.h"

typedef struct SDL_Thread SDL_Thread;
typedef struct SDL_mutex SDL_mutex;

class AbstractWindow_t;
class AbstractRender_t;

class FrmMain
{
    const Uint8 *m_keyboardState = nullptr;
    Uint32 m_lastMousePress = 0;
    SDL_Event m_event = {};

    std::unique_ptr<AbstractWindow_t> m_win;
    std::unique_ptr<AbstractRender_t> m_render;

public:
    int MousePointer = 0;

    FrmMain() noexcept;

    Uint8 getKeyState(SDL_Scancode key);

    bool initSystem(const CmdLineSetup_t &setup);
    void freeSystem();

    void show();
    void hide();
    void doEvents();
    void waitEvents();

    bool hasWindowInputFocus();
    bool hasWindowMouseFocus();

    void eventDoubleClick();
    void eventKeyPress(SDL_Scancode KeyASCII);
    void eventKeyDown(SDL_KeyboardEvent &evt);
    void eventKeyUp(SDL_KeyboardEvent &evt);
    void eventMouseDown(SDL_MouseButtonEvent &m_event);
    void eventMouseMove(SDL_MouseMotionEvent &m_event);
    void eventMouseWheel(SDL_MouseWheelEvent &m_event);
    void eventMouseUp(SDL_MouseButtonEvent &m_event);
    void eventResize();


    enum MessageBoxFlags
    {
        MESSAGEBOX_ERROR                 = 0x00000010,   /**< error dialog */
        MESSAGEBOX_WARNING               = 0x00000020,   /**< warning dialog */
        MESSAGEBOX_INFORMATION           = 0x00000040,   /**< informational dialog */
        MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT = 0x00000080,   /**< buttons placed left to right */
        MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT = 0x00000100    /**< buttons placed right to left */
    };
    int simpleMsgBox(uint32_t flags, const std::string &title, const std::string &message);

    void errorMsgBox(const std::string &title, const std::string &message);

private:
    void processEvent();
};

//! Main window
extern FrmMain frmMain;

#endif // FRMMAIN_H
