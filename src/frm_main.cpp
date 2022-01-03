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

#include "control/joystick.h"
#include "gfx.h"

#include <Logger/logger.h>
#include <Graphics/graphics_funcs.h>

#include "core/render.h"
#include "core/render_sdl.h"

#include "core/window.h"
#include "core/window_sdl.h"

#include "core/msgbox.h"
#include "core/msgbox_sdl.h"

#include "core/events.h"
#include "core/events_sdl.h"

#include "frm_main.h"


FrmMain frmMain;


FrmMain::FrmMain() noexcept
{}

bool FrmMain::initSystem(const CmdLineSetup_t &setup)
{
    std::unique_ptr<RenderSDL> render;
    std::unique_ptr<WindowSDL> window;
    std::unique_ptr<MsgBoxSDL> msgbox;
    std::unique_ptr<EventsSDL> events;
    bool res = false;

    LoadLogSettings(setup.interprocess, setup.verboseLogging);
    //Write into log the application start event
    pLogDebug("<Application started>");

    //Initialize FreeImage
    GraphicsHelps::initFreeImage();

    window.reset(new WindowSDL());
    render.reset(new RenderSDL());
    msgbox.reset(new MsgBoxSDL());
    events.reset(new EventsSDL());

    render->init();
    res = window->initSDL(setup, render->SDL_InitFlags());

    if(!res)
        return true;

    msgbox->init(window->getWindow());
    events->init(this);

    pLogDebug("Init renderer settings...");

    if(!render->initRender(setup, window->getWindow()))
    {
        freeSystem();
        return true;
    }

    g_msgBox = msgbox.get();
    m_msgbox.reset(msgbox.get());
    msgbox.release();

    g_events = events.get();
    m_events.reset(events.get());
    events.release();

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

    m_msgbox->close();
    m_msgbox.reset();
    g_msgBox = nullptr;

    m_events.reset();
    g_events = nullptr;

    m_win->close();
    m_win.reset();
    g_window = nullptr;
    GraphicsHelps::closeFreeImage();

    pLogDebug("<Application closed>");
    CloseLog();
}
