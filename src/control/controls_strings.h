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

#ifndef CONTROLS_STRINGS_H
#define CONTROLS_STRINGS_H

#include <string>

#include "control/controls_methods.h"

// configuration: build all strings if lang tools enabled, otherwise just the needed ones
#if !defined(THEXTECH_DISABLE_LANG_TOOLS) || defined(INPUT_16M_H) || defined(INPUT_3DS_H)
#define CONTROLS_16M_STRINGS
#endif

#if !defined(THEXTECH_DISABLE_LANG_TOOLS) || defined(INPUT_3DS_H)
#define CONTROLS_3DS_STRINGS
#endif

#if !defined(THEXTECH_DISABLE_LANG_TOOLS) || defined(INPUT_WII_H)
#define CONTROLS_WII_STRINGS
#endif

#if !defined(THEXTECH_DISABLE_LANG_TOOLS) || defined(KEYBOARD_H)
#define CONTROLS_KEYBOARD_STRINGS
#endif

#if !defined(THEXTECH_DISABLE_LANG_TOOLS) || defined(TOUCHSCREEN_H)
#define CONTROLS_TOUCHSCREEN_STRINGS
#endif

#if !defined(THEXTECH_DISABLE_LANG_TOOLS) || defined(JOYSTICK_H)
#define CONTROLS_JOYSTICK_STRINGS
#endif


struct ControlsStrings_t
{

    std::string sharedCaseInvalid = "(Invalid)";
    std::string sharedOptionMaxPlayers = "Max Players";

#ifdef CONTROLS_KEYBOARD_STRINGS
    std::string nameKeyboard = "Keyboard";
    std::string keyboardOptionTextEntryStyle = "Text Entry Style";
#endif

#if defined(CONTROLS_KEYBOARD_STRINGS) || defined(CONTROLS_JOYSTICK_STRINGS)
    std::string nameGamepad = "Gamepad";
#endif

#if defined(CONTROLS_JOYSTICK_STRINGS)
    std::string nameOldJoy = "Old Joy";
    std::string phraseNewProfOldJoy = "New Profile for Old Joystick";
#endif

#ifdef CONTROLS_TOUCHSCREEN_STRINGS
    std::string nameTouchscreen = "Touchscreen";
#endif

#ifdef CONTROLS_16M_STRINGS
    std::string tdsButtonA = "A";
    std::string tdsButtonB = "B";
    std::string tdsButtonX = "X";
    std::string tdsButtonY = "Y";
    std::string tdsButtonL = "L";
    std::string tdsButtonR = "R";
    std::string tdsButtonSelect = "Select";
    std::string tdsButtonStart = "Start";

    std::string tdsCasePen = "(Pen)";
#endif // #ifdef CONTROLS_16M_STRINGS

#if defined(CONTROLS_3DS_STRINGS)
    std::string tdsDpad = "D-Pad";
    std::string tdsTstick = "Thumb";
    std::string tdsCstick = "C-Stick";
    std::string tdsButtonZL = "ZL";
    std::string tdsButtonZR = "ZR";
#endif // #ifdef CONTROLS_3DS_STRINGS

#ifdef CONTROLS_WII_STRINGS
    std::string wiiTypeWiimote = "Wiimote";

    std::string wiiDpad = "D-Pad";

    std::string wiiButtonA = "A";
    std::string wiiButtonB = "B";
    std::string wiiButtonMinus = "-";
    std::string wiiButtonPlus = "+";
    std::string wiiButtonHome = "Home";
    std::string wiiButton2 = "2";
    std::string wiiButton1 = "1";
    std::string wiiShake = "Shake";

    std::string wiiTypeNunchuck = "Nunchuck";
    std::string wiiPhraseNewNunchuck = "New Nunchuck Profile";

    std::string wiiPrefixNunchuck = "N";

    std::string wiiButtonZ = "Z";
    std::string wiiButtonC = "C";

    std::string wiiTypeClassic = "Classic";
    std::string wiiPhraseNewClassic = "New Classic Profile";

    std::string wiiLStick = "L-Pad";
    std::string wiiRStick = "R-Pad";

    std::string wiiButtonZL = "ZL";
    std::string wiiButtonZR = "ZR";
    std::string wiiButtonLT = "LT";
    std::string wiiButtonRT = "RT";
    std::string wiiButtonX = "X";
    std::string wiiButtonY = "Y";

    std::string wiiCaseIR = "(IR)";

#endif // #ifdef CONTROLS_WII_STRINGS


};

extern ControlsStrings_t g_controlsStrings;

#endif // #ifndef CONTROLS_STRINGS_H
