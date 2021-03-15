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

#ifndef CON_CONTROL_H
#define CON_CONTROL_H

#include <cstdint>
#include <string>

//Public Type conKeyboard  'Input settings for the keyboard
struct ConKeyboard_t
{
//    Up As Integer
    int Up = 0;
//    Down As Integer
    int Down = 0;
//    Left As Integer
    int Left = 0;
//    Right As Integer
    int Right = 0;
//    Jump As Integer
    int Jump = 0;
//    AltJump As Integer
    int AltJump = 0;
//    Run As Integer
    int Run = 0;
//    AltRun As Integer
    int AltRun = 0;
//    Drop As Integer
    int Drop = 0;
//    Start As Integer
    int Start = 0;
//End Type
};

struct KM_Key
{
    int val = -1;
    int id = -1;
    int type = -1;
};

//Public Type conJoystick   'Input settings for the joystick
struct ConJoystick_t
{
// EXTRA
    std::string hwGUID;
    bool isGameController = false;

    enum CtrlTypes
    {
        NoControl=-1,
        JoyAxis=0,
        JoyBallX,
        JoyBallY,
        JoyHat,
        JoyButton
    };

    KM_Key Up;
    KM_Key Down;
    KM_Key Left;
    KM_Key Right;

//    Jump As Integer
    KM_Key Jump;
//    Run As Integer
    KM_Key Run;
//    Drop As Integer
    KM_Key Drop;
//    Start As Integer
    KM_Key Start;
//    AltJump As Integer
    KM_Key AltJump;
//    AltRun As Integer
    KM_Key AltRun;
//End Type
};

#endif // CON_CONTROL_H
