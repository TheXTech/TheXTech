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

// Workaround for older SDL versions that lacks the floating-point based rects and points
#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(2, 0, 10)
#define XTECH_SDL_NO_RECTF_SUPPORT
typedef struct SDL_FPoint
{
    float x;
    float y;
} SDL_FPoint;
#endif


class FrmMain
{
    std::string m_windowTitle;
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_gRenderer = nullptr;
    SDL_Texture  *m_tBuffer = nullptr;
    SDL_Texture  *m_recentTarget = nullptr;
    std::set<SDL_Texture *> m_textureBank;
    const Uint8 *m_keyboardState = nullptr;
    Uint32 m_lastMousePress = 0;
    SDL_Event m_event = {};
    SDL_RendererInfo m_ri = {};
#ifdef __ANDROID__
    bool m_blockRender = false;
#endif

    size_t m_lazyLoadedBytes = 0;

public:
    int MousePointer = 0;

    FrmMain() noexcept;

    SDL_Window *getWindow();

    Uint8 getKeyState(SDL_Scancode key);

    bool initSDL(const CmdLineSetup_t &setup);
    void freeSDL();

    void show();
    void hide();
    void doEvents();
    void waitEvents();

    bool isWindowActive();
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

    int setFullScreen(bool fs);
    void setWindowSize(int w, int h);
    void getWindowSize(int *w, int *h);


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

    bool isSdlError();

private:
    void processEvent();
};

//! Main window
extern FrmMain frmMain;

#endif // FRMMAIN_H
