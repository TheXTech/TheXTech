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

#pragma once
#ifndef EVENTSSDL_H
#define EVENTSSDL_H

#include <SDL2/SDL_events.h>
#include "../base/events_base.h"


class EventsSDL final : public AbstractEvents_t
{
    SDL_Event m_event = {};

public:
    EventsSDL();
    virtual ~EventsSDL() override;

    /*!
     * \brief Initialize the events processor
     * \param form Pointer to the main form
     */
    void init(FrmMain *form) override;

    /*!
     * \brief Process events
     */
    void doEvents() override;

    /*!
     * \brief Wait until any events will happen
     */
    void waitEvents() override;

private:
    void processEvent();
};


#endif // EVENTSSDL_H
