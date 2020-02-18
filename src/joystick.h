/*
 * A2xTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "range_arr.hpp"

// Public JoyNum As Long
extern long JoyNum;
// Public CenterX(0 To 7) As Long
extern RangeArr<long, 0, 7> CenterX;
// Public CenterY(0 To 7) As Long
extern RangeArr<long, 0, 7> CenterY;
// Public JoyButtons(-15 To 15) As Boolean
extern RangeArr<bool, -255, 255> JoyButtons;
// Public CurrentJoyX As Long
extern long CurrentJoyX;
// Public CurrentJoyY As Long
extern long CurrentJoyY;
// Public CurrentJoyPOV As Long
extern long CurrentJoyPOV;  // Also looks like useless, kept be a constant -1


// Public Sub UpdateControls() 'Gets players controls
// Gets players controls
void UpdateControls();

int InitJoysticks();

void CloseJoysticks();
// Public Function StartJoystick(Optional ByVal JoystickNumber As Integer = 0) As Boolean
bool StartJoystick(int JoystickNumber);
// Public Sub PollJoystick()
void PollJoystick(int joystick);


#endif // JOYSTICK_H
