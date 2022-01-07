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
#ifndef ABSTRACTEVENTS_T_H
#define ABSTRACTEVENTS_T_H

#include <stdint.h>

class FrmMain;

class AbstractEvents_t
{
    friend void SetOrigRes();

protected:
    static FrmMain *m_form;

public:
    AbstractEvents_t() = default;
    virtual ~AbstractEvents_t() = default;

    /*!
     * \brief Initialize the events processor
     * \param form Pointer to the main form
     */
    virtual void init(FrmMain *form);

    /*!
     * \brief Process events
     */
    virtual void doEvents() = 0;

    /*!
     * \brief Wait until any events will happen
     */
    virtual void waitEvents() = 0;

public:
    static void eventResize();
};

extern AbstractEvents_t *g_events;

#endif // ABSTRACTEVENTS_T_H
