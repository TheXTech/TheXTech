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
#ifndef MSGBOX_HHHHH
#define MSGBOX_HHHHH

#ifndef MSGBOX_CUSTOM
#   include <SDL2/SDL_stdinc.h>
#   define E_INLINE SDL_FORCE_INLINE
#   define TAIL
#   include "base/msgbox_base.h"
#else
#   define E_INLINE    extern
#   define TAIL ;
#endif


/*!
 *  Message boxinterface
 */
namespace XMsgBox
{
/*!
 * \brief Show the simple message box
 * \param flags Message box flags
 * \param title Title of the message box
 * \param message Message text
 * \return 0 on success or a negative error code on failure
 */
E_INLINE int simpleMsgBox(uint32_t flags, const std::string &title, const std::string &message) TAIL
#ifndef MSGBOX_CUSTOM
{
    if(g_msgBox)
        return g_msgBox->simpleMsgBox(flags, title, message);
    return -1;
}
#endif

/*!
 * \brief Show the error message box
 * \param title Title of the message box
 * \param message Text of the emssage box
 */
E_INLINE void errorMsgBox(const std::string &title, const std::string &message) TAIL
#ifndef MSGBOX_CUSTOM
{
    if(g_msgBox)
        g_msgBox->errorMsgBox(title, message);
}
#endif

} // XMsgBox

#ifndef MSGBOX_CUSTOM
#   undef E_INLINE
#   undef TAIL
#endif

#endif // MSGBOX_HHHHH
