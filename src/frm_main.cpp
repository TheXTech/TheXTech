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


#include <Logger/logger.h>
#include <Graphics/graphics_funcs.h>

#include "control/joystick.h"
#include "gfx.h"

#ifdef CORE_EVERYTHING_SDL
#   include "core/sdl/render_sdl.h"
typedef RenderSDL RenderUsed;
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

#include "frm_main.h"


bool FrmMain::initSystem(const CmdLineSetup_t &setup)
{
    bool res = false;

    LoadLogSettings(setup.interprocess, setup.verboseLogging);
    //Write into log the application start event
    pLogDebug("<Application started>");

    //Initialize FreeImage
    GraphicsHelps::initFreeImage();

    // Build interfaces
    WindowUsed *window = new WindowUsed();
    RenderUsed *render = new RenderUsed();
    MsgBoxUsed *msgbox = new MsgBoxUsed();
    EventsUsed *events = new EventsUsed();

    m_window.reset(window);
    m_render.reset(render);
    m_msgbox.reset(msgbox);
    m_events.reset(events);

    g_window = m_window.get();
    g_msgBox = m_msgbox.get();
    g_events = m_events.get();
    g_render = m_render.get();


    // Initializing window

#ifdef USE_CORE_WINDOW_SDL
    res = window->initSDL(setup, render->SDL_InitFlags());
#else
#error "FIXME: Implement supported window initialization here"
#endif

    if(!res)
        return true;


    // Initializing message box

#if defined(USE_CORE_WINDOW_SDL) && defined(USE_CORE_MSGBOX_SDL)
    msgbox->init(window->getWindow());
#else
#error "FIXME: Implement supported message boxes initialization here"
#endif



    // Initializing events
    events->init(this);


    // Initializing render
    pLogDebug("Init renderer settings...");

#if defined(USE_CORE_WINDOW_SDL) && defined(USE_CORE_RENDER_SDL)
    res = render->initRender(setup, window->getWindow());
#else
#error "FIXME: Implement supported render initialization here"
#endif

    if(!res)
    {
        freeSystem();
        return true;
    }

    return !res;
}

void FrmMain::freeSystem()
{
    GFX.unLoad();
    if(m_render)
        m_render->clearAllTextures();

    m_render->close();
    m_render.reset();
    g_render = nullptr;

    m_msgbox->close();
    m_msgbox.reset();
    g_msgBox = nullptr;

    m_events.reset();
    g_events = nullptr;

    m_window->close();
    m_window.reset();
    g_window = nullptr;
    GraphicsHelps::closeFreeImage();

    pLogDebug("<Application closed>");
    CloseLog();
}
