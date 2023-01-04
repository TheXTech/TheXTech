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
#ifndef MSGBOX_HHHHH
#define MSGBOX_HHHHH

#include <string>

#ifndef MSGBOX_CUSTOM
#   include "sdl_proxy/sdl_stdinc.h"
#   include "base/msgbox_base.h"
#   define E_INLINE SDL_FORCE_INLINE
#   define TAIL
#else
#   define E_INLINE    extern
#   define TAIL ;
#endif


/*!
 *  Message boxinterface
 */
namespace XMsgBox
{


enum MessageBoxFlags
{
    MESSAGEBOX_ERROR                 = 0x00000010,   /**< error dialog */
    MESSAGEBOX_WARNING               = 0x00000020,   /**< warning dialog */
    MESSAGEBOX_INFORMATION           = 0x00000040,   /**< informational dialog */
    MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT = 0x00000080,   /**< buttons placed left to right */
    MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT = 0x00000100    /**< buttons placed right to left */
};


#ifdef MSGBOX_CUSTOM

extern bool init();
extern void quit();

#endif

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
