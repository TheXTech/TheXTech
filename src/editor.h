/*
 * TheXTech - A platform game engine ported from old source code for VB6
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

extern int EditorNPCFrame(int A, float C, int N = 0);

extern void GetEditorControls();

extern void OptCursorSync();

extern void SetCursor();

extern void PositionCursor();

extern void HideCursor();

extern void KillWarp(int A);

extern void zTestLevel(bool magicHand = false);

extern void MouseMove(float X, float Y, bool nCur = false);

extern void ResetNPC(int A);

extern void BlockFill(Location_t Loc);

#endif // EDITOR_H
