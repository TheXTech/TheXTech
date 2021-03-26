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
