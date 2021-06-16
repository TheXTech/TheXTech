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

#ifndef SPEEDRUNNER_H
#define SPEEDRUNNER_H

#include "control_types.h"

enum
{
    SPEEDRUN_MODE_OFF = 0,
    SPEEDRUN_MODE_1,
    SPEEDRUN_MODE_2,
    SPEEDRUN_MODE_3
};

extern int  g_speedRunnerMode;

void speedRun_tick();

void speedRun_setSemitransparentRender(bool r);

void speedRun_syncControlKeys(int plr, Controls_t &keys);

void speedRun_loadStats();
void speedRun_saveStats();

void speedRun_resetCurrent();
void speedRun_resetTotal();

void speedRun_bossDeadEvent();

void speedRun_render();

#endif // SPEEDRUNNER_H
