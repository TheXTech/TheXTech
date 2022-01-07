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
#ifndef ABSTRACTWINDOW_T_H
#define ABSTRACTWINDOW_T_H


class AbstractWindow_t
{
public:
    AbstractWindow_t();
    virtual ~AbstractWindow_t();

    /*!
     * \brief De-Init the stuff and close
     */
    virtual void close() = 0;

    /*!
     * \brief Show the window
     */
    virtual void show() = 0;

    /*!
     * \brief Hide the window
     */
    virtual void hide() = 0;

    /**
     *  \brief Toggle whether or not the cursor is shown.
     *
     *  \param toggle 1 to show the cursor, 0 to hide it, -1 to query the current
     *                state.
     *
     *  \return 1 if the cursor is shown, or 0 if the cursor is hidden.
     */
    virtual int showCursor(int show) = 0;

    /*!
     * \brief Cursor type
     */
    enum Cursor_t
    {
        CURSOR_DEFAULT = 0,
        CURSOR_NONE
    };

    /*!
     * \brief Change the displayable cursor type
     * \param cursor cursor type
     */
    virtual void setCursor(Cursor_t cursor) = 0;

    /*!
     * \brief Get the current cursor type
     * \return Cursor type
     */
    virtual Cursor_t getCursor() = 0;

    /*!
     * \brief Is full-screen mode active?
     * \return True if the full-screen mode works right now
     */
    virtual bool isFullScreen() = 0;

    /*!
     * \brief Change between fullscreen and windowed modes
     * \param fs Fullscreen state
     * \return 1 when full-screen mode toggled, 0 when windowed mode toggled, -1 on any errors
     */
    virtual int setFullScreen(bool fs) = 0;

    /*!
     * \brief Restore the size and position of a minimized or maximized window.
     */
    virtual void restoreWindow() = 0;

    /**
     * @brief Change window size
     * @param w Width
     * @param h Height
     */
    virtual void setWindowSize(int w, int h) = 0;

    /*!
     * \brief Get the current size of the window
     * \param w Width
     * \param h Height
     */
    virtual void getWindowSize(int *w, int *h) = 0;

    /*!
     * \brief Does window has an input focus?
     * \return true if window active
     */
    virtual bool hasWindowInputFocus() = 0;

    /*!
     * \brief Does window has a mouse focus?
     * \return true if window has a mouse focus
     */
    virtual bool hasWindowMouseFocus() = 0;
};

extern AbstractWindow_t *g_window;

#endif // ABSTRACTWINDOW_T_H
