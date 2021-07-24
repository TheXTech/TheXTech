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

struct EditorConKeyboard_t
{
    int FastScroll = 0;
    int ScrollUp = 0;
    int ScrollDown = 0;
    int ScrollLeft = 0;
    int ScrollRight = 0;

    int PrevSection = 0;
    int NextSection = 0;

    int SwitchScreens = 0;
    int TestPlay = 0;
    int Select = 0;
    int Erase = 0;
};

struct KM_Key
{
    // SDL_Joystick control
    int val = -1;
    int id = -1;
    int type = -1;

    // SDL_GameController control
    int ctrl_val = -1;
    int ctrl_id = -1;
    int ctrl_type = -1;
};

//Public Type conJoystick   'Input settings for the joystick
struct ConJoystick_t
{
// EXTRA
    bool isValid = false;
    bool isGameController = false;
    bool isHaptic = false;

    enum CtrlTypes
    {
        NoControl=-1,
        JoyAxis=0,
        JoyBallX,
        JoyBallY,
        JoyHat,
        JoyButton,
        CtrlButton,
        CtrlAxis
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

struct EditorConJoystick_t
{
    bool isValid = false;
    bool isGameController = false;
    bool isHaptic = false;

    enum CtrlTypes
    {
        NoControl=-1,
        JoyAxis=0,
        JoyBallX,
        JoyBallY,
        JoyHat,
        JoyButton
    };
    KM_Key MouseLeft;
    KM_Key MouseRight;
    KM_Key MouseUp;
    KM_Key MouseDown;
    KM_Key MouseClick;

    KM_Key FastScroll;
    KM_Key ScrollUp;
    KM_Key ScrollDown;
    KM_Key ScrollLeft;
    KM_Key ScrollRight;

    KM_Key PrevSection;
    KM_Key NextSection;

    KM_Key SwitchScreens;
    KM_Key TestPlay;
    KM_Key Select;
    KM_Key Erase;
};

#endif // CON_CONTROL_H
