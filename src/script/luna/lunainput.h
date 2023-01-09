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
#ifndef LUNAINPUT_H
#define LUNAINPUT_H

struct Player_t;

extern int gFrames;

extern int gLastDownPress;
extern int gDownTapped;
extern int gLastUpPress;
extern int gUpTapped;
extern int gLastLeftPress;
extern int gLeftTapped;
extern int gLastRightPress;
extern int gRightTapped;

extern int gLastJumpPress;
extern int gJumpTapped;
extern int gLastRunPress;
extern int gRunTapped;


namespace Input {

void CheckSpecialCheats();

void UpdateInputTasks();

void ResetTaps();
void ResetAll();
void UpdateKeyRecords(Player_t* player);

bool PressingUp();
bool PressingDown();
bool PressingLeft();
bool PressingRight();
bool PressingRun();
bool PressingJump();

bool UpThisFrame();
bool DownThisFrame();
bool LeftThisFrame();
bool RightThisFrame();
bool RunThisFrame();
bool JumpThisFrame();

}


#endif // LUNAINPUT_H
