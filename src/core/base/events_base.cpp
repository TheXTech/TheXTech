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

#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>

#include "events_base.h"
#include "window_base.h"
#include "../render.h"
#include "../window.h"
#include "controls.h"
#include "main/cheat_code.h"
#include "globals.h"
#include "graphics.h"
#include "editor.h"
#include "frame_timer.h"


AbstractEvents_t *g_events = nullptr;

FrmMain *AbstractEvents_t::m_form = nullptr;


void AbstractEvents_t::init(FrmMain *form)
{
    m_form = form;
}

void AbstractEvents_t::eventResize()
{
    XRender::updateViewport();
    SetupScreens();
    Controls::UpdateTouchScreenSize();
}
