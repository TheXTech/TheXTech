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
#ifndef ABSTRACTMSGBOX_H
#define ABSTRACTMSGBOX_H

#include <string>


class AbstractMsgBox_t
{
public:
    AbstractMsgBox_t() = default;
    virtual ~AbstractMsgBox_t() = default;

    /*!
     * \brief De-Initialize the message box
     */
    virtual void close() = 0;

    /*!
     * \brief Show the simple message box
     * \param flags Message box flags
     * \param title Title of the message box
     * \param message Message text
     * \return 0 on success or a negative error code on failure
     */
    virtual int simpleMsgBox(uint32_t flags, const std::string &title, const std::string &message) = 0;

    /*!
     * \brief Show the error message box
     * \param title Title of the message box
     * \param message Text of the emssage box
     */
    virtual void errorMsgBox(const std::string &title, const std::string &message) = 0;
};

extern AbstractMsgBox_t *g_msgBox;

#endif // ABSTRACTMSGBOX_H
