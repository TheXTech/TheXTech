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

#pragma once
#ifndef EVENTS_HHHHHHH
#define EVENTS_HHHHHHH

#ifndef EVENTS_CUSTOM
#   include "core/sdl.h"
#   include "base/events_base.h"
#   define E_INLINE SDL_FORCE_INLINE
#   define TAIL
#else
#   include <stdint.h>
#   define E_INLINE    extern
#   define TAIL ;
#endif


/*!
 *  Events interface
 */
namespace XEvents
{

#ifdef EVENTS_CUSTOM

extern bool init();
extern void quit();

#endif

/*!
 * \brief Process events
 */
E_INLINE void doEvents() TAIL

#ifndef EVENTS_CUSTOM
{
    g_events->doEvents();
}
#endif

/*!
 * \brief Wait until any events will happen
 */
E_INLINE void waitEvents() TAIL

#ifndef EVENTS_CUSTOM
{
    g_events->waitEvents();
}
#endif

E_INLINE void eventResize() TAIL

#ifndef EVENTS_CUSTOM
{
    g_events->eventResize();
}
#endif

} // XEvents

#ifndef EVENTS_CUSTOM
#   undef E_INLINE
#   undef TAIL
#endif

#endif // EVENTS_HHHHHHH
