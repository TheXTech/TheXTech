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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <string>
#include <vector>
#include "../range_arr.hpp"
#include "con_control.h"

struct SDL_JoyDeviceEvent;

// Public Sub UpdateControls() 'Gets players controls
// Gets players controls
void UpdateControls();

#ifdef USE_TOUCHSCREEN_CONTROLLER
void RenderTouchControls();
void UpdateTouchScreenSize();
const Controls_t &CurrentTouchControls();
#endif

extern void           joyFillDefaults(ConJoystick_t &j);
extern void           joyFillDefaults(ConKeyboard_t &k);

extern int            joyInitJoysticks();
extern void           joyGetAllUUIDs(int player, std::vector<std::string> &out);

extern int            joyCount();
extern ConJoystick_t &joyGetByUuid(int player, const std::string &uuid);
extern void           joyGetByUuid(ConJoystick_t &dst, int player, const std::string &uuid);
extern ConJoystick_t &joyGetByIndex(int player, int joyNum);
extern void           joyGetByIndex(int player, int joyNum, ConJoystick_t &dst);
extern void           joySetByUuid(int player, const std::string &uuid, const ConJoystick_t &cj);
extern void           joySetByIndex(int player, int index, const ConJoystick_t &cj);

extern int            joyGetPowerLevel(int joyNum);
extern void           joyRumble(int joyNum, int ms, float strength);
extern void           joyRumbleAllPlayers(int ms, float strength);

extern std::string    joyGetUuidStr(int joystick);
extern std::string    joyGetName(int joystick);

extern void           joyDeviceAddEvent(const SDL_JoyDeviceEvent *e);
extern void           joyDeviceRemoveEvent(const SDL_JoyDeviceEvent *e);

extern bool joyIsKeyDown(int JoystickNumber, const KM_Key &key);

extern void joyCloseJoysticks();
// Public Function StartJoystick(Optional ByVal JoystickNumber As Integer = 0) As Boolean
extern bool joyStartJoystick(int JoystickNumber);
// Public Sub PollJoystick()
extern bool joyPollJoystick(int joystick, KM_Key &key);

extern void setKey(ConKeyboard_t &ck, int id, int val);
extern void setKey(ConJoystick_t &cj, int id, const KM_Key &val);
extern KM_Key &getKey(ConJoystick_t &cj, int id);

#endif // JOYSTICK_H
