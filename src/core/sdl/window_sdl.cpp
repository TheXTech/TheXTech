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

#include <locale.h>
#include <SDL2/SDL.h>

#include <Logger/logger.h>
#include <Graphics/graphics_funcs.h>

#include "main/game_info.h"
#include "window_sdl.h"
#include "../render.h"

//! Path to game resources assets (by default it's ~/.PGE_Project/thextech/)
extern std::string AppPath;


WindowSDL::WindowSDL() :
    AbstractWindow_t()
{}

WindowSDL::~WindowSDL()
{}

bool WindowSDL::initSDL(const CmdLineSetup_t &setup, uint32_t windowInitFlags)
{
    bool res = true;

    m_windowTitle = g_gameInfo.titleWindow;

    if(setup.allowBgInput)
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

#if defined(__ANDROID__) || (defined(__APPLE__) && (defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_IPHONE)))
    // Restrict the landscape orientation only
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

#if defined(__ANDROID__)
    SDL_setenv("SDL_AUDIODRIVER", "openslES", 1);
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
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
    res = (SDL_Init(sdlInitFlags) >= 0);

    // Workaround: https://discourse.libsdl.org/t/26995
    setlocale(LC_NUMERIC, "C");

    const char *error = SDL_GetError();
    if(*error != '\0')
        pLogWarning("Error while SDL Initialization: %s", error);
    SDL_ClearError();
    if(!res)
        return false;

    SDL_GL_ResetAttributes();

#if defined(__SWITCH__) /* On Switch, expect the initial size 1280x720 */
    const int initWindowW = 1920;
    const int initWindowH = 1080;

#else
    const auto initWindowW = ScreenW;
    const auto initWindowH = ScreenH;
#endif

#if defined(RENDER_FULLSCREEN_ALWAYS)
    windowInitFlags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_SHOWN;
#endif

    m_window = SDL_CreateWindow(m_windowTitle.c_str(),
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                initWindowW, initWindowH,
                                SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_HIDDEN |
                                SDL_WINDOW_ALLOW_HIGHDPI |
                                windowInitFlags);

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
    SDL_SetWindowMinimumSize(m_window, ScreenW / 2, ScreenH / 2);
#elif defined(__ANDROID__) || defined(__SWITCH__) // Set as small as possible
    SDL_SetWindowMinimumSize(m_window, 200, 150);
#elif defined(VITA)
    SDL_SetWindowMinimumSize(m_window, 960, 544);
#else
    SDL_SetWindowMinimumSize(m_window, ScreenW, ScreenH);
#endif //__EMSCRIPTEN__

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

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

    return res;
}

void WindowSDL::close()
{
    if(m_window)
        SDL_DestroyWindow(m_window);
    m_window = nullptr;

    SDL_Quit();
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

void WindowSDL::setCursor(Cursor_t cursor)
{
    m_cursor = cursor;
    // Do nothing, just remember the last cursor type was set
}

AbstractWindow_t::Cursor_t WindowSDL::getCursor()
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
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? SDL_TRUE : SDL_FALSE;
}

int WindowSDL::setFullScreen(bool fs)
{
    if(!m_window)
        return -1;

    if(fs != isFullScreen())
    {
        if(fs)
        {
            // Swith to FULLSCREEN mode
            if(SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
            {
                pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
                return -1;
            }

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
#ifdef __EMSCRIPTEN__
            SDL_SetWindowSize(m_window, ScreenW, ScreenH);
#endif
            return 0;
        }
    }

    return 0;
}

void WindowSDL::restoreWindow()
{
    SDL_RestoreWindow(m_window);
}

void WindowSDL::setWindowSize(int w, int h)
{
    SDL_SetWindowSize(m_window, w, h);
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
