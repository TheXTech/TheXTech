/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <Logger/logger.h>

#if defined(__WII__) || !defined(RENDER_CUSTOM)
#   include <Graphics/graphics_funcs.h>
#endif

#include "gfx.h"

#include "core/render.h"
#include "core/window.h"
#include "core/msgbox.h"
#include "core/events.h"

#ifdef CORE_EVERYTHING_SDL
#   include "core/sdl/render_sdl.h"
#   include "core/sdl/render_gl11.h"
#   include "core/sdl/render_gles.h"
typedef RenderGLES RenderUsed;
#   define USE_CORE_RENDER_SDL

#   include "core/sdl/window_sdl.h"
typedef WindowSDL WindowUsed;
#   define USE_CORE_WINDOW_SDL

#   include "core/sdl/msgbox_sdl.h"
typedef MsgBoxSDL MsgBoxUsed;
#   define USE_CORE_MSGBOX_SDL

#   include "core/sdl/events_sdl.h"
typedef EventsSDL EventsUsed;
#   define USE_CORE_EVENTS_SDL
#endif

#include "fontman/font_manager.h"

#include "frm_main.h"


bool FrmMain::initSystem(const CmdLineSetup_t &setup)
{
    bool res = false;

    LoadLogSettings(setup.interprocess, setup.verboseLogging);
    //Write into log the application start event
    pLogDebug("<Application started>");

#if defined(__WII__) || !defined(RENDER_CUSTOM)
    //Initialize FreeImage
    D_pLogDebugNA("FrmMain: Loading FreeImage...");
    GraphicsHelps::initFreeImage();
#endif

    // Build interfaces
#ifndef WINDOW_CUSTOM
    WindowUsed *window = new WindowUsed();
    m_window.reset(window);
    g_window = m_window.get();
#endif

#ifndef RENDER_CUSTOM
    RenderUsed *render = new RenderUsed();
    m_render.reset(render);
    g_render = m_render.get();
#endif

#ifndef MSGBOX_CUSTOM
    MsgBoxUsed *msgbox = new MsgBoxUsed();
    m_msgbox.reset(msgbox);
    g_msgBox = m_msgbox.get();
#endif

#ifndef EVENTS_CUSTOM
    EventsUsed *events = new EventsUsed();
    m_events.reset(events);
    g_events = m_events.get();
#endif

    // Initializing window

#ifdef WINDOW_CUSTOM
    D_pLogDebugNA("FrmMain: Loading XWindow...");
    res = XWindow::init();
#elif defined(USE_CORE_WINDOW_SDL)
    res = window->initSDL(setup, render->SDL_InitFlags());
#else
#   error "FIXME: Implement supported window initialization here"
#endif

    if(!res)
        return true;


    // Initializing message box

#ifdef MSGBOX_CUSTOM
    D_pLogDebugNA("FrmMain: Loading XMsgBox...");
    res &= XMsgBox::init();
#elif defined(USE_CORE_WINDOW_SDL) && defined(USE_CORE_MSGBOX_SDL)
    msgbox->init(window->getWindow());
#else
#   error "FIXME: Implement supported message boxes initialization here"
#endif



    // Initializing events

#ifdef EVENTS_CUSTOM
    D_pLogDebugNA("FrmMain: Loading XEvents...");
    res &= XEvents::init();
#else
    events->init(this);
#endif


    // Initializing render
    pLogDebug("Init renderer settings...");

#ifdef RENDER_CUSTOM
    D_pLogDebugNA("FrmMain: Loading XRender...");
    res &= XRender::init();
#elif defined(USE_CORE_WINDOW_SDL) && defined(USE_CORE_RENDER_SDL)
    res = render->initRender(setup, window->getWindow());
#else
#   error "FIXME: Implement supported render initialization here"
#endif

    if(!res)
    {
        D_pLogCriticalNA("FrmMain: Error has occured, exiting...");
        freeSystem();
        return true;
    }

    FontManager::initFull();

    return !res;
}

void FrmMain::freeSystem()
{
    FontManager::quit();

    GFX.unLoad();

#ifdef RENDER_CUSTOM
    XRender::quit();
#else
    if(m_render)
    {
        m_render->clearAllTextures();
        m_render->close();
    }

    m_render.reset();
    g_render = nullptr;
#endif

#ifdef MSGBOX_CUSTOM
    XMsgBox::quit();
#else
    if(m_msgbox)
        m_msgbox->close();

    m_msgbox.reset();
    g_msgBox = nullptr;
#endif

#ifdef EVENTS_CUSTOM
    XEvents::quit();
#else
    m_events.reset();
    g_events = nullptr;
#endif

#ifdef WINDOW_CUSTOM
    XWindow::quit();
#else
    if(m_window)
        m_window->close();

    m_window.reset();
    g_window = nullptr;
#endif

#if defined(__WII__) || !defined(RENDER_CUSTOM)
    GraphicsHelps::closeFreeImage();
#endif

    pLogDebug("<Application closed>");
    CloseLog();
}
