/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "core/window.h"
#ifdef USE_RENDER_BLOCKING
#include "core/render.h"
#endif
#include "config.h"
#include "frm_main.h"
#include "game_main.h"
#include "sound.h"
#include "controls.h"


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
}

void EventsSDL::doEvents()
{
#ifdef __WIIU__
    if(m_gotExit)
        return;
#endif

    while(SDL_PollEvent(&m_event))
    {
        processEvent();
#ifdef __WIIU__
        if(m_event.type == SDL_QUIT) // Don't process any events after quit
        {
            m_gotExit = true;
            break;
        }
#endif
    }
}

void EventsSDL::waitEvents()
{
#ifdef __WIIU__
    if(m_gotExit)
        return;
#endif

    if(SDL_WaitEventTimeout(&m_event, 1000))
        processEvent();
    doEvents();
}

void EventsSDL::processEvent()
{
#ifdef __WIIU__
    if(m_gotExit)
        return;
#endif

    if(Controls::ProcessEvent(&m_event))
        return;

    switch(m_event.type)
    {
    case SDL_QUIT:
        XWindow::showCursor(1);
        KillIt();
        break;

#ifdef DEBUG_BUILD
    case SDL_FINGERDOWN:
        pLogDebug("Touch finger down at device %d", m_event.tfinger.touchId);
        break;

    case SDL_FINGERUP:
        pLogDebug("Touch finger up at device %d", m_event.tfinger.touchId);
        break;
#endif

    case SDL_WINDOWEVENT:
        switch(m_event.window.event)
        {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_MOVED:
            eventResize();
            break;
#if !defined(NO_WINDOW_FOCUS_TRACKING)
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            if(!g_config.background_work && !LoadingInProcess)
                SoundPauseEngine(0);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            if(!g_config.background_work && !LoadingInProcess)
                SoundPauseEngine(1);
            break;
#endif
        default:
            break;
        }
        break;
#ifdef USE_RENDER_BLOCKING
    case SDL_RENDER_DEVICE_RESET:
        pLogInfo("Android: Render Device Reset");
        g_frmMain.restartRenderer();
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        XRender::setBlockRender(true);
        D_pLogDebugNA("Android: Entering background");
        break;
    case SDL_APP_DIDENTERFOREGROUND:
        XRender::setBlockRender(false);
        D_pLogDebugNA("Android: Resumed foreground");
        break;
#endif
    }
}
