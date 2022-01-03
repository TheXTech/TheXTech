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

#include <SDL2/SDL_messagebox.h>
#include <SDL2/SDL_version.h>

#include "msgbox_sdl.h"


MsgBoxSDL::MsgBoxSDL() :
    AbstractMsgBox_t()
{}

MsgBoxSDL::~MsgBoxSDL()
{}

void MsgBoxSDL::init(SDL_Window *ptr)
{
    m_window = ptr;
}

void MsgBoxSDL::close()
{}

int MsgBoxSDL::simpleMsgBox(uint32_t flags, const std::string &title, const std::string &message)
{
    Uint32 dFlags = 0;

    if(flags & MESSAGEBOX_ERROR)
        dFlags |= SDL_MESSAGEBOX_ERROR;

    if(flags & MESSAGEBOX_WARNING)
        dFlags |= SDL_MESSAGEBOX_WARNING;

    if(flags & MESSAGEBOX_INFORMATION)
        dFlags |= SDL_MESSAGEBOX_INFORMATION;

#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(2, 0, 12)
    if(flags & MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT)
        dFlags |= SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;

    if(flags & MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT)
        dFlags |= SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT;
#endif

    return SDL_ShowSimpleMessageBox(dFlags,
                                    title.c_str(),
                                    message.c_str(),
                                    m_window);
}

void MsgBoxSDL::errorMsgBox(const std::string &title, const std::string &message)
{
    const std::string &ttl = title;
    const std::string &msg = message;
    SDL_MessageBoxData mbox;
    SDL_MessageBoxButtonData mboxButton;
    const SDL_MessageBoxColorScheme colorScheme =
    {
        { /* .colors (.r, .g, .b) */
            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
            { 200, 200, 200 },
            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
            {   0,   0,   0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
            { 255, 255, 255 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
            { 150, 150, 150 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
            { 255, 255, 255 }
        }
    };

    mboxButton.buttonid = 0;
    mboxButton.flags    = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    mboxButton.text     = "Ok";
    mbox.flags          = SDL_MESSAGEBOX_ERROR;
    mbox.window         = m_window;
    mbox.title          = ttl.c_str();
    mbox.message        = msg.c_str();
    mbox.numbuttons     = 1;
    mbox.buttons        = &mboxButton;
    mbox.colorScheme    = &colorScheme;
    SDL_ShowMessageBox(&mbox, nullptr);
}
