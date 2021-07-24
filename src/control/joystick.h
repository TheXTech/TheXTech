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

// Gets editor controls
void UpdateEditorControls();

#ifdef USE_TOUCHSCREEN_CONTROLLER
void RenderTouchControls();
void UpdateTouchScreenSize();
const Controls_t &CurrentTouchControls();
#endif

extern void           joyFillDefaults(ConJoystick_t &j);
extern void           joyFillDefaults(ConKeyboard_t &k);
extern void           editorJoyFillDefaults(EditorConJoystick_t &j);
extern void           editorJoyFillDefaults(EditorConKeyboard_t &j);

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
