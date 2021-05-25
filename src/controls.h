/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
