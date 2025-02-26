/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef SPEEDRUNNER_H
#define SPEEDRUNNER_H

#include "control_types.h"

namespace XPower { struct StatusInfo; }

enum
{
    SPEEDRUN_MODE_OFF = 0,
    SPEEDRUN_MODE_1,
    SPEEDRUN_MODE_2,
    SPEEDRUN_MODE_3
};

void speedRun_tick();

void speedRun_syncControlKeys(int l_player_i, const Controls_t &keys);

void speedRun_loadStats();
void speedRun_saveStats();

void speedRun_resetCurrent();
void speedRun_resetTotal();

void speedRun_triggerEnter();
void speedRun_triggerLeave();

void speedRun_bossDeadEvent();

void RenderPowerInfo(int l_player_i, int bx, int by, int bw, int bh, uint8_t alpha, const XPower::StatusInfo* status);

// l_player_i is an index into the local screen's player array, not a global player index anymore.
void RenderControls(int l_player_i, int x, int y, int w, int h, bool missing, uint8_t alpha, bool connect_screen = false);
void RenderControls(const Controls_t& controls, int x, int y, int w, int h, bool missing, uint8_t alpha);

enum
{
    SPEEDRUN_ALIGN_LEFT = -1,
    SPEEDRUN_ALIGN_AUTO,
    SPEEDRUN_ALIGN_RIGHT,
};

// player is an index into the local screen's player array, not a global player index
void speedRun_renderControls(int l_player_i, int screenZ = -1, int align = SPEEDRUN_ALIGN_AUTO);
void speedRun_renderTimer();

#endif // SPEEDRUNNER_H
