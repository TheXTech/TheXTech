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
#ifndef EVENTS_HHHHHHH
#define EVENTS_HHHHHHH

#include <stdint.h>

#include <SDL2/SDL_stdinc.h>
#include "base/events_base.h"

#ifndef EVENTS_CUSTOM
#   define E_INLINE SDL_FORCE_INLINE
#   define TAIL
#else
#   define E_INLINE    extern
#   define TAIL ;
#endif


/*!
 *  Events interface
 */
namespace XEvents
{

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

/*!
 * \brief Get key state by scancode
 * \param scan_code Scancode of the backend
 * \return Key state value
 */
E_INLINE bool getKeyState(int scan_code) TAIL

#ifndef EVENTS_CUSTOM
{
    if(scan_code < 0)
        return false;
    return g_events->getKeyState(scan_code);
}
#endif

/*!
 * \brief Get the name of the key by scancode
 * \param scan_code Scancode of the backend
 * \return Human-readable name of the key
 */
E_INLINE const char *getScanCodeName(int scan_code) TAIL

#ifndef EVENTS_CUSTOM
{
    return g_events->getScanCodeName(scan_code);
}
#endif


SDL_FORCE_INLINE void eventResize()
{
    g_events->eventResize();
}

} // XEvents

#ifndef EVENTS_CUSTOM
#   undef E_INLINE
#   undef TAIL
#endif

#endif // EVENTS_HHHHHHH
