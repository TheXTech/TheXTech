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

#include <SDL2/SDL_stdinc.h>

#include <Logger/logger.h>

#include "events_sdl.h"
#include "core/window.h"
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

void EventsSDL::processEvent()
{
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
            if(!neverPause && !LoadingInProcess)
                SoundPauseEngine(0);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            if(!neverPause && !LoadingInProcess)
                SoundPauseEngine(1);
            break;
#endif
        default:
            break;
        }
        break;
#ifdef USE_RENDER_BLOCKING
    case SDL_RENDER_DEVICE_RESET:
        D_pLogDebug("Android: Render Device Reset");
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        XRender::setBlockRender(true);
        D_pLogDebug("Android: Entering background");
        break;
    case SDL_APP_DIDENTERFOREGROUND:
        XRender::setBlockRender(false);
        D_pLogDebug("Android: Resumed foreground");
        break;
#endif
    }
}
