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
extern bool g_drawController;

void speedRun_tick();

void speedRun_setSemitransparentRender(bool r);

void speedRun_syncControlKeys(int plr, const Controls_t &keys);

void speedRun_loadStats();
void speedRun_saveStats();

void speedRun_resetCurrent();
void speedRun_resetTotal();

void speedRun_triggerEnter();
void speedRun_triggerLeave();

void speedRun_bossDeadEvent();

void RenderControls(int player, int x, int y, int w, int h);

void speedRun_renderControls(int player, int screenZ = -1);
void speedRun_renderTimer();

#endif // SPEEDRUNNER_H
