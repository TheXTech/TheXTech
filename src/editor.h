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

#ifndef EDITOR_H
#define EDITOR_H

#include "location.h"

struct Point_t
{
    int X = 0;
    int Y = 0;
};

extern Point_t CursorPos;
extern bool HasCursor;
extern bool NoReallyKillIt;
extern int curSection;

extern bool enableAutoAlign;

struct OptCursor_t
{
    enum Level {
        LVL_SELECT = 13,
        LVL_ERASER0 = 0,
        LVL_ERASER = 6,
        LVL_BLOCKS = 1,
        LVL_BGOS = 3,
        LVL_NPCS = 4,
        LVL_SETTINGS = 2,
        LVL_WARPS = 5,
        LVL_WATER = 15
    };
    int current = LVL_SELECT;
};

extern OptCursor_t optCursor;

// this sub handles the level editor
// it is still called when the player is testing a level in the editor in windowed mode
extern void UpdateEditor();

extern void UpdateInterprocess();

extern int EditorNPCFrame(const int A, float& C, int N = 0);

extern void GetEditorControls();

extern void OptCursorSync();

extern void SetCursor();

extern void PositionCursor();

extern void HideCursor();

extern void KillWarp(int A);

extern void zTestLevel(bool magicHand = false, bool interProcess = false);

extern void MouseMove(float X, float Y, bool nCur = false);

extern void ResetNPC(int A);

extern void BlockFill(const Location_t &Loc);

#endif // EDITOR_H
