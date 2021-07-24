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

#ifndef FRAME_TIMER_H
#define FRAME_TIMER_H

#include <functional>

struct PerformanceStats_t
{
    // How many objects got drawn in one frame
    int renderedBlocks = 0;
    int renderedSzBlocks = 0;
    int renderedBGOs = 0;
    int renderedNPCs = 0;
    int renderedEffects = 0;

    // How many objects got scanned to find what to render
    int checkedBlocks = 0;
    int checkedSzBlocks = 0;
    int checkedBGOs = 0;
    int checkedNPCs = 0;
    int checkedEffects = 0;

    int renderedTiles = 0;
    int renderedScenes = 0;
    int renderedPaths = 0;
    int renderedLevels = 0;

    int checkedTiles = 0;
    int checkedScenes = 0;
    int checkedPaths = 0;
    int checkedLevels = 0;

    // How many objects got checked during the physics processing
    int physScannedBlocks = 0;
    int physScannedBGOs = 0;
    int physScannedNPCs = 0;

    bool enabled = false;

    void reset();
    void print();
};

extern PerformanceStats_t g_stats;

void resetFrameTimer();
void resetTimeBuffer();
bool frameSkipNeeded();

typedef void (*LoopCall_t)(void);

bool canProceedFrame();
void computeFrameTime1();
void computeFrameTime2();
void frameNextInc();
void cycleNextInc();

void frameRenderStart();
void frameRenderEnd();

void runFrameLoop(LoopCall_t doLoopCallbackPre,
                  LoopCall_t doLoopCallbackPost,
                  std::function<bool ()> condition,
                  std::function<bool ()> subCondition = nullptr,
                  std::function<void ()> preTimerExtraPre = nullptr,
                  std::function<void ()> preTimerExtraPost = nullptr);

#endif // FRAME_TIMER_H
