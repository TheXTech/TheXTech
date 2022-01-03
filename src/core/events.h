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


enum MouseButton_t
{
    MOUSE_BUTTON_NONE = 0,
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT
};

struct MouseButtonEvent_t
{
    int button = MOUSE_BUTTON_NONE;
};

struct MouseMoveEvent_t
{
    int x;
    int y;
};

struct MouseWheelEvent_t
{
    int x;
    int y;
};

enum KeyboardModifier_t
{
    KEYMOD_NONE = 0x0000,
    KEYMOD_LSHIFT = 0x0001,
    KEYMOD_RSHIFT = 0x0002,
    KEYMOD_LCTRL = 0x0040,
    KEYMOD_RCTRL = 0x0080,
    KEYMOD_LALT = 0x0100,
    KEYMOD_RALT = 0x0200,

    KEYMOD_CTRL = KEYMOD_LCTRL | KEYMOD_RCTRL,
    KEYMOD_SHIFT = KEYMOD_LSHIFT | KEYMOD_RSHIFT,
    KEYMOD_ALT = KEYMOD_LALT | KEYMOD_RALT,
};

struct KeyboardEvent_t
{
    int scancode;
    int mod;
};


class AbstractEvents_t
{
    friend void SetOrigRes();

    uint32_t m_lastMousePress = 0;

protected:
    FrmMain *m_form = nullptr;

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

    /*!
     * \brief Get key state by scancode
     * \param scan_code Scancode of the backend
     * \return Key state value
     */
    virtual uint8_t getKeyState(int scan_code) = 0;

    /*!
     * \brief Get the name of the key by scancode
     * \param scan_code Scancode of the backend
     * \return Human-readable name of the key
     */
    virtual const char *getScanCodeName(int scan_code) = 0;


protected:
    void eventDoubleClick();
    void eventKeyPress(int scan_code);
    void eventKeyDown(const KeyboardEvent_t &evt);
    void eventKeyUp(const KeyboardEvent_t &evt);
    void eventMouseDown(const MouseButtonEvent_t &event);
    void eventMouseMove(const MouseMoveEvent_t &event);
    void eventMouseWheel(const MouseWheelEvent_t &event);
    void eventMouseUp(const MouseButtonEvent_t &event);
    void eventResize();
};

extern AbstractEvents_t *g_events;

#endif // ABSTRACTEVENTS_T_H
