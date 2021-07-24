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

#ifndef CONTROLS_H
#define CONTROLS_H

//Public Type Controls 'Controls for the player
struct Controls_t
{
//    Up As Boolean
    bool Up = false;
//    Down As Boolean
    bool Down = false;
//    Left As Boolean
    bool Left = false;
//    Right As Boolean
    bool Right = false;
//    Jump As Boolean
    bool Jump = false;
//    AltJump As Boolean
    bool AltJump = false;
//    Run As Boolean
    bool Run = false;
//    AltRun As Boolean
    bool AltRun = false;
//    Drop As Boolean
    bool Drop = false;
//    Start As Boolean
    bool Start = false;
//End Type
};

// Controls for the editor
struct EditorControls_t
{
    double Game_MouseX;
    double Game_MouseY;
    bool MouseClick = false;

    bool FastScroll = false;
    bool ScrollUp = false;
    bool ScrollDown = false;
    bool ScrollLeft = false;
    bool ScrollRight = false;

    bool NextSection = false;
    bool PrevSection = false;
 
    bool SwitchScreens = false;
    bool TestPlay = false;
    bool Select = false;
    bool Erase = false;
};

#endif // CONTROLS_H
