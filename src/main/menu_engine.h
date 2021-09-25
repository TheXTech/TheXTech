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

#ifndef MENU_ENGINE_H
#define MENU_ENGINE_H

#include <SDL2/SDL_atomic.h>
#include <string>
#include <vector>
#include <functional>

#include "globals.h"
#include "sound.h"

struct MenuList;

struct MenuItem
{
    //! Draw the state
    std::function<void(MenuItem *self, int x, int y, int ix, int iy)> render = nullptr;
    //! Trigger the menu item
    std::function<bool(MenuItem *self)> trigger = nullptr;
    //! Update the state (by left-right actions for example)
    std::function<void(MenuItem *self)> update = nullptr;
    //! Displayable label
    std::string label;
    //! Key value used for special cases like playable character selection menus
    int key = -1;

    MenuItem() = default;
    MenuItem(const MenuItem &o) = default;
    MenuItem &operator=(const MenuItem &o) = default;
};

struct MenuList
{
    int x_base = 0;
    int y_base = 0;
    int x_value_offset = 0;
    int v_spacing = 30;
    int char_width = 19;
    int item_height = 16;
    bool scrollable = false;
    int scroll_min = 0;
    int scroll_max = 0;
    int scroll_delay = 0;
    //! Maximum items visible
    int max_visible = 5;

    //! Menu it of this list
    int menu_id = 0;

    std::vector<MenuItem> list;

    std::function<void(MenuList *self)> action_back = nullptr;

    MenuList() = default;
    MenuList(const MenuList &o) = default;
    MenuList &operator=(const MenuList &o) = default;

    void prepare()
    {
        scrollable = false;
        if(list.size() > (size_t)max_visible)
        {
            scrollable = true;
            scroll_min = 0;
            scroll_max = (max_visible - 1);
        }
    }

    bool update(SDL_atomic_t &loading)
    {
        bool altPressed = getKeyState(SDL_SCANCODE_LALT) == KEY_PRESSED ||
                          getKeyState(SDL_SCANCODE_RALT) == KEY_PRESSED;
        bool escPressed = getKeyState(SDL_SCANCODE_ESCAPE) == KEY_PRESSED;
#ifdef __ANDROID__
        escPressed |= getKeyState(SDL_SCANCODE_AC_BACK) == KEY_PRESSED;
#endif
        bool spacePressed = getKeyState(SDL_SCANCODE_SPACE) == KEY_PRESSED;
        bool returnPressed = getKeyState(SDL_SCANCODE_RETURN) == KEY_PRESSED;
        bool upPressed = getKeyState(SDL_SCANCODE_UP) == KEY_PRESSED;
        bool downPressed = getKeyState(SDL_SCANCODE_DOWN) == KEY_PRESSED;

        bool menuDoPress = (returnPressed && !altPressed) || spacePressed;
        bool menuBackPress = (escPressed && !altPressed);

        Controls_t &c = Player[1].Controls;

        menuDoPress |= (c.Start || c.Jump) && !altPressed;
        menuBackPress |= c.Run && !altPressed;

        if(frmMain.MousePointer != 99)
        {
            frmMain.MousePointer = 99;
            showCursor(0);
        }

        if(!c.Up && !c.Down && !c.Jump && !c.Run && !c.Start)
        {
            bool k = false;
            k |= menuDoPress;
            k |= upPressed;
            k |= downPressed;
            k |= escPressed;

            if(!k)
                MenuCursorCanMove = true;
        }

        if(!getNewKeyboard && !getNewJoystick)
        {
            int cursorDelta = 0;

            if(c.Up || upPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor -= 1;
                    cursorDelta = -1;
                }

                MenuCursorCanMove = false;
            }
            else if(c.Down || downPressed)
            {
                if(MenuCursorCanMove)
                {
                    MenuCursor += 1;
                    cursorDelta = +1;
                }
                MenuCursorCanMove = false;
            }

            if(cursorDelta != 0)
            {
                PlaySoundMenu(SFX_Slide);
            }
        } // No keyboard/Joystick grabbing active

        // Block any menu actions while loading in process
        if(SDL_AtomicGet(&loading))
        {
            if((menuDoPress && MenuCursorCanMove) || MenuMouseClick)
                PlaySoundMenu(SFX_BlockHit);
            if(MenuCursor != 0)
                MenuCursor = 0;
            return false;
        }

        if(scroll_delay > 0)
        {
            MenuMouseMove = true;
            scroll_delay -= 1;
        }

        if(MenuMouseMove)
        {
            int ibeg = 0;
            int iend = (list.size() - 1);

            if(scrollable)
            {
                ibeg = scroll_min;
                iend = scroll_max;
            }

            for(int i = ibeg; i <= iend; ++i)
            {
                //! Relative index
                int ri = (i - ibeg);

                if(MenuMouseY >= y_base + (ri * v_spacing) &&
                   MenuMouseY <= y_base + item_height + (ri * v_spacing))
                {
                    int len = list[i].label.size() * char_width;
                    if(MenuMouseX >= x_base && MenuMouseX <= x_base + len)
                    {
                        if(MenuMouseRelease && MenuMouseDown)
                            MenuMouseClick = true;

                        if(MenuCursor != i && scroll_delay == 0)
                        {
                            scroll_delay = 10;
                            PlaySoundMenu(SFX_Slide);
                            MenuCursor = i;
                        }
                    }
                }
            }
        }

        if(MenuCursorCanMove || MenuMouseClick || MenuMouseBack)
        {
            if(menuBackPress || MenuMouseBack)
            {
                if(action_back)
                    action_back(this);
                MenuCursorCanMove = false;
            }
            else if(menuDoPress || MenuMouseClick)
            {
                auto &it = list[MenuCursor];
                if(it.trigger)
                {
                    if(it.trigger(&it))
                        return true; // Menu closure has been requested
                }
                MenuCursorCanMove = false;
            }
        }

        if(MenuMode == menu_id)
        {
            int ms = (list.size() - 1);
            if(MenuCursor > ms) MenuCursor = 0;
            if(MenuCursor < 0) MenuCursor = ms;
        }

        return false; // Menu is not going to be closed
    }

    void renderMenu()
    {
        int ibeg = 0;
        int iend = (list.size() - 1);

        if(scrollable)
        {
            ibeg = scroll_min;
            iend = scroll_max;
        }

        for(int i = ibeg; i <= iend; ++i)
        {
            //! Relative index
            int ri = (i - ibeg);
            auto &it = list[i];
            it.render(&it,
                      x_base, y_base + (ri * v_spacing),
                      x_base + x_value_offset, y_base + (ri * v_spacing));
        }

        if(scrollable)
        {
            if(scroll_min > 1)
                frmMain.renderTexture(x_base + 100 - 8, y_base - 20, GFX.MCursor[1]);
            if(scroll_max < NumSelectWorld)
                frmMain.renderTexture(x_base + 100 - 8, y_base + (v_spacing * max_visible) - 20, GFX.MCursor[2]);
        }

        frmMain.renderTexture(x_base - 20, y_base + (MenuCursor * v_spacing), 16, 16, GFX.MCursor[0], 0, 0);
    }
};

#endif // MENU_ENGINE_H
