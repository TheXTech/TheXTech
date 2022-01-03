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
#ifndef WINDOWSDL_H
#define WINDOWSDL_H

#include <string>
#include "window.h"
#include "cmd_line_setup.h"


typedef struct SDL_Window SDL_Window;

class WindowSDL : public AbstractWindow_t
{
    std::string m_windowTitle;
    SDL_Window *m_window = nullptr;

public:
    WindowSDL();
    virtual ~WindowSDL();

    bool initSDL(const CmdLineSetup_t &setup, uint32_t windowInitFlags);

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
     * @brief Change window size
     * @param w Width
     * @param h Height
     */
    void setWindowSize(int w, int h) override;

    /*!
     * \brief Get the current size of the window
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
};

#endif // WINDOWSDL_H
