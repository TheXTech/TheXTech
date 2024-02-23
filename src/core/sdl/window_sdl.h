/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef WINDOWSDL_H
#define WINDOWSDL_H

#include <string>
#include "../base/window_base.h"
#include "cmd_line_setup.h"


typedef struct SDL_Window SDL_Window;

class WindowSDL final : public AbstractWindow_t
{
    SDL_Window *m_window = nullptr;
    WindowCursor_t m_cursor = CURSOR_DEFAULT;

    bool m_fullscreen = false;

public:
    WindowSDL();
    virtual ~WindowSDL();

    bool initSDL(uint32_t windowInitFlags);

    void close() override;

    SDL_Window *getWindow();

    bool isSdlError();

    /*!
     * \brief Show the window
     */
    void show() override;

    /*!
     * \brief Hide the window
     */
    void hide() override;

    /*!
     * \brief Updates the window icon based on AppPath
     */
    void updateWindowIcon() override;

    /**
     *  \brief Toggle whether or not the cursor is shown.
     *
     *  \param toggle 1 to show the cursor, 0 to hide it, -1 to query the current
     *                state.
     *
     *  \return 1 if the cursor is shown, or 0 if the cursor is hidden.
     */
    int showCursor(int show) override;

    /*!
     * \brief Change the displayable cursor type
     * \param cursor cursor type
     */
    void setCursor(WindowCursor_t cursor) override;

    /*!
     * \brief Get the current cursor type
     * \return Cursor type
     */
    WindowCursor_t getCursor() override;

    /*!
     * \brief Place cursor at desired window position
     * \param x and y positions in physical window coordinates
     */
    void placeCursor(int window_x, int window_y) override;

    /*!
     * \brief Is full-screen mode active?
     * \return True if the full-screen mode works right now
     */
    bool isFullScreen() override;

    /*!
     * \brief Change between fullscreen and windowed modes
     * \param fs Fullscreen state
     * \return 1 when full-screen mode toggled, 0 when windowed mode toggled, -1 on any errors
     */
    int setFullScreen(bool fs) override;

    /*!
     * \brief Restore the size and position of a minimized or maximized window.
     */
    void restoreWindow() override;

    /**
     * @brief Change window size in cursor pixels
     * @param w Width
     * @param h Height
     */
    void setWindowSize(int w, int h) override;

    /*!
     * \brief Get the current size of the window in cursor pixels
     * \param w Width
     * \param h Height
     */
    void getWindowSize(int *w, int *h) override;

    /*!
     * \brief Does window has an input focus?
     * \return true if window active
     */
    bool hasWindowInputFocus() override;

    /*!
     * \brief Does window has a mouse focus?
     * \return true if window has a mouse focus
     */
    bool hasWindowMouseFocus() override;

    /*!
     * \brief Is window maximized (resized to fill desktop)?
     * \return true if window is maximized
     */
    bool isMaximized() override;

    /*!
     * \brief Set the title of the window
     * \param title Title to set, as C string
     */
    void setTitle(const char* title) override;
};

#endif // WINDOWSDL_H
